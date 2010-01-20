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
		session_file_storage(std::string path,int concurrency_hint,int procs_no,bool force_flock);
		virtual ~session_file_storage();
		virtual void save(std::string const &sid,time_t timeout,std::string const &in);
		virtual bool load(std::string const &sid,time_t &timeout,std::string &out);
		virtual void remove(std::string const &sid);
	private:
		struct locked_file;
		struct data;
		bool read_timestamp(int fd);
		bool read_from_file(int fd,time_t &timeout,std::string &data);
		void save_to_file(int fd,time_t timeout,std::string const &in);
		bool read_all(int fd,void *vbuf,int n);
		bool write_all(int fd,void const *vbuf,int n);
		void gc();
		std::string file_name(std::string const &sid);
		pthread_mutex_t *sid_to_pos(std::string const &sid);
		void lock(std::string const &sid);
		void unlock(std::string const &sid);

		// members
	
		util::hold_ptr<data> d;

		void *memory_;
		std::string path_;
		unsigned lock_size_;
		bool file_lock_;
		pthread_mutex_t *locks_;
		std::vector<pthread_mutex_t> mutexes_;

		// friends 
		friend struct locked_file;
		friend class session_file_storage_factory;
	};

	class CPPCMS_API session_file_storage_factory : public session_storage_factory {
	public:
		session_file_storage_factory(std::string path,int conc,int proc_no,bool force_lock);
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
