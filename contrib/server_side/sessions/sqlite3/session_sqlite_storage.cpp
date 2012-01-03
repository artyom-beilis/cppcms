///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/config.h>
#include <cppcms/session_storage.h>
#include <booster/thread.h>
#include <booster/backtrace.h>
#include <sstream>
#include <cppcms/json.h>
#include <iostream>
#include <map>
#include <time.h>

#include <sqlite3.h>

namespace {

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
	factory_object(std::string const &file)
	{
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

} // anon

#if defined(CPPCMS_WIN32)
# define STORAGE_API __declspec(dllexport)
#else
# define STORAGE_API
#endif

extern "C" {
STORAGE_API cppcms::sessions::session_storage_factory *sessions_generator(cppcms::json::value const &opt)
{
	return new factory_object(opt.get<std::string>("db"));
}
} // extern "C"

