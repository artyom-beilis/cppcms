#ifndef CPPCMS_DBIXX_STORAGE_H
#define CPPCMS_DBIXX_STORAGE_H

#include "session_server_storage_with_cache.h"
#include <dbixx/dbixx.h>

namespace cppcms {
class session_dbixx_storage : public session_server_storage_with_cache {
	dbixx::session &sql;
public:
	session_dbixx_storage(dbixx::session &sql_,cache_iface &cache_) :
		session_server_storage_with_cache(cache_),
		sql(sql_)
	{
	}
protected:	
	virtual void impl_save(std::string const &sid,entry &e)
	{
		dbixx::transaction tr(sql);
		impl_remove(sid);
		std::tm t;
		localtime_r(&e.timeout,&t);
		sql<<"INSERT INTO cppcms_sessions(sid,timeout,data) "
		     "VALUES (?,?,?)",sid,t,e.data;
		sql.exec();
		tr.commit();
		time_t now;
		time(&now);
		localtime_r(&now,&t);
		sql<<"DELETE FROM cppcms_sessions WHERE timeout < ?",t;
		sql.exec();
	}

	virtual bool impl_load(std::string const &sid,entry &e)
	{
		sql<<"SELECT timeout,data FROM cppcms_sessions WHERE sid=?",sid;
		dbixx::row r;
		if(sql.single(r)) {
			std::tm t;
			r>>t>>e.data;
			time_t now;
			time(&now);
			e.timeout=mktime(&t);
			if(e.timeout < now) 
				return false;
			return true;
		}
		return false;
	}
	virtual void remove(std::string const &sid)
	{
		sql<<"DELETE FROM cppcms_sessions WHERE sid=?",sid;
		sql.exec();
	}
};


} // cppcms

#endif
