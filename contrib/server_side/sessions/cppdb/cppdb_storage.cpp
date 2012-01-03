///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppdb/frontend.h>
#include <cppdb/backend.h>
#include <cppdb/pool.h>
#include <cppcms/session_storage.h>
#include <cppcms/json.h>
#include <sstream>
#include <time.h>

namespace { // anon

class cppdb_storage : public cppcms::sessions::session_storage  {
public:

	enum engine_type {
		mysql,
		sqlite3,
		postgresql
	};

	enum transactivity {
		acid,
		relaxed,
		non_durable
	};

	struct options_setter;
	friend struct options_setter;

	struct options_setter {
		options_setter(cppdb_storage *p);
		transactivity transaction_mode;
		engine_type engine;
		void operator()(cppdb::session &sql) const
		{
			switch(engine) {
			case sqlite3:
				switch(transaction_mode) {
				case relaxed:
					sql << "PRAGMA synchronous = NORMAL" << cppdb::exec;
					break;
				case non_durable:
					sql << "PRAGMA synchronous = OFF" << cppdb::exec;
					break;
				default:
					;
				}
				break;
			case postgresql:
				switch(transaction_mode) {
				case relaxed:
				case non_durable:
					sql << "SET SESSION synchronous_commit = OFF" << cppdb::exec;
					break;
				default:
					;
				}
			default:
				;
			}
		}

	};


	void save(std::string const &sid,time_t timeout,std::string const &in)
	{
		cppdb::session sql(pool_->open(),options_setter(this));
		std::istringstream ss(in);
		std::istream &si = ss;
		switch(engine_) {
		case mysql:
		case sqlite3:
			sql << 	"REPLACE INTO cppdb_sessions "
				"VALUES(?,?,?) " 
				<< sid 
				<< timeout 
				<< si << cppdb::exec;
			break;
		case postgresql:
			{
				cppdb::transaction tr(sql);
				sql <<	"DELETE FROM cppdb_sessions WHERE sid=?" << sid << cppdb::exec;
				sql << 	"INSERT INTO cppdb_sessions "
					"VALUES(?,?,?) " << sid << timeout << si << cppdb::exec;
				tr.commit();
			}
		}
	}

	bool load(std::string const &sid,time_t &timeout,std::string &out)
	{
		cppdb::session sql(pool_->open(),options_setter(this));
		cppdb::result r;
		std::ostringstream ss;
		std::ostream &os=ss;
		r=sql<<	"SELECT timeout,content FROM cppdb_sessions "
			"WHERE sid = ?" << sid << cppdb::row;
		if(r.empty())
			return false;
		time_t to;
		r >> to;
		if(to < time(0)) {
			return false;
		}
		r >> os;
		r.clear();
		out = ss.str();
		timeout = to;
		return true;
	}
		
	virtual void remove(std::string const &sid) 
	{
		cppdb::session sql(pool_->open(),options_setter(this));
		sql << "DELETE FROM cppdb_sessions WHERE sid = ?" 
			<< sid <<cppdb::exec;
	}

	void gc()
	{
		cppdb::session sql(pool_->open(),options_setter(this));
		sql <<	"DELETE FROM cppdb_sessions WHERE timeout < ?"
			<< time(0) << cppdb::exec;
	}

	cppdb_storage(cppcms::json::value const &val) 
	{
		std::string conn_str = val.get<std::string>("connection_string");
		std::string mode = val.get("transactivity","acid");

		if(mode == "acid")
			transaction_mode_ = acid;
		else if(mode == "relaxed")
			transaction_mode_ = relaxed;
		else if(mode == "non_durable")
			transaction_mode_ = non_durable;
		else 
			throw std::runtime_error("session-cppdb-storage: unsupported transactivity=`"+mode + "' "
						"valid values are acid, relaxed and non_durable");
		std::string engine;

		cppdb::session sql(conn_str);
		std::string driver = sql.engine();
		if(driver == "mysql")
			engine_ = mysql;
		else if(driver == "sqlite3")
			engine_ = sqlite3;
		else if(driver == "postgresql")
			engine_ = postgresql;
		else
			throw std::runtime_error("session-cppdb-storage: unsupported driver:" + driver);

		switch(engine_) {
		case sqlite3:
			{
				sql <<	"CREATE TABLE IF NOT EXISTS cppdb_sessions ("
					" sid varchar(32) primary key not null, "
					" timeout bigint not null, "
					" content blob not null "
					")" << cppdb::exec;
				sql <<	"CREATE INDEX IF NOT EXISTS "
					"sessions_timeout on cppdb_sessions(timeout)" << cppdb::exec;
				std::string ver;
				sql <<	"SELECT sqlite_version()" << cppdb::row >> ver;
				size_t pos = ver.find('.');
				if(pos!=std::string::npos)
					pos = ver.find('.',pos+1);
				if(pos!=std::string::npos)
					ver.resize(pos);
				if(atof(ver.c_str()) > 3.699999) {
					sql << "PRAGMA journal_mode = WAL" << cppdb::row;
				}
				switch(transaction_mode_) {
				case relaxed:
					sql << "PRAGMA synchronous=NORMAL" << cppdb::exec;
					break;
				case non_durable:
					sql << "PRAGMA synchronous=OFF" << cppdb::exec;
					break;
				default:
					;
				}
			}
			break;
		case mysql:
			{
				switch(transaction_mode_) {
				case acid:
					sql <<	"CREATE TABLE IF NOT EXISTS cppdb_sessions ("
						" sid varchar(32) primary key not null, "
						" timeout bigint not null, "
						" content blob not null, "
						" index sessions_timeout (timeout) "
						") Engine=InnoDB" << cppdb::exec;
					break;
				case relaxed:
				case non_durable:
					sql <<	"CREATE TABLE IF NOT EXISTS cppdb_sessions ("
						" sid varchar(32) primary key not null, "
						" timeout bigint not null, "
						" content blob not null, "
						" index sessions_timeout (timeout) "
						") Engine=MyISAM" << cppdb::exec;
					break;
				}
			}
			break;
		case postgresql:
			{
				cppdb::transaction tr(sql);
				cppdb::result r = sql 
					<<  "SELECT 1 FROM pg_tables "
					    "WHERE tablename = 'cppdb_sessions' " 
					<<cppdb::row;
				if(r.empty()) {
					sql <<	"CREATE TABLE cppdb_sessions ("
						" sid varchar(32) primary key not null, "
						" timeout bigint not null, "
						" content bytea not null "
						")" << cppdb::exec;
					sql <<	"CREATE INDEX "
						"sessions_timeout on cppdb_sessions(timeout)" << cppdb::exec;
				}
				tr.commit();
			}
			break;
		}

		pool_ = cppdb::pool::create(conn_str);
	}
	bool is_blocking() 
	{
		return true;
	}
private:

	cppdb::pool::pointer pool_;
	transactivity transaction_mode_;
	engine_type engine_;
};

cppdb_storage::options_setter::options_setter(cppdb_storage *p) :
	transaction_mode(p->transaction_mode_),
	engine(p->engine_)
{
}


class cppdb_factory : public cppcms::sessions::session_storage_factory {
public:
	cppdb_factory(cppcms::json::value const &v)
	{
		storage_.reset(new cppdb_storage(v));
	}
	virtual booster::shared_ptr<cppcms::sessions::session_storage> get() 
	{
		return storage_;
	}

	virtual bool requires_gc() 
	{
		return true;
	}
	virtual void gc_job()
	{
		storage_->gc();
	}
private:
	booster::shared_ptr<cppdb_storage> storage_;
};

} // anon
#if defined(CPPCMS_WIN32)
# define STORAGE_API __declspec(dllexport)
#else
# define STORAGE_API
#endif

extern "C" {
	STORAGE_API cppcms::sessions::session_storage_factory *sessions_generator(cppcms::json::value const &options)
	{
		return new cppdb_factory(options);
	}
}


