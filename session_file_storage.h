#ifndef SESSION_FILE_STORAGE_H
#define SESSION_FILE_STORAGE_H

#include <string>
#include <vector>
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/noncopyable.hpp>
#   include <boost/shared_ptr.hpp>
#else // Internal Boost
#   include <cppcms_boost/noncopyable.hpp>
#   include <cppcms_boost/shared_ptr.hpp>
    namespace boost = cppcms_boost;
#endif
#include <pthread.h>
#include "session_storage.h"
#include "session_backend_factory.h"
#include "config.h"


namespace cppcms {
class cppcms_config;

namespace storage {

class io : private boost::noncopyable {
protected:
	std::string dir;
	int lock_id(std::string const &sid) const;
	std::string mkname(std::string const &sid) const;
	virtual void close(int fid) const;
public:
	std::string const &get_dir() const { return dir; }	
	io(std::string d) : dir(d) {}
	virtual void wrlock(std::string const &sid) const = 0;
	virtual void rdlock(std::string const &sid) const = 0;
	virtual void unlock(std::string const &sid) const = 0;
	virtual void write(std::string const &sid,time_t timestamp,void const *buf,size_t len) const;
	virtual bool read(std::string const &sid,time_t &timestamp,std::vector<unsigned char> *out) const;
	virtual void unlink(std::string const &sid) const;
	virtual ~io(){};
};

#if !defined(CPPCMS_EMBEDDED)

class local_io : public io {
protected:
	pthread_rwlock_t *locks;
public:
	local_io(std::string dir,pthread_rwlock_t *l);
	virtual void wrlock(std::string const &sid) const;
	virtual void rdlock(std::string const &sid) const;
	virtual void unlock(std::string const &sid) const;
};

class thread_io : public local_io
{
	pthread_rwlock_t *create_locks();
public:
	thread_io(std::string dir);
	~thread_io();
};

class nfs_io : public thread_io {
	int fid;
protected:
	virtual void close(int fid) const;
public:
	nfs_io(std::string dir);
	virtual void wrlock(std::string const &sid) const;
	virtual void rdlock(std::string const &sid) const;
	virtual void unlock(std::string const &sid) const;
	~nfs_io();
};


#ifdef HAVE_PTHREADS_PSHARED

class shmem_io : public local_io
{
	int creator_pid;
	pthread_rwlock_t *create_locks();
public:
	shmem_io(std::string dir);
	~shmem_io();
};

#endif

#else // !CPPCMS_EMBEDDED

class embed_io : public io {
	int fid;
public:
	embed_io(std::string dir);
	virtual void wrlock(std::string const &sid) const;
	virtual void rdlock(std::string const &sid) const;
	virtual void unlock(std::string const &sid) const;
	~embed_io();
};


#endif // CPPCMS_EMBEDDED

} // storage

class session_file_storage : public session_server_storage {
	boost::shared_ptr<storage::io> io;
public:
	static void gc(boost::shared_ptr<storage::io>);
	static session_backend_factory factory(cppcms_config const &);
	session_file_storage(boost::shared_ptr<storage::io> io_) : io(io_) {}
	virtual void save(std::string const &sid,time_t timeout,std::string const &in);
	virtual bool load(std::string const &sid,time_t *timeout,std::string &out);
	virtual void remove(std::string const &sid) ;
	virtual ~session_file_storage(){};
};

} // cppcms


#endif
