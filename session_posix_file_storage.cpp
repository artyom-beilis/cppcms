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
#include <sys/mman.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "posix_util.h"

namespace cppcms {
namespace sessions {

struct session_file_storage::data {};

using namespace cppcms::impl;

session_file_storage::session_file_storage(std::string path,int concurrency_hint,int procs_no,bool force_flock) :
	memory_(MAP_FAILED)
{
	if(path.empty()){
		if(::getenv("TEMP"))
			path_=std::string(::getenv("TEMP")) + "/cppcms_sessions";
		else if(::getenv("TMP"))
			path_=std::string(::getenv("TMP")) + "/cppcms_sessions";
		else
			path_ = "/tmp/cppcms_sessions";
	}
	else
		path_=path;

	if(::mkdir(path_.c_str(),0777) < 0) {
		if(errno!=EEXIST) {
			int err=errno;
			throw cppcms_error(err,"Failed to create a directory for session storage " + path_);
		}
	}
		
	lock_size_ = concurrency_hint;
	if(force_flock || (procs_no > 1 && !test_pthread_mutex_pshared()))
		file_lock_=true;
	else
		file_lock_=false;
	if(!file_lock_) {
		#ifdef MAP_ANONYMOUS
		int flags = MAP_ANONYMOUS | MAP_SHARED;
		#else // defined(MAP_ANON)
		int flags = MAP_ANON | MAP_SHARED;
		#endif
		memory_ = ::mmap(0,sizeof(pthread_mutex_t) * lock_size_,PROT_READ | PROT_WRITE,flags,-1,0);
		if(memory_ == MAP_FAILED)
			throw cppcms_error(errno,"Memory map failed:");
		locks_ = reinterpret_cast<pthread_mutex_t *>(memory_);
		for(unsigned i=0;i<lock_size_;i++)
			create_mutex(locks_+i,true);
	}
	else {
		mutexes_.resize(lock_size_);
		locks_ = &mutexes_.front();
		for(unsigned i=0;i<lock_size_;i++)
			create_mutex(locks_+i,false);
	}
}

session_file_storage::~session_file_storage()
{
	if(memory_ !=MAP_FAILED) {
		for(unsigned i=0;i<lock_size_;i++)
			destroy_mutex(reinterpret_cast<pthread_mutex_t *>(memory_) + i);
		munmap(memory_,sizeof(pthread_mutex_t) * lock_size_);
	}
	else {
		for(unsigned i=0;i<lock_size_;i++)
			destroy_mutex(&mutexes_[i]);
	}
}

pthread_mutex_t *session_file_storage::sid_to_pos(std::string const &sid)
{
	char buf[5] = { sid[0],sid[1],sid[2],sid[3],0};
	unsigned pos;
	sscanf(buf,"%x",&pos);
	return locks_ + (pos % lock_size_); 
}

void session_file_storage::lock(std::string const &sid)
{
	pthread_mutex_lock(sid_to_pos(sid));
}

void session_file_storage::unlock(std::string const &sid)
{
	pthread_mutex_unlock(sid_to_pos(sid));
}

std::string session_file_storage::file_name(std::string const &sid)
{
	return path_ + "/" + sid;
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
		if(!object_->file_lock_)
			return;
		for(;;) {
			if(create)
				fd_=::open(name_.c_str(),O_CREAT | O_RDWR,0666);
			else
				fd_=::open(name_.c_str(),O_RDWR);

			if(fd_ < 0) return;

			struct flock lock;
			memset(&lock,0,sizeof(lock));
			lock.l_type=F_WRLCK;
			lock.l_whence=SEEK_SET;
			int res;
			while((res=fcntl(fd_,F_SETLKW,&lock)!=0) && errno==EINTR)
				;
			if(res < 0) {
				::close(fd_);
				fd_=-1;
			}
			struct stat s_id,s_name;
			if(::stat(name_.c_str(),&s_name) < 0) {
				// looks like file was deleted
				::close(fd_);
				fd_=-1;
				continue;
			}
			if(::fstat(fd_,&s_id) < 0) {
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
			if(object_->file_lock_) {
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

bool session_file_storage::load(std::string const &sid,time_t &timeout,std::string &out)
{
	locked_file file(this,sid,false);
	int fd=file.fd();
	if(fd<0) return false;
	if(!read_from_file(fd,timeout,out)) {
		::unlink(file.name().c_str());
		return false;
	}
	return true;
}

void session_file_storage::remove(std::string const &sid)
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
	return true;
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
		uint32_t crc;
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
	int path_len=pathconf(path_.c_str(),_PC_NAME_MAX);
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
		if((d=::opendir(path_.c_str()))==NULL) {
			int err=errno;
			throw cppcms_error(err,"Failed to open directory :"+path_);
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
				locked_file file(this,sid,false);
				if(file.fd() >=0 && !read_timestamp(file.fd()))
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

struct session_file_storage_factory::data {};

session_file_storage_factory::session_file_storage_factory(std::string path,int conc,int proc_no,bool force_lock) :
	storage_(new session_file_storage(path,conc,proc_no,force_lock))
{
}

session_file_storage_factory::~session_file_storage_factory()
{
}

intrusive_ptr<session_storage> session_file_storage_factory::get()
{
	return storage_;
}

bool session_file_storage_factory::requires_gc()
{
	return true;
}

void session_file_storage_factory::gc_job()
{
	storage_->gc();
}


} // sessions
} // cppcms
