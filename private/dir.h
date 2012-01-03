///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CPPCMS_IMPL_DIR_H
#define CPPCMS_IMPL_DIR_H
#if defined(__sun)
#define _POSIX_PTHREAD_SEMANTICS
#endif

#include <cppcms/defs.h>

#ifdef CPPCMS_WIN_NATIVE
#  include <booster/locale/encoding.h>

#  ifndef NOMINMAX
#    define NOMINMAX
#  endif

#  include <windows.h>
#else

#  include <stdlib.h>
#  include <sys/types.h>
#  include <dirent.h>
#  include <unistd.h>
#endif

#include <booster/noncopyable.h>
#include <string>
#include <assert.h>

namespace cppcms {
namespace impl {

class directory  : public booster::noncopyable {
public:
	directory();
	~directory();
	bool open(std::string const &dir_name);
	void close();
	bool next();
	char const *name();
private:
	#ifdef CPPCMS_WIN_NATIVE
	HANDLE dir_;
	_WIN32_FIND_DATAW entry_;
	std::string utf_name_;
	#else  // POSIX
	DIR *dir_;
	#if defined(__linux) || defined(__FreeBSD__) || defined(__APPLE__)
	struct dirent de_;
	#else
	#define CPPCMS_DIR_ALLOCATE_DE
	struct dirent *de_;
	#endif
	struct dirent *entry_p_;
	#endif
};

inline directory::~directory()
{
	close();
}

#ifdef CPPCMS_WIN_NATIVE
inline directory::directory() : dir_(INVALID_HANDLE_VALUE)
{

}

inline void directory::close()
{
	if(dir_!=INVALID_HANDLE_VALUE) {
		FindClose(dir_);
		dir_ = INVALID_HANDLE_VALUE;
	}
}

inline bool directory::open(std::string const &name)
{
	std::wstring search = booster::locale::conv::utf_to_utf<wchar_t>(name.c_str()) +L"/*";
	dir_ = FindFirstFileW(search.c_str(),&entry_);
	if(dir_ == INVALID_HANDLE_VALUE) {
		if(GetLastError() == ERROR_FILE_NOT_FOUND)
			return true;
		else
			return false;
	}
	return true;
}

inline bool directory::next()
{
	if(dir_ == INVALID_HANDLE_VALUE)
		return false;
	utf_name_  = booster::locale::conv::utf_to_utf<char>(entry_.cFileName);
	if(!FindNextFileW(dir_,&entry_)) {
		close();
	}
	return true;
}

inline char const *directory::name()
{
	return utf_name_.c_str();
}

#else  // POSIX

inline directory::directory() : dir_(0), entry_p_(0)
{
#ifdef CPPCMS_DIR_ALLOCATE_DE
	de_ = 0;
#endif
}




inline bool directory::open(std::string const &dir_name)
{
	close();
	dir_ = opendir(dir_name.c_str());

	if(!dir_)
		return false;
	#ifdef CPPCMS_DIR_ALLOCATE_DE
	//
	// This pathconf/opendir exploit can be used with only file systems
	// with small file names like fat... So we require
	// at least 4K so it would be impossible to exploit 
	// this
	//
	int name_len = pathconf(dir_name.c_str(),_PC_NAME_MAX);
	if(name_len < 4096) // -1 or small value
		name_len = 4096; // guess
	#ifdef NAME_MAX
	if(name_len < NAME_MAX)
		name_len = NAME_MAX;
	#endif
	de_ = static_cast<struct dirent*>(malloc(sizeof(struct dirent) + name_len + 1 - sizeof(de_->d_name)));
	if(!de_)
		throw std::bad_alloc();
	#endif // alloc de
	return true;
}

inline void directory::close()
{
	#ifdef CPPCMS_DIR_ALLOCATE_DE
	if(de_)  {
		free(de_);
		de_ = 0;
	}
	#endif
	entry_p_ = 0;
	if(dir_) {
		closedir(dir_);
		dir_=0;
	}
}

inline bool directory::next()
{
	assert(dir_);
	#ifdef CPPCMS_DIR_ALLOCATE_DE
	struct dirent *d = de_;
	#else
	struct dirent *d = &de_;
	#endif
	return readdir_r(dir_,d,&entry_p_) == 0 && entry_p_ != 0;
}

inline char const *directory::name()
{
	assert(entry_p_);
	return entry_p_->d_name;
}

#endif // POSIX

} // impl
} // cppcms
#endif
