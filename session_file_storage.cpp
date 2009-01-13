#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <boost/crc.hpp>
#include <boost/bind.hpp>
#include "global_config.h"

#include "session_file_storage.h"
#include "cppcms_error.h"
#include "session_sid.h"

#include "config.h"

using namespace std;

namespace cppcms {

namespace storage {

#define LOCK_SIZE 256
	
class io_error : public std::runtime_error {
public:
	io_error() : std::runtime_error("IO"){};		
};

void io::close(int fid) const
{
	::close(fid);
}

int io::lock_id(std::string const &sid) const
{
	int id;
	char buf[3] = {0};
	buf[0]=sid.at(0);
	buf[1]=sid.at(1);
	sscanf(buf,"%x",&id);
	return id;
}

string io::mkname(std::string const &sid) const
{
	return dir+"/"+sid;
}
void io::write(std::string const &sid,time_t timestamp,void const *buf,size_t len) const
{
	string name=mkname(sid);
	int fid=::open(name.c_str(),O_CREAT | O_TRUNC | O_WRONLY ,0666);
	if(fid<0) {
		throw cppcms_error(errno,"storage::local_id");
	}
	::write(fid,&timestamp,sizeof(time_t));
	::write(fid,buf,len);
	boost::crc_32_type crc_calc;
	crc_calc.process_bytes(buf,len);
	uint32_t crc=crc_calc.checksum();
	::write(fid,&crc,4);
	close(fid);
}

bool io::read(std::string const &sid,time_t &timestamp,vector<unsigned char> *out) const 
{
	int fid=-1;
	string name=mkname(sid);
	try {
		fid=::open(name.c_str(),O_RDONLY);
		if(fid<0) {
			return false;
		}
		time_t tmp;
		if(::read(fid,&tmp,sizeof(time_t))!=sizeof(time_t) || tmp < time(NULL)) 
			throw io_error();
		timestamp=tmp;
		if(!out) {
			close(fid);
			return true;
		}
		int size=lseek(fid,0,SEEK_END);
		if(size==-1 || size <(int)sizeof(time_t)+4) 
			throw io_error();
		size-=sizeof(time_t)+4;
		if(lseek(fid,sizeof(time_t),SEEK_SET) < 0) 
			throw io_error();
		out->resize(size,0);
		if(::read(fid,&out->front(),size)!=size) 
			throw io_error();
		boost::crc_32_type crc_calc;
		crc_calc.process_bytes(&out->front(),size);
		uint32_t crc_ch,crc=crc_calc.checksum();
		if(::read(fid,&crc_ch,4)!=4 || crc_ch != crc)
			throw io_error();
		close(fid);
		return true;
	}
	catch(io_error const &e){
		if(fid>=0)
			close(fid);
		::unlink(name.c_str());
		return false;
	}
	catch(...) {
		if(fid>=0) close(fid);
		::unlink(name.c_str());
		throw;
	}
}

void io::unlink(std::string const &sid) const 
{
	string name=mkname(sid);
	::unlink(name.c_str());
}

local_io::local_io(std::string dir,pthread_rwlock_t *l):
	io(dir),
	locks(l)
{
}

void local_io::wrlock(std::string const &sid) const
{
	pthread_rwlock_wrlock(&locks[lock_id(sid)]);
}
void local_io::rdlock(std::string const &sid) const
{
	pthread_rwlock_rdlock(&locks[lock_id(sid)]);
}
void local_io::unlock(std::string const &sid) const
{
	pthread_rwlock_unlock(&locks[lock_id(sid)]);
}

void nfs_io::close(int fid)
{
	::fsync(fid);
	::close(fid);
}

nfs_io::nfs_io(std::string dir) : thread_io(dir)
{
	string lockf=dir+"/"+"nfs.lock";
	fid=::open(lockf.c_str(),O_CREAT | O_RDWR,0666);
	if(fid<0) {
		throw cppcms_error(errno,"storage::nfs_io::open");
	}
	::lseek(fid,LOCK_SIZE,SEEK_SET);
	::write(fid,"",1);
}

nfs_io::~nfs_io()
{
	::close(fid);
}

namespace {

bool flock(int fid,int how,int pos)
{
	struct flock lock;
	memset(&lock,0,sizeof(lock));
	lock.l_type=how;
	lock.l_whence=SEEK_SET;
	lock.l_start=pos;
	lock.l_len=1;
	int res;
	while((res=fcntl(fid,F_SETLKW,&lock)!=0) && errno==EINTR)
		;
	if(res) return false;
	return true;	
}

} // anon namespace


// withing same process you should add additional mutex on the operations

void nfs_io::wrlock(std::string const &sid) const
{
	thread_io::wrlock(sid);
	if(!flock(fid,F_WRLCK,lock_id(sid)))
		throw cppcms_error(errno,"storage::nfs_io::fcntl::WRITE LOCK");
}
void nfs_io::rdlock(std::string const &sid) const
{
	thread_io::rdlock(sid);
	if(!flock(fid,F_RDLCK,lock_id(sid)))
		throw cppcms_error(errno,"storage::nfs_io::fcntl::READ LOCK");
}
void nfs_io::unlock(std::string const &sid) const
{
	flock(fid,F_UNLCK,lock_id(sid));
	thread_io::unlock(sid);
}


pthread_rwlock_t *thread_io::create_locks()
{
	pthread_rwlock_t *array = new pthread_rwlock_t [LOCK_SIZE];
	for(int i=0;i<LOCK_SIZE;i++) {
		if(pthread_rwlock_init(array+i,NULL)) {
			int err=errno;
			i--;
			for(;i>=0;i--) {
				pthread_rwlock_destroy(array+i);
			}
			delete [] array;
			throw cppcms_error(err,"storage:pthread_rwlock_init");
		}
	}
	return array;
}

thread_io::thread_io(std::string dir) :
	local_io(dir,create_locks())
{
}

thread_io::~thread_io()
{
	for(int i=0;i<LOCK_SIZE;i++) {
		pthread_rwlock_destroy(locks+i);
	}
	delete [] locks;
}

#ifdef HAVE_PTHREADS_PSHARED

pthread_rwlock_t *shmem_io::create_locks()
{
	int size=sizeof(pthread_rwlock_t)*LOCK_SIZE;
	void *ptr=mmap(0,size,PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, 0,0);
	if(ptr==MAP_FAILED) {
		throw cppcms_error(errno,"storage:mmap");
	}
	pthread_rwlock_t *array =(pthread_rwlock_t*)ptr;
	for(int i=0;i<LOCK_SIZE;i++) {
		pthread_rwlockattr_t attr;
		if(	pthread_rwlockattr_init(&attr) != 0 
			|| pthread_rwlockattr_setpshared(&attr,PTHREAD_PROCESS_SHARED) != 0
			|| pthread_rwlock_init(array+i,&attr) !=0)
		{
			int err=errno;
			i--;
			for(;i>=0;i--) {
				pthread_rwlock_destroy(array+i);
			}
			munmap(ptr,size);
			throw cppcms_error(err,"storage:pthread_rwlock_init");
		}
		pthread_rwlockattr_destroy(&attr);
	}
	creator_pid=getpid();
	return array;
}

shmem_io::shmem_io(std::string dir) :
	local_io(dir,create_locks())
{
}

shmem_io::~shmem_io()
{
	if(creator_pid==getpid()) {
		for(int i=0;i<LOCK_SIZE;i++) {
			pthread_rwlock_destroy(locks+i);
		}
	}
	munmap(locks,sizeof(*locks)*LOCK_SIZE);
}

#endif // HAVE_PTHREADS_PSHARED

} // namespace storeage

void session_file_storage::save(string const &sid,time_t timeout,string const &in)
{
	try {
		io->wrlock(sid);
		io->write(sid,timeout,in.data(),in.size());
		io->unlock(sid);
	}
	catch(...) {
		io->unlock(sid);
		throw;
	}
}

bool session_file_storage::load(string const &sid,time_t *timeout,string &out)
{
	try {
		io->rdlock(sid);
		vector<unsigned char> data;
		time_t tmp;
		bool res=io->read(sid,tmp,&data);
		io->unlock(sid);
		if(timeout) *timeout=tmp;
		out.assign(data.begin(),data.end());
		return res;
	}
	catch(...) {
		io->unlock(sid);
		throw;
	}
}

void session_file_storage::remove(string const &sid)
{
	try {
		io->wrlock(sid);
		io->unlink(sid);
		io->unlock(sid);
	}
	catch(...) {
		io->unlock(sid);
		throw;
	}
	
}

void session_file_storage::gc(boost::shared_ptr<storage::io> io)
{
	DIR *d=NULL;
	try{
		string dir=io->get_dir();
		if((d=opendir(dir.c_str()))==NULL) {
			int err=errno;
			throw cppcms_error(err,"Failed to open directory :"+dir);
		}
		struct dirent entry,*entry_p;
		while(readdir_r(d,&entry,&entry_p)==0 && entry_p!=NULL) {
			int i;
			for(i=0;i<32;i++) {
				if(!isxdigit(entry.d_name[i]))
					break;
			}
			if(i!=32 || entry.d_name[i]!=0) 
				continue;
			string sid=entry.d_name;
			try{
				io->rdlock(sid);
				time_t tmp;
				io->read(sid,tmp,NULL);
				// if it timeouted -- will be removed
				io->unlock(sid);
			}
			catch(...) {
				io->unlock(sid);
				throw;
			}
			
		}
		closedir(d);
	}
	catch(...) {
		if(d) closedir(d);
		throw;
	}
}


#ifndef NO_BUILDER_INTERFACE

namespace {

	static pthread_mutex_t gc_mutex=PTHREAD_MUTEX_INITIALIZER;
	static pthread_cond_t  gc_cond=PTHREAD_COND_INITIALIZER;
	static int gc_exit=-1;
	static int gc_period=-1;
	static int starter_pid=-1;

	void *thread_func(void *param);

struct builder_impl : private boost::noncopyable {
public:
	boost::shared_ptr<storage::io> io;
	bool has_thread;
	pthread_t pid;
	int cache;

	string def_dir()
	{
		char const *d=getenv("TEMP");
		if(!d) d=getenv("TMP");
		if(!d) d="/tmp";
		string dir=string(d)+"/cppcms_sessions";
		if(::mkdir(dir.c_str(),0777)!=0 && errno!=EEXIST)
			throw cppcms_error(errno,"session::file_storage::mkdir");
		return dir;
	}
	
	builder_impl(cppcms_config const config)
	{
		gc_exit=-1;
		cache=config.ival("session.server_enable_cache",0);
		string dir=config.sval("session.files_dir","");
		if(dir=="") {
			dir=def_dir();
		}
		string mod = config.sval("server.mod","");
		string default_type;
		if(mod=="thread")
			default_type = "thread";
#ifdef HAVE_PTHREADS_PSHARED
		else if(mod=="prefork")
			default_type = "prefork";
#endif
		else 
			default_type = "nfs";
		string type=config.sval("session.files_comp",default_type);
		if(type=="thread")
			io.reset(new storage::thread_io(dir));
#ifdef HAVE_PTHREADS_PSHARED
		else if(type=="prefork")
			io.reset(new storage::shmem_io(dir));
#endif
		else if(type=="nfs")
			io.reset(new storage::nfs_io(dir));
		else
			throw cppcms_error("Unknown option for session.files_comp `"+type+"'");
		// Clean first time
		session_file_storage::gc(io);

		gc_period=config.ival("session.files_gc_frequency",-1);

		if(gc_period>0) {
			has_thread=true;
			gc_exit=0;
			starter_pid=getpid();
			pthread_create(&pid,NULL,thread_func,this);
		}
		else
			has_thread=false;

	}
	~builder_impl()
	{
		if(has_thread) {
			pthread_mutex_lock(&gc_mutex);
			gc_exit=1;
			pthread_cond_signal(&gc_cond);
			pthread_mutex_unlock(&gc_mutex);
			pthread_join(pid,NULL);
		}
	}

	boost::shared_ptr<session_api> operator()(worker_thread &w)
	{
		boost::shared_ptr<session_server_storage> ss(new session_file_storage(io));
		return boost::shared_ptr<session_sid>(new session_sid(ss,cache));
	}
};

struct builder {
	boost::shared_ptr<builder_impl> the_builder;

	builder(boost::shared_ptr<builder_impl> b) :
		the_builder(b)
	{
	}
	boost::shared_ptr<session_api> operator()(worker_thread &w)
	{
		return (*the_builder)(w);
	}

};

void *thread_func(void *param)
{
	builder_impl *blder=(builder_impl*)param;
	int exit=0;
	while(!exit) {
		pthread_mutex_lock(&gc_mutex);
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec+=gc_period;
		pthread_cond_timedwait(&gc_cond,&gc_mutex,&ts);
		if(starter_pid!=getpid() || gc_exit) exit=1;
		pthread_mutex_unlock(&gc_mutex);
		if(!exit) {
			try {
				session_file_storage::gc(blder->io);
			}
			catch(std::exception const &e) {
				fprintf(stderr,"%s\n",e.what());
				return NULL;
			}
			catch(...)
			{
				return NULL;
			}
		}
	}
	return NULL;
}

} // anon namespace


session_backend_factory session_file_storage::factory(cppcms_config const &conf)
{
	return builder(boost::shared_ptr<builder_impl>(new builder_impl(conf)));
}

#else // !NO_BUILDER_INTERFACE 

session_backend_factory session_file_storage::factory(cppcms_config const &conf)
{
	throw runtime_error("session_file_storage::factory should not be used");
}

#endif 



} // namespace cppcms
