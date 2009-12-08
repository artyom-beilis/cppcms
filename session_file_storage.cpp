#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/crc.hpp>
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/crc.hpp>
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif
#include <limits.h>
#include "global_config.h"

#include "session_file_storage.h"
#include "cppcms_error.h"
#include "session_sid.h"

#include "config.h"

using namespace std;

namespace cppcms {

namespace {

bool flock(int fid,int how,int pos=-1)
{
	struct flock lock;
	memset(&lock,0,sizeof(lock));
	lock.l_type=how;
	lock.l_whence=SEEK_SET;
	if(pos<0) { // All file
		lock.l_start=pos;
		lock.l_len=1;
	}
	int res;
	while((res=fcntl(fid,F_SETLKW,&lock)!=0) && errno==EINTR)
		;
	if(res) return false;
	return true;	
}

} // anon namespace

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

#if !defined(CPPCMS_EMBEDDED)

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

void nfs_io::close(int fid) const
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
	void *ptr=mmap(0,size,PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1,0);
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

#else // CPPCMS_EMBEDDED

#if defined(CPPCMS_EMBEDDED_THREAD) 
static pthread_rwlock_t cppcms_embed_io_lock = PTHREAD_RWLOCK_INITIALIZER;
#define embed_wrlock() pthread_rwlock_wrlock(&cppcms_embed_io_lock)
#define embed_rdlock() pthread_rwlock_rdlock(&cppcms_embed_io_lock)
#define embed_unlock() pthread_rwlock_unlock(&cppcms_embed_io_lock)
#else

#define embed_wrlock() 
#define embed_rdlock() 
#define embed_unlock() 

#endif

embed_io::embed_io(std::string dir) : io(dir)
{
	string lockf=dir+"/"+"flock";
	fid=::open(lockf.c_str(),O_CREAT | O_RDWR,0666);
	if(fid<0) {
		throw cppcms_error(errno,"storage::embed_io::open");
	}
}

embed_io::~embed_io()
{
	::close(fid);
}

void embed_io::wrlock(std::string const &sid) const
{
	embed_wrlock();
	if(!flock(fid,F_WRLCK))
		throw cppcms_error(errno,"storage::nfs_io::fcntl::WRITE LOCK");
}
void embed_io::rdlock(std::string const &sid) const
{
	embed_rdlock();
	if(!flock(fid,F_RDLCK))
		throw cppcms_error(errno,"storage::nfs_io::fcntl::READ LOCK");
}
void embed_io::unlock(std::string const &sid) const
{
	flock(fid,F_UNLCK);
	embed_unlock();
}


#endif 

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
	string dir=io->get_dir();
	struct dirent *entry_st=NULL,*entry_p;
	int path_len=pathconf(dir.c_str(),_PC_NAME_MAX);
	if(path_len < 0 ) { 
		// Only "sessions" should be in this directory
		// also this directory has high level of trust
		// thus... Don't care about symlink exploits
		#ifdef NAME_MAX
		path_len=NAME_MAX;
		#elif defined(PATH_MAX)
		path_len=PATH_MAX;
		#else
		path_len=4096; // guess
		#endif
	}
	// this is for Solaris... 
	entry_st=(struct dirent *)new char[sizeof(struct dirent)+path_len+1];
	try{
		if((d=opendir(dir.c_str()))==NULL) {
			int err=errno;
			throw cppcms_error(err,"Failed to open directory :"+dir);
		}
		while(readdir_r(d,entry_st,&entry_p)==0 && entry_p!=NULL) {
			int i;
			for(i=0;i<32;i++) {
				if(!isxdigit(entry_st->d_name[i]))
					break;
			}
			if(i!=32 || entry_st->d_name[i]!=0) 
				continue;
			string sid=entry_st->d_name;
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
		delete [] entry_st;
		throw;
	}
}


#ifndef NO_BUILDER_INTERFACE

namespace {

#if !defined(CPPCMS_EMBEDDED) || defined(CPPCMS_EMBEDDED_THREAD)
	static pthread_mutex_t gc_mutex=PTHREAD_MUTEX_INITIALIZER;
	static pthread_cond_t  gc_cond=PTHREAD_COND_INITIALIZER;
	static int gc_exit=-1;
	static int starter_pid=-1;
#endif
	static int gc_period=-1;

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
		string dir=config.sval("session.files_dir","");
		if(dir=="") {
			dir=def_dir();
		}
#if !defined(CPPCMS_EMBEDDED)
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
#else // EMBEDDED

		io.reset(new storage::embed_io(dir));
#endif

		gc_period=config.ival("session.files_gc_frequency",-1);
#if !defined(CPPCMS_EMBEDDED) || defined(CPPCMS_EMBEDDED_THREAD)
		// Clean first time
		session_file_storage::gc(io);
		
		gc_exit=-1;
		if(gc_period>0) {
			has_thread=true;
			gc_exit=0;
			starter_pid=getpid();
			pthread_create(&pid,NULL,thread_func,this);
		}
		else
			has_thread=false;
#else // We have only cgi
		string timestamp=dir+"/gc.timestamp";
		struct stat info;
		if(stat(timestamp.c_str(),&info)==0 && (time(NULL) - info.st_mtime) < gc_period)
			return;
		fclose(fopen(timestamp.c_str(),"w")); // timestamp it
		session_file_storage::gc(io);
#endif

	}
	~builder_impl()
	{
#if !defined(CPPCMS_EMBEDDED) || defined(CPPCMS_EMBEDDED_THREAD)
		if(has_thread) {
			pthread_mutex_lock(&gc_mutex);
			gc_exit=1;
			pthread_cond_signal(&gc_cond);
			pthread_mutex_unlock(&gc_mutex);
			pthread_join(pid,NULL);
		}
#endif
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
#if !defined(CPPCMS_EMBEDDED) || defined(CPPCMS_EMBEDDED_THREAD)
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
#endif
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
