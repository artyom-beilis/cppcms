///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "session_win32_file_storage.h"
#include <cppcms/cppcms_error.h>
#include <cppcms/config.h>

#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/crc.hpp>
#else // Internal Boost
#   include <cppcms_boost/crc.hpp>
    namespace boost = cppcms_boost;
#endif

#include <memory>
#include <time.h>

#include <windows.h>

#include <sstream>
#include <cppcms/cstdint.h>

namespace cppcms {
namespace sessions {

struct session_file_storage::_data {};

session_file_storage::session_file_storage(std::string path)
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

	if(!::CreateDirectory(path_.c_str(),NULL)) {
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
			if(h_==INVALID_HANDLE_VALUE) {
				if(GetLastError()==ERROR_ACCESS_DENIED && sleep_time<1000 ) {
					::Sleep(sleep_time);
					sleep_time = sleep_time == 0 ? 1 : sleep_time * 2;				
					continue;
				}
				else {
					std::ostringstream tmp;
					tmp << "Failed to open file:" + name_ + "error code " <<std::hex << ::GetLastError();
					throw cppcms_error(tmp.str());
				}
			}
			else
				break;
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
	HANDLE handle() { return h_; }
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

bool session_file_storage::read_timestamp(HANDLE h)
{
	int64_t stamp;
	if(!read_all(h,&stamp,sizeof(stamp)) || stamp < ::time(0))
		return false;
	return true;
}

bool session_file_storage::read_from_file(HANDLE h,time_t &timeout,std::string &data)
{
	int64_t f_timeout;
	uint32_t crc;
	uint32_t size;
	if(!read_all(h,&f_timeout,sizeof(f_timeout)))
		return false;
	if(f_timeout < time(0))
		return false;
	if(!read_all(h,&crc,sizeof(crc)) || !read_all(h,&size,sizeof(size)))
		return false;
	std::vector<char> buffer(size,0);
	boost::crc_32_type crc_calc;
	if(size > 0) {
		if(!read_all(h,&buffer.front(),size))
			return false;
		crc_calc.process_bytes(&buffer.front(),size);
	}
	uint32_t real_crc=crc_calc.checksum();
	if(crc != real_crc)
		return false;
	timeout=f_timeout;
	if(size > 0)
		data.assign(&buffer.front(),size);
	else
		data.clear();
	return true;
}

void session_file_storage::save_to_file(HANDLE h,time_t timeout,std::string const &in)
{
	struct {
		int64_t timeout;
		uint32_t crc;
		uint32_t size;
	} tmp = { timeout, 0, in.size() };
	boost::crc_32_type crc_calc;
	crc_calc.process_bytes(in.data(),in.size());
	tmp.crc=crc_calc.checksum();
	if(!write_all(h,&tmp,sizeof(tmp)) || !write_all(h,in.data(),in.size()))
		throw cppcms_error("Failed to write to file");
}

bool session_file_storage::write_all(HANDLE h,void const *vbuf,int n)
{
	DWORD written;
	if(!::WriteFile(h,vbuf,n,&written,NULL) || written!=unsigned(n))
		return false;
	return true;
}

bool session_file_storage::read_all(HANDLE h,void *vbuf,int n)
{
	DWORD read;
	if(!::ReadFile(h,vbuf,n,&read,NULL) || read!=unsigned(n))
		return false;
	return true;
}

void session_file_storage::gc()
{
	std::auto_ptr<WIN32_FIND_DATA> entry(new WIN32_FIND_DATA);
	HANDLE d=INVALID_HANDLE_VALUE;
	std::string search_path = path_ + "/*";
	try{
		if((d=::FindFirstFile(search_path.c_str(),entry.get()))==INVALID_HANDLE_VALUE) {
			if(GetLastError() == ERROR_FILE_NOT_FOUND)
				return;
			throw cppcms_error("Failed to open directory :"+path_);
		}
		do {
			int i;
			for(i=0;i<32;i++) {
				if(!isxdigit(entry->cFileName[i]))
					break;
			}
			if(i!=32 || entry->cFileName[i]!=0) 
				continue;
			std::string sid=entry->cFileName;
			{
				locked_file file(this,sid);
				if(!read_timestamp(file.handle()))
					::DeleteFile(file.name().c_str());
			}
		} while(::FindNextFile(d,entry.get()));
		::FindClose(d);
	}
	catch(...) {
		if(d!=INVALID_HANDLE_VALUE) ::FindClose(d);
		throw;
	}
}

struct session_file_storage_factory::_data {};

session_file_storage_factory::session_file_storage_factory(std::string path) :
	storage_(new session_file_storage(path))
{
}

session_file_storage_factory::~session_file_storage_factory()
{
}

booster::shared_ptr<session_storage> session_file_storage_factory::get()
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
