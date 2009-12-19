#define CPPCMS_SOURCE
#include "session_posix_file_storage.h"
#include "cppcms_error.h"
#include "config.h"

#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/crc.hpp>
#else // Internal Boost
#   include <cppcms_boost/crc.hpp>
    namespace boost = cppcms_boost;
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>



namespace cppcms {
namespace sessions {

bool session_file_storage::test_pshared()
{
#if !defined(MAP_ANONYMOUS) && !defined(MAP_ANON)
	return false;
#else
	#ifdef MAP_ANONYMOUS
		int flags = MAP_ANONYMOUS | MAP_SHARED;
	#else // defined(MAP_ANON)
		int flags = MAP_ANON | MAP_SHARED;
	#endif
	void *memory=mmap(0,sizeof(pthread_mutex_t),PROT_READ | PROT_WRITE, flags, -1, 0);
	if(memory==MAP_FAILED)
		return false;
	try {
		create_mutex(reinterpret_cast<pthread_mutex_t *>(memory));
		destroy_mutex(reinterpret_cast<pthread_mutex_t *>(memory));
	}
	catch(cppcms_error const &e) {
		munmap(memory,sizeof(pthread_mutex_t));
		return false;
	}
	munmap(memory,sizeof(pthread_mutex_t));
	return true;
#endif
}


void session_file_storage::create_mutex(pthread_mutex_t *m,bool pshared)
{
	if(!pshared) {
		pthread_mutex_init(m,0);
	}
	else {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		try {
			int res;
			res = pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
			if(res==0)
				res = pthread_mutex_init(m,&attr);
			if(res < 0)
				throw cppcm_error(errno,"Failed to create process shared mutex");
			pthread_mutexattr_destroy(&attr);
		}
		catch(...) {
			pthread_mutexattr_destroy(&attr);
			throw;
		}
	}
}

void session_file_storage::destroy_mutex(pthread_mutex_t *m)
{
	pthread_mutex_destroy(mutex_);
}

session_file_storage::session_file_storage(std::string path,int concurrency_hint,int procs_no,bool force_flock)
{
	path_=path;
	lock_size_ = concurrency_hint * 2;
	if(force_flock || procs_no > 1 && !test_pshared())
		file_lock_=true;
	else
		file_lock_=false;
}

class session_file_storage::locked_file {
public:
	locked_file(session_file_storage *object,std::string sid,bool create) :
		object_(object),
		sid_(sid),
		fd_(-1)
	{
		name_=object_->file_name(sid);
		object_->lock(sid_);
		if(!object_->file_locking())
			return;
		for(;;) {
			if(create)
				fd_=::open(name.c_str(),O_CREAT | O_RDWR,0666);
			else
				fd_=::open(name.c_str(),O_RDWR);

			if(fd < 0) return;

			struct flock lock;
			memset(&lock,0,sizeof(lock));
			lock.l_type=F_WRLCK;
			lock.l_whence=SEEK_SET;
			int res;
			while((res=fcntl(fd_,F_SETLKW,&lock)!=0) && errno==EINTR)
				;
			if(res < 0) {
				::close(fd_)
				fd_=-1;
			}
			struct stat s_id,s_name;
			if(::stat(file_name.c_str(),&s_name) < 0) {
				// looks like file was deleted
				::close(fd_);
				fd_=-1;
				continue;
			}
			if(::fstat(fd_&s_id) < 0) {
				::close(fd_);
				fd_=-1;
				return;
			}
			if(s_id.st_ino!=s_name.st_ino || s_id.st_dev!=s_name.st_dev) {
				::close(fd_);
				fd_=-1;
				continue;
			}
			return;
		}

	}
	~locked_file()
	{
		if(fd_>=0) {
			if(object_->file_locking()) {
				struct flock lock;
				memset(&lock,0,sizeof(lock));
				lock.l_type=F_UNLCK;
				lock.l_whence=SEEK_SET;
				int res;
				while((res=fcntl(fd_,F_SETLKW,&lock)!=0) && errno==EINTR)
					;
			}
			::close(fd_);
		}
		object_->unlock(sid_);
	}
	int fd() { return fd_; }
	std::string name() { return name_; }
private:
	session_file_storage *object_;
	std::string sid_;
	int fd_;
	std::string name_;
};


void session_file_storage::save(std::string const &sid,time_t timeout,std::string const &in)
{
	locked_file file(this,sid,true);
	int fd=file.fd();
	if(fd<0)
		throw cppcms_error(errno,"failed to create session file");
	save_to_file(fd,timeout,in);
}

void session_file_storage::load(std::string const &sid,time_t &timeout,std::string &out)
{
	locked_file file(this,sid,false);
	int fd=file.fd();
	if(fd<0) return false;
	if(!read_from_file(fd,timeout,out)) {
		::unlink(file.name().c_str());
		return false;
	}
	return false;
}

void session_file_storage::remove(std:string const &sid)
{
	locked_file file(this,sid,false);
	if(file.fd() >= 0)
		::unlink(file.name().c_str());
}

bool session_file_storage::read_timestamp(int fd)
{
	::lseek(fd,0,SEEK_SET);
	int64_t stamp;
	if(!read_all(fd,&stamp,sizeof(stamp)) || stamp < ::time(0))
		return false;
}

bool session_file_storage::read_from_file(int fd,time_t &timeout,std::string &data)
{
	int64_t f_timeout;
	uint32_t crc;
	uint32_t size;
	::lseek(fd,0,SEEK_SET);
	if(!read_all(fd,&f_timeout,sizeof(f_timeout)))
		return false;
	if(f_timeout < time(0))
		return false;
	if(!read_all(fd,&crc,sizeof(crc)) || !read_all(fd,&size,sizeof(size)))
		return false;
	std::vector<char> buffer(size,0);
	if(!read_all(fd,&buffer.front(),size))
		return false;
	boost::crc_32_type crc_calc;
	crc_calc.process_bytes(&buffer.front(),size);
	uint32_t real_crc=crc_calc.checksum();
	if(crc != real_crc)
		return false;
	timeout=f_timeout;
	data.assign(&buffer.front(),size);
	return true;
}

void session_file_storage::save_to_file(int fd,time_t timeout,std::string const &in)
{
	struct {
		int64_t timeout;
		uint32_t ctc;
		uint32_t size;
	} tmp = { timeout, 0, in.size() };
	boost::crc_32_type crc_calc;
	crc_calc.process_bytes(in.data(),in.size());
	tmp.crc=crc_calc.checksum();
	if(!write_all(fd,&tmp,sizeof(tmp)) || !write_all(fd,in.data(),in.size()))
		throw cppcms_error(errno,"Failed to write to file");
}

bool session_file_storage::write_all(int fd,void const *vbuf,int n)
{
	char const *buf=reinterpret_cast<char const *>(vbuf);
	while(n > 0) {
		int res = ::write(fd,buf,n);
		if(res < 0 && errno==EINTR)
			continue;
		if(res <= 0)
			return false;
		n-=res;
	}
	return true;
}

bool session_file_storage::read_all(int fd,void *vbuf,int n)
{
	char *buf=reinterpret_cast<char *>(vbuf);
	while(n > 0) {
		int res = ::read(fd,buf,n);
		if(res < 0 && errno==EINTR)
			continue;
		if(res <= 0)
			return false;
		n-=res;
	}
	return true;
}

void session_file_storage::gc()
{
	DIR *d=0;
	struct dirent *entry_st=0,*entry_p;
	int path_len=pathconf(directory_.c_str(),_PC_NAME_MAX);
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
		if((d=::opendir(directory_.c_str()))==NULL) {
			int err=errno;
			throw cppcms_error(err,"Failed to open directory :"+dir);
		}
		while(::readdir_r(d,entry_st,&entry_p)==0 && entry_p!=NULL) {
			int i;
			for(i=0;i<32;i++) {
				if(!isxdigit(entry_st->d_name[i]))
					break;
			}
			if(i!=32 || entry_st->d_name[i]!=0) 
				continue;
			std::string sid=entry_st->d_name;
			{
				locked_file file(this,sid);
				if(file.fd() >=0 && !read_timestamp(fd))
					::unlink(file.name().c_str());
			}
		}
		::closedir(d);
	}
	catch(...) {
		if(d) ::closedir(d);
		delete [] entry_st;
		throw;
	}
}


} // sessions
} // cppcms
