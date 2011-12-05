#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <db.h>

#include <booster/posix_time.h>
#include <booster/thread.h>
#include <cppcms/session_storage.h>
#include <cppcms/json.h>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>



namespace { // anon

extern "C" {
	static int get_key(DB *, const DBT *, const DBT *pdata, DBT *skey)
	{
		skey->data = pdata->data;
		skey->size = 8;
		return 0;
	}
}

class single_storage 
{
public:
	void throw_error(int r)
	{
		throw std::runtime_error("Berkeley DB CppCMS storage: " + std::string(db_strerror(r)));
	}
	void check(int r,char const *op)
	{
		if(r!=0) {
			throw std::runtime_error(std::string("Berkeley DB CppCMS storage:") + op + db_strerror(r));
		}
	}
	single_storage(std::string const &directory) : env(0),dbp(0),dbp_to(0)
	{
		try {
			check(db_env_create(&env,0),"db_env_create");
			check(env->open(env,directory.c_str(),DB_INIT_CDB | DB_INIT_MPOOL |  DB_CREATE,0666),"db_env::open");
			check(db_create(&dbp, env, 0),"db_create");
			check(dbp->open(dbp, NULL, "sid.db", NULL, DB_HASH, DB_CREATE, 0666),"db::open");
			check(db_create(&dbp_to, env, 0),"db_create");
			check(dbp_to->set_flags(dbp_to, DB_DUP | DB_DUPSORT),"db::set_flags");
			check(dbp_to->open(dbp_to,NULL,"timeouts.db",NULL,DB_BTREE,DB_CREATE,0666),"db::open");
			check(dbp->associate(dbp, NULL, dbp_to, get_key, 0),"db::associate");
		}
		catch(...) {
			if(dbp_to)
				dbp_to->close(dbp_to,0);
			if(dbp)
				dbp->close(dbp,0);
			if(env)
				env->close(env,0);
		}
	}

	void save(std::string const &sid,time_t timeout,std::string const &in) 
	{
		DBT key, data;
		memset(&key,0,sizeof(key));
		memset(&data,0,sizeof(data));
		key.data = const_cast<char *>(sid.c_str());
		key.size = sid.size();
		std::vector<char> d(8 + in.size());
		int64_t be_time = to_big_endian(timeout);
		memcpy(&d[0],&be_time,8);
		memcpy(&d[0]+8,in.c_str(),in.size());
		data.data = &d[0];
		data.size = d.size();
		int ret = dbp->put(dbp,NULL,&key,&data,0);
		check(ret,"db::put");
	}

	bool load(std::string const &sid,time_t &timeout,std::string &out)
	{
		DBT key, data;
		memset(&key,0,sizeof(key));
		memset(&data,0,sizeof(data));
		key.data = const_cast<char *>(sid.c_str());
		key.size = sid.size();
		data.flags = DB_DBT_MALLOC;
		int ret = dbp->get(dbp,NULL,&key,&data,0);
		if(ret == DB_NOTFOUND)
			return false;
		check(ret,"db::get");
		int64_t be_time;
		memcpy(&be_time,data.data,8);
		time_t to = to_big_endian(be_time);
		if(to < time(0)) {
			free(data.data);
			return false;
		}
		timeout = to;
		out.assign((char *)(data.data)+8,data.size - 8);
		free(data.data);
		return true;
	}
	
	void remove(std::string const &sid)
	{
		DBT key;
		memset(&key,0,sizeof(key));
		key.data = const_cast<char *>(sid.c_str());
		key.size = sid.size();
		int r = dbp->del(dbp,NULL,&key,0);	
		if(r== DB_NOTFOUND)
			return;
		check(r,"db::del");
	}

	void gc()
	{
		DBC *cur = 0;
		check(dbp_to->cursor(dbp_to,0,&cur,DB_WRITECURSOR),"db::cursor");
		
		for(;;) {
			DBT data;
			memset(&data,0,sizeof(data));
			data.flags = DB_DBT_MALLOC;
		
			#if DB_VERSION_MAJOR * 100 + DB_VERSION_MINOR >= 406 
			int ret = cur->get(cur,0,&data,DB_FIRST);
			#else
			int ret = cur->c_get(cur,0,&data,DB_FIRST);
			#endif
			if(ret == DB_NOTFOUND)
				break;
			if(ret!=0) {
				#if DB_VERSION_MAJOR * 100 + DB_VERSION_MINOR >= 406 
				cur->close(cur);
				#else
				cur->c_close(cur);
				#endif
				check(ret,"dbc::get");
			}
			
			int64_t be_time;
			memcpy(&be_time,data.data,8);
			free(data.data);
			time_t to = to_big_endian(be_time);
			if(to < time(0))
				break;
		}

		#if DB_VERSION_MAJOR * 100 + DB_VERSION_MINOR >= 406 
		cur->close(cur);
		#else
		cur->c_close(cur);
		#endif
		cur = 0;
		
		check(dbp->sync(dbp,0),"db::sync");
		check(dbp_to->sync(dbp_to,0),"db::sync");
	}

	~single_storage()
	{
		dbp->close(dbp,0);
		dbp_to->close(dbp_to,0);
		env->close(env,0);
	}
private:
	static int64_t to_big_endian(int64_t val_in)
	{
		uint64_t val = val_in;
		union { char c[8]; int64_t v; } u;
		for(unsigned i=0;i<8;i++) {
			u.c[i]=val >> 56;
			val <<=8;
		}
		return u.v;
	}		
	DB_ENV *env;
	DB *dbp;
	DB *dbp_to;
};

class bdb_storage : public cppcms::sessions::session_storage {
public:
	void save(std::string const &sid,time_t timeout,std::string const &in) 
	{
		ptr()->save(sid,timeout,in);
	}
	bool load(std::string const &sid,time_t &timeout,std::string &out)
	{
		return ptr()->load(sid,timeout,out);
	}
	void remove(std::string const &sid)
	{
		return ptr()->remove(sid);
	}
	bool is_blocking()
	{
		return true;
	}
	void gc()
	{
		ptr()->gc();
	}
	bdb_storage(std::string const &dir) : dir_(dir) 
	{
	}
private:
	single_storage *ptr()
	{
		single_storage *p = ptr_.get();
		if(!p) {
			p=new single_storage(dir_);
			ptr_.reset(p);
		}
		return p;
	}
	std::string dir_;
	booster::thread_specific_ptr<single_storage> ptr_;
};

class bdb_factory : public cppcms::sessions::session_storage_factory {
public:
	virtual booster::shared_ptr<cppcms::sessions::session_storage> get()
	{
		return storage_;
	}

	virtual bool requires_gc() 
	{
		return false;
	}
	virtual void gc_job() 
	{

		storage_->gc();
	}
	bdb_factory(std::string const &dir)
	{
		storage_.reset(new bdb_storage(dir));
	}
private:
	booster::shared_ptr<bdb_storage> storage_;
};

#if defined(CPPCMS_WIN32)
# define STORAGE_API __declspec(dllexport)
#else
# define STORAGE_API
#endif

} // namespace

extern "C" {
	STORAGE_API cppcms::sessions::session_storage_factory *sessions_generator(cppcms::json::value const &v)
	{
		std::string dir = v.get<std::string>("directory");
		return new bdb_factory(dir);
	}
}

