#ifndef CPPCMS_SESSION_POSIX_FILE_STORAGE_H
#define CPPCMS_SESSION_POSIX_FILE_STORAGE_H

#include "defs.h"
#include "session_storage.h"
#include "hold_ptr.h"
#include <pthread.h>
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
	private:
		struct locked_file;
		struct data;
		bool read_timestamp(HANDLE h);
		bool read_from_file(HANDLE h,time_t &timeout,std::string &data);
		void save_to_file(HANDLE h,time_t timeout,std::string const &in);
		void gc();
		std::string file_name(std::string const &sid);

		// members
	
		util::hold_ptr<data> d;

		std::string path_;
		// friends 
		friend struct locked_file;
		friend class session_file_storage_factory;
	};

	class CPPCMS_API session_file_storage_factory : public session_storage_factory {
	public:
		session_file_storage_factory(std::string path);
		virtual intrusive_ptr<session_storage> get();
		virtual bool requires_gc();
		virtual void gc_job();
		virtual ~session_file_storage_factory();
	private:
		struct data;
		intrusive_ptr<session_file_storage> storage_;
	};
} // sessions
} // cppcms

#endif
