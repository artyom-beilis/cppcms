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
#include "session_sid.h"
#include "global_config.h"
#include "session_sqlite_storage.h"
#include "cppcms_error.h"
#include "posix_mutex.h"
#include <sqlite3.h>
#include <pthread.h>
#include <poll.h>
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/lexical_cast.hpp>
#else // Internal Boost
#   include <cppcms_boost/lexical_cast.hpp>
    namespace boost = cppcms_boost;
#endif

namespace cppcms {
namespace storage {

class sqlite {
	sqlite3 *db;
	pthread_mutex_t mutexes;
	int write_ops;
	time_t last_commit;
	int deferred_commit_count;
	int deferred_commit_time;
	pthread_mutex_t mutex;

public:
	void exec(char const *s)
	{
		char *err=NULL;
		int res;
		while((res=sqlite3_exec(db,s,NULL,NULL,&err))!=0) {
			if(res==SQLITE_BUSY) {
				sqlite3_free(err);
				poll(NULL,0,5);
				continue;
			}
			string err_msg=err;
			sqlite3_free(err);
			throw cppcms_error(err_msg);
		}
	}
	sqlite (string db_name,
		bool sync_,
		int deferred_commit_count_,
		int deferred_commit_time_) :
		db(0),
			deferred_commit_count(deferred_commit_count_),
			deferred_commit_time (deferred_commit_time_)
	{
		pthread_mutex_init(&mutex,NULL);
		try {
			if(sqlite3_open(db_name.c_str(),&db)) {
				string error=sqlite3_errmsg(db);
				throw cppcms_error(error);
			}
			exec(	"CREATE TABLE IF NOT EXISTS sessions ( "
				" sid varchar(32) primary key not null, "
				" data varchar(1)  not null, "
				" timeout integer not null "
				")");
			exec(	"CREATE INDEX IF NOT EXISTS sessions_timeout "
				" ON sessions(timeout)");
			if(!sync_) {
				exec(	"PRAGMA synchronous=OFF");
			}
			write_ops=0;
			last_commit=time(NULL);
			exec("begin");
		}
		catch(...) {
			if(db) sqlite3_close(db);
			pthread_mutex_destroy(&mutex);
			throw;
		}
	}
	~sqlite()
	{
		try {
			exec("commit");
		}
		catch(...) {
		}
		if(db) sqlite3_close(db);
		if(deferred_commit_count > 0) 
			pthread_mutex_destroy(&mutex);
	}
	void check_commits()
	{
		long long int now=time(NULL);
		if(	write_ops >= deferred_commit_count 
			|| (deferred_commit_time > 0 && last_commit + deferred_commit_time < now))
		{
			char stat[128];
			snprintf(stat,sizeof(stat),"DELETE FROM sessions WHERE timeout < %lld",now);
			exec(stat);
			exec("commit");
			exec("begin");
			last_commit=time(NULL);
			write_ops=0;
		}
	}
	void exec(char const *s,string const &sid,string const &data,int64_t timeout)
	{
		sqlite3_stmt *st;
		if(sqlite3_prepare(db,s,-1,&st,NULL)!=0) {
			throw cppcms_error(string("sqlite prepared statement:")+sqlite3_errmsg(db));
		}
		sqlite3_bind_text(st,1, sid.c_str(), sid.size(),SQLITE_STATIC);
		sqlite3_bind_blob(st,2,data.c_str(),data.size(),SQLITE_STATIC);
		sqlite3_bind_int64(st,3,timeout);
		int res;
		while((res=sqlite3_step(st))==SQLITE_BUSY){
			poll(NULL,0,5);
		}
		if(res==SQLITE_DONE) {
			sqlite3_finalize(st);
		}
		else {
			throw cppcms_error(string("Insert error:")+sqlite3_errmsg(db));
		}
	}
	bool select(string const &sid,time_t &timeout,string &data)
	{
		sqlite3_stmt *st;
		char const *q="SELECT data,timeout FROM sessions WHERE sid=?";
		if(sqlite3_prepare(db,q,-1,&st,NULL)!=0) {
			throw cppcms_error(string("sqlite prepared statement:")+sqlite3_errmsg(db));
		}
		int res;
		sqlite3_bind_text(st,1, sid.c_str(), sid.size(),SQLITE_STATIC);
		while((res=sqlite3_step(st))==SQLITE_BUSY){
			poll(NULL,0,5);
		}
		if(res==SQLITE_DONE) {
			sqlite3_finalize(st);
			return false;
		}
		else if(res==SQLITE_ROW) {
			int64_t to=sqlite3_column_int64(st,1);
			if(to < time(NULL)) {
				sqlite3_finalize(st);
				del(sid);
				write_ops++;
				return false;
			}
			size_t length=sqlite3_column_bytes(st,0);
			data.assign((const char *)sqlite3_column_blob(st,0),length);
			timeout=to;
			sqlite3_finalize(st);
			return true;
		}
		else {
			throw cppcms_error(string("Insert error:")+sqlite3_errmsg(db));
		}
	
	}
	void save(string const &sid,time_t timeout,string const &data)
	{
		mutex_lock lock(mutex);
		exec("INSERT OR REPLACE INTO sessions VALUES(?,?,?)",sid,data,timeout);
		write_ops++;
		check_commits();

	}
	void del(string const &sid)
	{
		char q[128];
		snprintf(q,sizeof(q),"DELETE FROM sessions WHERE sid='%s'",sid.c_str());
		exec(q);
	}
	void remove(string const &sid)
	{
		mutex_lock lock(mutex);
		del(sid);
		write_ops++;
		check_commits();	
	}
	bool load(std::string const &sid,time_t *timeout,std::string &out)
	{
		mutex_lock lock(mutex);
		time_t tout;
		if(!select(sid,tout,out)) {
			check_commits();
			return false;
		}
		if(timeout) *timeout=tout;
		check_commits();
		return true;
	}
};


sqlite &sqlite_N::db(string const &sid) 
{
	char buf[3]={ sid.at(10) , sid.at(15) , 0 };
	int v;
	sscanf(buf,"%x",&v);
	v = v%size;
	return *dbs.at(v);
}
sqlite_N::sqlite_N(string db,int n,bool sync,int def_comm,int def_to) :
	size(n)
{
	dbs.resize(n);
	int i;
	for(i=0;i<n;i++) {
		string fname=db+"_"+boost::lexical_cast<string>(i);
		dbs[i]=boost::shared_ptr<sqlite>(new sqlite(fname,sync,def_comm,def_to));
	}
}

bool sqlite_N::load(std::string const &sid,time_t *timeout,std::string &out)
{
	return db(sid).load(sid,timeout,out);
}
void sqlite_N::remove(string const &sid)
{	
	db(sid).remove(sid);
}
void sqlite_N::save(string const &sid,time_t timeout,string const &data)
{
	db(sid).save(sid,timeout,data);
}
	

} // storage

#ifndef NO_BUILDER_INTERFACE

namespace {

// The database is created at startup
struct builder_thread {
	boost::shared_ptr<storage::sqlite_N> db;
	bool cache;
	builder_thread(string dir,int n,bool sync,int dc,int dt,bool c) :
		db(new storage::sqlite_N(dir,n,sync,dc,dt)),
		cache(c)
	{
	}
	boost::shared_ptr<session_api> operator()(worker_thread &w)
	{	
		boost::shared_ptr<session_server_storage> storage(new session_sqlite_storage(db));
		return boost::shared_ptr<session_api>(new session_sid(storage,cache));
	}
};

// The database created *AFTER* forks + no deferred commits
struct builder_proc {
	string dir;
	bool sync;
	int size;
	bool cache;
	builder_proc(string d,int n,bool s,bool c) : dir(d) , sync(s) , size(n), cache(c)
	{
	}
	boost::shared_ptr<session_api> operator()(worker_thread &w)
	{	
		boost::shared_ptr<storage::sqlite_N> db(new storage::sqlite_N(dir,size,sync,0,0));
		boost::shared_ptr<session_server_storage> storage(new session_sqlite_storage(db));
		return boost::shared_ptr<session_api>(new session_sid(storage,cache));
	}

};

}

session_backend_factory session_sqlite_storage::factory(cppcms_config const  &config)
{
	string db=config.sval("session.sqlite_db");
	int db_count=config.ival("session.sqlite_db_num",4);
	if(db_count>8)
		db_count=8;
	if(db_count<0)
		db_count=0;
	db_count=1<<db_count;
	string def="fork";
	if(config.sval("server.mod","")=="thread")
		def="thread";
	string mod=config.sval("session.sqlite_mod",def);
	bool cache=config.ival("session.server_enable_cache",0);
	if(mod=="fork") {
		bool sync=config.ival("session.sqlite_sync",0);
		return builder_proc(db,db_count,sync,cache);
	}
	else if(mod=="thread") {
		bool sync=config.ival("session.sqlite_sync",1);
		int  dc=config.ival("session.sqlite_commits",1000);
		int  dt=config.ival("session.sqlite_commit_timeout",5);
		return builder_thread(db,db_count,sync,dc,dt,cache);
	}
	else {
		throw cppcms_error("Unknown sqlite mode:"+mod);
	}
}

#else // NO_BUILDER_INTERFACE
session_backend_factory session_sqlite_storage::factory(cppcms_config const  &config)
{
	throw runtime_error("session_sqlite_storage::factory should bot be used");
}
#endif


session_sqlite_storage::session_sqlite_storage(boost::shared_ptr<storage::sqlite_N> db_):
	db(db_)
{
}
void session_sqlite_storage::save(std::string const &sid,time_t timeout,std::string const &in)
{
	db->save(sid,timeout,in);
}
bool session_sqlite_storage::load(std::string const &sid,time_t *timeout,std::string &out)
{
	return db->load(sid,timeout,out);
}
void session_sqlite_storage::remove(std::string const &sid) 
{
	return db->remove(sid);
}

} // cppcms
