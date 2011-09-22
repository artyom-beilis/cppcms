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
#include <cppcms/config.h>
#include <cppcms/session_storage.h>
#include <booster/thread.h>
#include <booster/backtrace.h>
#include <sstream>
#include <iostream>
#include <map>
#include <time.h>

#ifdef CPPCMS_SQLITE_LINK_STATIC
#include <sqlite3.h>
namespace cppcms { namespace sessions { namespace sqlite_session {namespace details {
	void load_sqlite(char const *) {}
}}}}
#else // dynamic loading

#ifdef CPPCMS_WIN_NATIVE
#include <booster/nowide/convert.h>
#include <windows.h>
#else
#include <dlfcn.h>
#endif


extern "C" {

// API We need and we can DLOPEN

#define SQLITE_OK 0
#define SQLITE_BUSY 5
#define SQLITE_LOCKED 6
#define SQLITE_DONE 101
#define SQLITE_ROW 100
#define SQLITE_STATIC ((void (*)(void*))0)

typedef long long int sqlite3_int64;
typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;
static int (*sqlite3_finalize)(sqlite3_stmt *);
static int (*sqlite3_bind_text)(sqlite3_stmt*, int, const char*, int n, void(*)(void*));
static int (*sqlite3_step)(sqlite3_stmt*);
static int (*sqlite3_bind_int64)(sqlite3_stmt*, int, sqlite3_int64);
static int (*sqlite3_bind_int)(sqlite3_stmt*, int, int);
static int (*sqlite3_bind_blob)(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
static sqlite3_int64 (*sqlite3_column_int64)(sqlite3_stmt*, int);
static const void *(*sqlite3_column_blob)(sqlite3_stmt*, int);
static int (*sqlite3_column_bytes)(sqlite3_stmt*, int);
static const char *(*sqlite3_errmsg)(sqlite3*);
static int (*sqlite3_reset)(sqlite3_stmt *);
static int (*sqlite3_clear_bindings)(sqlite3_stmt*);
static int (*sqlite3_close)(sqlite3 *);
static int (*sqlite3_open)(char const *,sqlite3 **);
static int (*sqlite3_prepare_v2)(sqlite3 *,char const *,int,sqlite3_stmt **,char const **);
static int (*sqlite3_libversion_number)();

} // extern "C"

namespace cppcms {
namespace sessions {
namespace sqlite_session {
namespace details {
	#ifdef CPPCMS_WIN_NATIVE
	#define RTLD_LAZY 0
	typedef HMODULE handle_type;
	handle_type dlopen(char const *x,int) { return LoadLibraryW(booster::nowide::convert(x).c_str()); }
	void dlclose(handle_type x)  { FreeLibrary(x); }
	void *dlsym(handle_type x,char const *y) { return (void*)(GetProcAddress(x,y)); }
	#else
	typedef void *handle_type;
	#endif

	class sqlite_loader {
	public:


		sqlite_loader() : loaded(false),handle(0)
		{
		}
		~sqlite_loader()
		{
			if(handle) {
				dlclose(handle);
			}
		}

		void load(char const *name)
		{
			if(loaded)
				return;
			try {
				open(name);
				load_sqlite_so();
			}
			catch(...) {
				if(handle) {
					dlclose(handle);
					handle = 0;
				}
				throw;
			}
			loaded = true;
		}
		bool loaded;
	private:

		template<typename T>
		void load_symbol(T &sym,char const *name)
		{
			void *p=dlsym(handle,name);
			if(!p) {
				throw std::runtime_error(std::string("Failed to resolve ") + name);
			}
			sym = (T)(p);
		}
		void open(char const *name)
		{
			handle=dlopen(name,RTLD_LAZY);
			if(!handle) {
				throw std::runtime_error(std::string("Failed to load library:") + name);
			}
		}
		void load_sqlite_so()
		{
			#define LOAD(x) load_symbol(x,#x)
			LOAD(sqlite3_finalize);
			LOAD(sqlite3_bind_text);
			LOAD(sqlite3_step);
			LOAD(sqlite3_bind_int64);
			LOAD(sqlite3_bind_int);
			LOAD(sqlite3_bind_blob);
			LOAD(sqlite3_column_int64);
			LOAD(sqlite3_column_blob);
			LOAD(sqlite3_column_bytes);
			LOAD(sqlite3_errmsg);
			LOAD(sqlite3_reset);
			LOAD(sqlite3_clear_bindings);
			LOAD(sqlite3_close);
			LOAD(sqlite3_open);
			LOAD(sqlite3_prepare_v2);
			LOAD(sqlite3_libversion_number);
		}

		handle_type handle;

	} sqlite_loader_instance;

	booster::mutex instance_lock;
	
	void load_sqlite(char const *name)
	{
		if(sqlite_loader_instance.loaded)
			return;
		booster::unique_lock<booster::mutex> g(instance_lock);
		sqlite_loader_instance.load(name);
	}
} // details
} // sqlite_session 
} // sessions
} // cppcms

#endif

namespace cppcms {
namespace sessions {
namespace sqlite_session {


class sql_object {
public:
	struct st_guard {
		st_guard(sqlite3_stmt *st) : st_(st) {}
		~st_guard()
		{
			sqlite3_clear_bindings(st_);
			sqlite3_reset(st_);
		}
	private:
		sqlite3_stmt *st_;
	};
	
	
	sql_object(std::string const &db_name) :
		conn_(0),
		replace_(0),
		remove_(0),
		cleanup_(0),
		load_(0)	
	{
		if(sqlite3_open(db_name.c_str(),&conn_)!=SQLITE_OK) {
			if(conn_==0)
				throw std::bad_alloc();
			std::string msg = sqlite3_errmsg(conn_);
			sqlite3_close(conn_);
			throw booster::runtime_error(msg);
		}
	}
	void begin()
	{
		exec("BEGIN");
	}
	void commit()
	{
		exec("COMMIT");
	}
	void prepare(sqlite3_stmt *&st,char const *stmt)
	{
		if(st)
			return;
		int r;
		
		while((r=sqlite3_prepare_v2(conn_,stmt,-1,&st,0))==SQLITE_LOCKED || r==SQLITE_BUSY)
			;
		if(r!=SQLITE_OK)
			throw_error();
	}
	void replace(std::string const &sid,time_t timeout,std::string const &data)
	{
		prepare(replace_,"REPLACE INTO sessions values(?,?,?)");
		st_guard g(replace_);
		check(sqlite3_bind_text(replace_,1,sid.c_str(),sid.size(),SQLITE_STATIC)==SQLITE_OK);
		check(sqlite3_bind_int64(replace_,2,timeout)==SQLITE_OK);
		check(sqlite3_bind_blob(replace_,3,data.c_str(),data.size(),SQLITE_STATIC)==SQLITE_OK);
		check(sqlite3_step(replace_)==SQLITE_DONE);
	}
	void remove(std::string const &sid)
	{
		prepare(remove_,"DELETE FROM sessions WHERE sid=?");
		st_guard g(remove_);
		check(sqlite3_bind_text(remove_,1,sid.c_str(),sid.size(),SQLITE_STATIC)==SQLITE_OK);
		check(sqlite3_step(remove_)==SQLITE_DONE);
	}
	void cleanup(time_t time,int limit)
	{
		prepare(cleanup_,
			"DELETE FROM sessions "
			"WHERE sid in "
			"("
			"  SELECT sid FROM sessions "
			"  WHERE timeout < ? LIMIT ? "
			")" 
		);
		st_guard g(cleanup_);
		check(sqlite3_bind_int64(cleanup_,1,time)==SQLITE_OK);
		check(sqlite3_bind_int(cleanup_,2,limit)==SQLITE_OK);
		check(sqlite3_step(cleanup_)==SQLITE_DONE);
	}
	bool load(std::string const &sid,time_t &to,std::string &value)
	{
		prepare(load_,"SELECT timeout,data FROM sessions WHERE sid=?");
		st_guard g(load_);
		check(sqlite3_bind_text(load_,1,sid.c_str(),sid.size(),SQLITE_STATIC)==SQLITE_OK);
		for(;;) {
			int r = sqlite3_step(load_);
			if(r == SQLITE_DONE) 
				return false;
			if(r == SQLITE_BUSY || r==SQLITE_LOCKED ) {
				sqlite3_reset(load_);
				continue;
			}
			if(r != SQLITE_ROW)
				throw_error();
			break;
		}
		time_t got_to = sqlite3_column_int64(load_,0);
		if(got_to < time(0))
			return false;
		to = got_to;
		char const *data = static_cast<char const *>(sqlite3_column_blob(load_,1));
		size_t size = sqlite3_column_bytes(load_,1);
		value.assign(data,size);
		return true;
	}
	void exec(char const *stmt)
	{
		sqlite3_stmt *st = 0;
		prepare(st,stmt);
		int r = sqlite3_step(st);
		sqlite3_finalize(st);
		if(r!=SQLITE_DONE && r!=SQLITE_ROW) {
			throw_error();
		}
	}
	~sql_object()
	{
		finalize(replace_);
		finalize(remove_);
		finalize(cleanup_);
		finalize(load_);
		sqlite3_close(conn_);
	}

private:
	void check(bool c)
	{
		if(!c)
			throw_error();
	}
	void finalize(sqlite3_stmt *&st)
	{
		if(st) {
			sqlite3_finalize(st);
			st = 0;
		}
	}
	void throw_error()
	{
		throw booster::runtime_error(sqlite3_errmsg(conn_));
	}

	sqlite3 *conn_;
	sqlite3_stmt *replace_;
	sqlite3_stmt *remove_;
	sqlite3_stmt *cleanup_;
	sqlite3_stmt *load_;
};

class sql_session_storage : public cppcms::sessions::session_storage
{
	struct data {
		time_t timeout;
		std::string value;
		data() : timeout(0) {}
		data(time_t t,std::string const &v) : timeout(t), value(v) {}
	};

	typedef std::map<std::string,data> data_type;
	typedef booster::unique_lock<booster::shared_mutex> unique_guard;
	typedef booster::shared_lock<booster::shared_mutex> shared_guard;

	data_type data_;
	booster::shared_mutex lock_;
	
	data_type data_in_write_;
	booster::shared_mutex data_in_write_lock_;
	
	booster::thread_specific_ptr<sql_object> sql_;
	std::string conn_;

public:
	sql_session_storage(std::string const &conn) : conn_(conn)
	{
	}

	virtual void save(std::string const &sid,time_t timeout,std::string const &in)
	{
		unique_guard g(lock_);
		data_[sid]=data(timeout,in);
	}

	///
	/// Load session with \a sid, put its end of life time to \a timeout and return its
	/// value to \a out
	///
	virtual bool load(std::string const &sid,time_t &timeout,std::string &out)
	{
		shared_guard g(lock_);
		data_type::iterator p=data_.find(sid);
		
		if(p!=data_.end()) {
			if(p->second.timeout < time(0))
				return false;
			timeout = p->second.timeout;
			out = p->second.value;
			return true;
		}

		{
			shared_guard g2(data_in_write_lock_);

			p=data_in_write_.find(sid);
			if(p!=data_in_write_.end()) {
				if(p->second.timeout < time(0))
					return false;
				timeout = p->second.timeout;
				out = p->second.value;
				return true;
			}
		}

		if(sql_.get()==0) {
			sql_.reset(new sql_object(conn_));
		}
		return sql_->load(sid,timeout,out);
	}
	
	
	virtual void remove(std::string const &sid) 
	{
		unique_guard g(lock_);
		data_[sid]=data();
	}

	virtual bool is_blocking() 
	{
		return true;
	}

	void gc()
	{
		{
			unique_guard g(lock_);
			{
				unique_guard g2(data_in_write_lock_);
				data_in_write_.swap(data_);
			}
		}
	
		{	
			shared_guard g2(data_in_write_lock_);
			sql_object sql(conn_);
			sql.begin();
			try {
				int count = 0;
				for(data_type::iterator p=data_in_write_.begin();p!=data_in_write_.end();++p) {
					if(p->second.timeout == 0)
						sql.remove(p->first);
					else 
						sql.replace(p->first,p->second.timeout,p->second.value);
					count ++;
				}
				sql.cleanup(time(0),((count * 5) + 1000));
				sql.commit();
			}
			catch(...) {
				try { sql.exec("ROLLBACK"); }catch(...){}
				throw;
			}
		}

		{
			unique_guard g2(data_in_write_lock_);
			data_in_write_.clear();
		}
	}
private:
};



///
/// \brief The factory is an interface to a factory that creates session_storage objects, it should be thread safe.
///
class factory_object : public cppcms::sessions::session_storage_factory {
public:
	factory_object(std::string const &file,std::string const &so)
	{
		details::load_sqlite(so.c_str());
		{
			sql_object sql(file);
			if(sqlite3_libversion_number() < (3*1000000 + 7*1000)) {
				throw booster::runtime_error("Sqlite 3.7 and above required");
			}
			// Very Important
			sql.exec("PRAGMA journal_mode = WAL"); 
			sql.exec("CREATE TABLE IF NOT EXISTS "
				 "sessions ( "
				 "  sid varchar(32) primary key not null,"
				 "  timeout integer not null, "
				 "  data blob not null"
				 ")");
			sql.exec("CREATE INDEX IF NOT EXISTS "
				 "sessions_timeout on sessions(timeout) ");
		}
		storage_.reset(new sql_session_storage(file));

	}
	///
	/// Get a pointer to session_storage. Note if the returned pointer is same for different calls
	/// session_storage implementation should be thread safe.
	///
	virtual booster::shared_ptr<cppcms::sessions::session_storage> get() 
	{
		return storage_;
	}

	///
	/// Return true if session_storage requires garbage collection - removal of expired session time-to-time
	///
	virtual bool requires_gc() 
	{
		return true;
	}
	virtual void gc_job()
	{
		storage_->gc();
	}
	///
	/// Delete the object, cleanup
	///
	virtual ~factory_object() 
	{
		try {
			storage_->gc();
		}
		catch(...) {}
	}
private:
	booster::shared_ptr<sql_session_storage> storage_;
};



CPPCMS_API session_storage_factory *factory(std::string const &database,std::string const &shared_object)
{
	return new cppcms::sessions::sqlite_session::factory_object(database,shared_object);
}

CPPCMS_API session_storage_factory *factory(std::string const &database)
{
	#ifndef CPPCMS_LIBRARY_PREFIX
	#define CPPCMS_LIBRARY_PREFIX ""
	#endif
	return factory(database,CPPCMS_LIBRARY_PREFIX "sqlite3" CPPCMS_LIBRARY_SUFFIX);
}

}// sqlite_session
}// sessions
}// cppcms


