///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_SESSION_POSIX_FILE_STORAGE_H
#define CPPCMS_SESSION_POSIX_FILE_STORAGE_H

#include <cppcms/defs.h>
#include <cppcms/session_storage.h>
#include <booster/hold_ptr.h>
#include <vector>


namespace cppcms {
namespace sessions {

	class session_file_storage_factory;

	class CPPCMS_API session_file_storage : public session_storage {
	public:
		session_file_storage(std::string path);
		virtual ~session_file_storage();
		virtual void save(std::string const &sid,time_t timeout,std::string const &in);
		virtual bool load(std::string const &sid,time_t &timeout,std::string &out);
		virtual void remove(std::string const &sid);
		virtual bool is_blocking();
	private:
		struct locked_file;
		struct _data;
		bool read_timestamp(void *h);
		bool read_from_file(void *h,time_t &timeout,std::string &data);
		void save_to_file(void *h,time_t timeout,std::string const &in);
		bool read_all(void *,void *vbuf,int n);
		bool write_all(void *,void const *vbuf,int n);
		void gc();
		std::string file_name(std::string const &sid);

		// members
	
		booster::hold_ptr<_data> d;

		std::string path_;
		// friends 
		friend struct locked_file;
		friend class session_file_storage_factory;
	};

	class CPPCMS_API session_file_storage_factory : public session_storage_factory {
	public:
		session_file_storage_factory(std::string path);
		virtual booster::shared_ptr<session_storage> get();
		virtual bool requires_gc();
		virtual void gc_job();
		virtual ~session_file_storage_factory();
	private:
		struct _data;
		booster::shared_ptr<session_file_storage> storage_;
	};
} // sessions
} // cppcms

#endif
