#include <cppdb/frontend.h>
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


	void set_session_option(cppdb::session &sql)
	{
		switch(engine_) {
		case sqlite3:
			switch(transaction_mode_) {
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
			switch(transaction_mode_) {
			case relaxed:
			case non_durable:
				sql << "SET SESSION synchronous_commit OFF" << cppdb::exec;
				break;
			default:
				;
			}
		default:
			;
		}
	}

	void save(std::string const &sid,time_t timeout,std::string const &in)
	{
		cppdb::session sql(conn_str_);
		set_session_option(sql);
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
		cppdb::session sql(conn_str_);
		set_session_option(sql);
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
		cppdb::session sql(conn_str_);
		set_session_option(sql);
		sql << "DELETE FROM cppdb_sessions WHERE sid = ?" 
			<< sid <<cppdb::exec;
	}

	void gc()
	{
		cppdb::session sql(conn_str_);
		set_session_option(sql);
		sql <<	"DELETE FROM sessions "
			"WHERE sid in "
			"("
			"  SELECT sid FROM sessions "
			"  WHERE timeout < ?"
			")" << cppdb::exec;
	}

	cppdb_storage(cppcms::json::value const &val) 
	{
		conn_str_ = val.get<std::string>("connection_string");
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

		cppdb::session sql(conn_str_);
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
				sql <<	"CREATE TABLE IF NOT EXISTS sessions ("
					" id varchar(32) primary key not null, "
					" timeout bigint not null, "
					" content blob non null "
					")" << cppdb::exec;
				sql <<	"CREATE INDEX IF NOT EXISTS "
					"sessions_timeout on sessions(timeout)" << cppdb::exec;
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
					sql <<	"CREATE TABLE IF NOT EXISTS sessions ("
						" id varchar(32) primary key not null, "
						" timeout bigint not null, "
						" content blob non null "
						") Engine=InnoDB" << cppdb::exec;
					break;
				case relaxed:
				case non_durable:
					sql <<	"CREATE TABLE IF NOT EXISTS sessions ("
						" id varchar(32) primary key not null, "
						" timeout bigint not null, "
						" content blob non null "
						") Engine=MyISAM" << cppdb::exec;
					break;
				}
				sql <<	"CREATE INDEX IF NOT EXISTS "
					"sessions_timeout on sessions(timeout)" << cppdb::exec;
			}
		case postgresql:
			{
				cppdb::transaction tr(sql);
				cppdb::result r = sql 
					<<  "SELECT 1 FROM pg_tables "
					    "WHERE tablename = 'sessions' " 
					<<cppdb::row;
				if(r.empty()) {
					sql <<	"CREATE TABLE sessions ("
						" id varchar(32) primary key not null, "
						" timeout bigint not null, "
						" content bytea non null "
						")" << cppdb::exec;
					sql <<	"CREATE INDEX IF NOT EXISTS "
						"sessions_timeout on sessions(timeout)" << cppdb::exec;
				}
				tr.commit();
			}
			break;
		}
	}
	bool is_blocking() 
	{
		return true;
	}
private:

	std::string conn_str_;
	transactivity transaction_mode_;
	engine_type engine_;
};


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
# define STORAGE_API declspec(__dllexport)
#else
# define STORAGE_API
#endif

extern "C" {
	STORAGE_API cppcms::sessions::session_storage_factory *sessions_generator(cppcms::json::value const &options)
	{
		return new cppdb_factory(options);
	}
}


