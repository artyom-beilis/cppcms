#define CPPCMS_SOURCE
#include "session_win32_file_storage.h"
#include "cppcms_error.h"
#include "config.h"

#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/crc.hpp>
#else // Internal Boost
#   include <cppcms_boost/crc.hpp>
    namespace boost = cppcms_boost;
#endif

#include <windows.h>

namespace cppcms {
namespace sessions {

struct session_file_storage::data {};

session_file_storage::session_file_storage(std::string path) :
	memory_(MAP_FAILED)
{
	if(path.empty()){
		if(::getenv("TEMP"))
			path_=std::string(::getenv("TEMP")) + "/cppcms_sessions";
		else if(::getenv("TMP"))
			path_=std::string(::getenv("TMP")) + "/cppcms_sessions";
		else
			path_ = "C:/TEMP";
	}
	else
		path_=path;

	if(!CreateDirectory(path_.c_str(),NULL)) {
		if(GetLastError()!=ERROR_ALREADY_EXISTS) {
			throw cppcms_error("Failed to create a directory for session storage " + path_);
		}
	}
}

session_file_storage::~session_file_storage()
{
}

std::string session_file_storage::file_name(std::string const &sid)
{
	return path_ + "/" + sid;
}

class session_file_storage::locked_file {
public:
	locked_file(session_file_storage *object,std::string sid) :
		h_(INVALID_HANDLE_VALUE)
	{
		name_=object->file_name(sid);
		int sleep_time=0;
		
		for(;;) {
			h_=::CreateFile(name_.c_str(),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					NULL,
					OPEN_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
			if(h_==INVALID_HANDLE_VALUE && GetLastError()==ERROR_ACCESS_DENIED && sleep_time<1000 ) {
				::Sleep(sleep_time);
				sleep_time = sleep_time == 0 ? 1 : sleep_time * 2;				
				continue;
			}
			else
				throw cppcms_error("Failed to open file:" + name_);
		}
		
		OVERLAPPED ov = {0};
		
		if(!::LockFileEx(h_,LOCKFILE_EXCLUSIVE_LOCK,0,0,16,&ov)) {
			::CloseHandle(h_);
			h_=INVALID_HANDLE_VALUE;
			throw cppcms_error("Failed to lock file:"+name_);
		}

	}
	~locked_file()
	{
		if(h_==INVALID_HANDLE_VALUE)
			return;
		OVERLAPPED ov = {0};
		::UnlockFileEx(h_,0,0,16,&ov);
		::CloseHandle(h_);
	}
	int handle() { return h_; }
	std::string name() { return name_; }
private:
	HANDLE h_;
	std::string name_;
};


void session_file_storage::save(std::string const &sid,time_t timeout,std::string const &in)
{
	locked_file file(this,sid);
	save_to_file(file.handle(),timeout,in);
}

bool session_file_storage::load(std::string const &sid,time_t &timeout,std::string &out)
{
	locked_file file(this,sid);
	if(!read_from_file(file.handle(),timeout,out)) {
		::DeleteFile(file.name().c_str());
		return false;
	}
	return true;
}

void session_file_storage::remove(std::string const &sid)
{
	locked_file file(this,sid);
	::DeleteFile(file.name().c_str());
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
