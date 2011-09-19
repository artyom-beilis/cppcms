#include <cppdb/frontend.h>
#include <cppcms/session_storage.h>
#include <booster/thread.h>
#include <sstream>
#include <map>

namespace {

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
	typedef booster::unique_lock<booster::shared_mutex> shared_guard;

	data_type data_;
	booster::shared_mutex lock_;
	
	data_type data_in_write_;
	booster::shared_mutex data_in_write_lock_;
	
	booster::thread_specific_ptr<cppdb::session> sql_;
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
			sql_.reset(new cppdb::session(conn_ + ";mode=readonly"));
			sql_->prepare("PRAGMA read_uncommited=true").exec();
		}
		cppdb::session &sql = *sql_;
		cppdb::result r;
		r= sql << "SELECT timeout,data FROM sessions "
			   "WHERE sid = ?" << sid << cppdb::row;
		if(r.empty())
			return false;
		time_t t;
		std::stringstream ss;
		r >> t;
		if(t < time(0))
			return false;
		r >> ss;
		out = ss.str();
		timeout = t;
		return true;
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
			cppdb::session sql(conn_);
			cppdb::transaction tr(sql);
			int count = 1;
			for(data_type::iterator p=data_.begin();p!=data_.end();++p) {
				if(p->second.timeout == 0)
					sql << "DELETE from sessions WHERE sid=?" << p->first << cppdb::exec;
				else {
					std::stringstream ss(p->second.value);
					sql << "REPLACE INTO sessions values(?,?,?)" 
					     << p->first << p->second.timeout << ss << cppdb::exec;
				}
				count ++;
			}
			sql << "DELETE FROM sessions WHERE timeout < ? limit ?" << time(0) << (count * 5) << cppdb::exec;
			tr.commit();
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
class factory : public cppcms::sessions::session_storage_factory {
public:
	factory(std::string const &file)
	{
		std::string conn_str = "sqlite3:db='" + file +"'";
		cppdb::session sql(conn_str);
		sql << 	"CREATE TABLE IF NOT EXISTS "
			"sessions ( "
			"  sid varchar(32) primary key not null,"
			"  timeout integer not null, "
			"  data blob not null"
			")"
			<< cppdb::exec;
		sql << 	"CREATE INDEX IF NOT EXISTS "
			"sessions_timeout on sessions(timeout) "
			<< cppdb::exec;
		sql.close();
		storage_.reset(new sql_session_storage(conn_str));

	}
	///
	/// Get a pointer to session_storage. Note if the returned pointer is same for different calls
	/// session_storage implementation should be thread safe.
	///
	virtual booster::shared_ptr<session_storage> get() 
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
	virtual ~factory() 
	{
		try {
			storage_->gc();
		}
		catch(...) {}
	}
private:
	booster::shared_ptr<sql_session_storage> storage_;
};




} // anonymous


extern "C" {

cppcms::sessions::session_storage_factory *cppdb_session_storage(std::string const &parameter)
{
	return new factory(parameter);
}

} // extern "C"


