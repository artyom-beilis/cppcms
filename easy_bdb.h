#ifndef EASY_BDB_H
#define EASY_BDB_H

#include <db_cxx.h>
#include <iostream>
#include <string>

namespace ebdb {

template<int size>
class varchar {
	char data[size+1];
	void reset() { memset(data,0,size+1); };
	void set(char const *s) {
		if(s==data) return;
		reset();
		strncpy(data,s,size);
	};
public:
	varchar() { reset(); };
	varchar(char const *s) { set(s); };
	varchar(std::string const s) { set(s.c_str()); };
	varchar &operator=(varchar &v) { set(v.data); return *this;};
	varchar &operator=(char const *s) { set(s); return *this;};
	operator char const *() { return data; };
	operator std::string () { return std::string(data); };
	bool operator<(char const *s) { return strcmp(data,s)<0; };
	bool operator>(char const *s) { return strcmp(data,s)>0; };
	bool operator==(char const *s) { return strcmp(data,s)==0; };
};
	
	
template<typename T1,typename T2>
struct Ord_Pair{
T1 first;
T2 second;
	Ord_Pair() {};
	Ord_Pair(T1 &t1,T2 &t2) { first=t1; second=t2; };
	bool operator<(Ord_Pair &s)
	{
		if(first<s.first)
			return true;
		if(first>s.first) {
			return false;
		}
		return second<s.second;
	};
	bool operator>(Ord_Pair &s)
	{
		if(first>s.first)
			return true;
		if(first<s.first) {
			return false;
		}
		return second>s.second;
	};
	bool operator==(Ord_Pair &s)
	{
		return first==s.first && second==s.second;
	};
};
	
	
enum { 	SELECT_START,
	SELECT_END,
	SELECT_GT,
	SELECT_GTE,
	SELECT_LT,
	SELECT_LTE,
	SELECT_EQ};

class Environment {
public:
	DbEnv *env;
	char *env_name;
public:
	Environment(char *name) {
		env = new DbEnv(0);
		env_name = new char [strlen(name)+1];
		strcpy(env_name,name);
	};
	
	DbEnv *getenv() {
		return env;
	};
	void close() {
		env->close(0);
	};
	void open() {
		env->open(env_name,DB_INIT_MPOOL | DB_THREAD ,0);
	};
	void create() {
		env->open(env_name,DB_CREATE | DB_INIT_MPOOL | DB_THREAD ,0);
	};
	~Environment() {
		delete [] env_name;
		delete env;
	};
};	

template<class DS,typename E> class Index_Base;

template<class DS,class E>
class cursor {
	DS data;
	void setup_dbt(Dbt &dbt,DS &data)
	{
		dbt.set_data(&data);
		dbt.set_ulen(sizeof(DS));
		dbt.set_flags(DB_DBT_USERMEM);
	};
	
	void setup_dbt(Dbt &dbt,E &data)
	{
		dbt.set_data(&data);
		dbt.set_ulen(sizeof(E));
		dbt.set_flags(DB_DBT_USERMEM);
	};
public:
	Dbc *cur;
	int (*cmp_fnc)(void const *,void const *);
	int direction;

	cursor(){
		cur=NULL;
		cmp_fnc=NULL;
		direction=1;
	};
	
	cursor(Index_Base<DS,E> &db) {
		cur=NULL;
		cmp_fnc=NULL;
		direction=1;
		db.init_cursor(*this);
	};
	
	cursor(Db *db,int (*cmp)(void *,void *)) {
		direction = 1;
		db->cursor(NULL,&cur,0);
		cmp_fnc=cmp;
	};
	
	~cursor() {
		if(cur)
			cur->close();
	};

	bool operator=(DS &data)
	{
		Dbt key;
		Dbt val(&data,sizeof(DS));
		return cur->put(&key,&val,DB_CURRENT)==0;
	};
	
	int select(int dir,DS &data)
	{
		E akey;
		Dbt key,val; 
		int res;
		
		setup_dbt(val,data);
		setup_dbt(key,akey);
		
		switch(dir) {
			case SELECT_START:
				direction=1;
				res=cur->get(&key,&val,DB_FIRST);
				break;
			case SELECT_END:
				direction=-1;
				res=cur->get(&key,&val,DB_LAST);
				break;
			default:
				throw "Incorrect cursor direction";
		}
		return res;
	}
	
	int select(int dir,E const &in_key,DS &data){
		E akey;
		
		memcpy(&akey,&in_key,sizeof(E));
		
		Dbt key,val; 
		int res;
		
		setup_dbt(val,data);
		setup_dbt(key,akey);
		switch(dir) {
			case SELECT_START:
			case SELECT_END:
				return select(direction,data);
			case SELECT_GTE:
			case SELECT_GT:
				direction=1;
				break;
			case SELECT_LTE:
			case SELECT_LT:
				direction=-1;
				break;
			case SELECT_EQ:
				direction=0;
				break;
			default:
				throw "Incorrect cursor direction";
		}
		
		if(dir==SELECT_EQ) {
			return cur->get(&key,&val,DB_SET);
		}
		res=cur->get(&key,&val,DB_SET_RANGE);
		
		if(res){
			if(dir == SELECT_LTE || dir == SELECT_LT) {
				res=cur->get(&key,&val,DB_LAST);
				return res;
			}
			return res;
		}
		
		if(!cmp_fnc) {
			return res;
		}
		
		int cmpres=cmp_fnc(&akey,&in_key);
		
		if(cmpres==0 && dir == SELECT_LT)
		{
			return next(data,false);
		}
		if(cmpres==0 && dir == SELECT_LTE)
		{
			direction=-direction;
			//std::cout<<data.name<<'\n';
			res=next(data,true);
			//std::cout<<data.name<<'\n';
			if(res) {
				return select(SELECT_END,data);
			}
			direction=-direction;
			return next(data,false);
			//return next(data,false);
		}
		if(cmpres==0 && dir == SELECT_GT)
		{
			return next(data,true);
		}
		
		if(cmpres>0 && (dir == SELECT_LT || dir == SELECT_LTE) )
		{
			return next(data,false);
		}
		
		return res;
	};
	
private:	
	int curget(DS &data,int d)
	{
		E akey;
		Dbt key,val; 
		setup_dbt(val,data);
		setup_dbt(key,akey);
		return cur->get(&key,&val,d);
	}
public:	
	int next(DS &data,bool use_no_dup=false) {
		int d;
		
		if(direction==1){
			d=use_no_dup ? DB_NEXT_NODUP : DB_NEXT;
		}
		else if(direction==0) {
			d=DB_CURRENT;
		}
		else /*if(direction==-1)*/ {
			d=use_no_dup ? DB_PREV_NODUP : DB_PREV;
		}
		return curget(data,d);
	};
	
	bool next() { return next(data)==0; };
	
	bool operator++() { return curget(data,DB_NEXT)==0;};
	bool operator--() { return curget(data,DB_PREV)==0;};
	bool operator++(int) { return curget(data,DB_NEXT)==0;};
	bool operator--(int) { return curget(data,DB_PREV)==0;};
	operator DS&()
	{
		return data;
	};
	bool end(){return select(SELECT_END,data)==0;};
	bool begin(){return select(SELECT_START,data)==0;};
	bool operator>(E const &k) { return select(SELECT_GT,k,data)==0; };
	bool operator>=(E const &k) { return select(SELECT_GTE,k,data)==0; };
	bool operator<=(E const &k) { return select(SELECT_LTE,k,data)==0; };
	bool operator<(E const &k) { return select(SELECT_LT,k,data)==0; };
	bool operator==(E const &k) { return select(SELECT_EQ,k,data)==0; };
};



inline int keycmp(int *k1,int *k2)
{
	if(*k1<*k2)
		return -1;
	if(*k1==*k2)
		return 0;
	//else *k1>*k2
		return +1;
}

inline int keycmp(long *k1,long *k2)
{
	if(*k1<*k2)
		return -1;
	if(*k1==*k2)
		return 0;
	//else *k1>*k2
		return +1;
}

typedef enum { UNIQUE , NOT_UNIQUE } unique_t;

class DB_Base {
protected:
	Db *db_primary;
	Db *db;
	char *db_name;
	DBTYPE db_type;

public:
	Db *get_primary_db() { return db_primary; };
	Db *get_db() { return db; };
	
	DB_Base(Environment &env,char *name,DBTYPE type,unique_t u = UNIQUE)
	{
		db = new Db(env.getenv(),0);
		db_name = new char [strlen(name+1)];
		if(u==NOT_UNIQUE) {
			db->set_flags(DB_DUP);
		}
		strcpy(db_name,name);
		db_type=type;
	};
	~DB_Base()
	{
		delete db;
		delete [] db_name;
	};
	void open()
	{
		db->open(NULL,db_name,NULL,db_type,DB_THREAD,0);
	};
	void create()
	{
		db->open(NULL,db_name,NULL,db_type,DB_THREAD | DB_CREATE,0);
	};
	void close()
	{
		db->close(0);
	};
};

class Texts_Collector : public DB_Base
{
	static int cmp(Db *db, const Dbt *dbt1, const Dbt *dbt2)
	{
		long *key1=(long*)dbt1->get_data();
		long *key2=(long*)dbt2->get_data();
		return keycmp(key1,key2);
	}
public:
	Texts_Collector(Environment &env,char *name) :
		DB_Base(env,name,DB_BTREE) 
	{
		db->set_bt_compare(cmp);
	};
	
	char *get(long id) {
		Dbt key(&id,sizeof(long));
		Dbt val;
		
		val.set_flags(DB_DBT_MALLOC);
		
		if(db->get(NULL,&key,&val,0)) {
			return NULL;
		}
		return (char*)val.get_data();
	};
	
	bool put(long id,char *text,int len=-1)
	{
		if(len<0) 
			len=strlen(text);
		Dbt data(text,len+1);
		Dbt key(&id,sizeof(long));
		return db->put(NULL,&key,&data,0)==0;
	};
	
	long add(char *text,int len=-1)
	{
		Dbc *cur;
		db->cursor(NULL,&cur,0);
		long id;
		Dbt key;
		
		key.set_data(&id);
		key.set_ulen(sizeof(long));
		key.set_flags(DB_DBT_USERMEM);
		
		Dbt tmpdat;
		char buf;
		
		tmpdat.set_dlen(1);
		tmpdat.set_doff(0);
		tmpdat.set_data(&buf);
		tmpdat.set_flags(DB_DBT_PARTIAL);
		
		if(cur->get(&key,&tmpdat,DB_LAST)==DB_NOTFOUND) {
			id=0;
		}
		if(len<0) len=strlen(text);
		while (true) {
			id++;
			Dbt data(text,len+1);
			Dbt key(&id,sizeof(long));
			int res=db->put(NULL,&key,&data,DB_NOOVERWRITE);
			if(res==0)
				break;
			if(res==DB_KEYEXIST)
				continue;
			cur->close();
			throw "Failed for unknown reason";
		}
		cur->close();
		return id;
	};
	bool update(long id,char *text,int len=-1)
	{
		if(len<0) len=strlen(text);
		Dbt key(&id,sizeof(long));
		Dbt data(text,len+1);
		return db->put(NULL,&key,&data,0)==0;
	};
};

template<class DS,typename E>
class	Index_Base: public DB_Base {
protected:

	void setup_dbt(Dbt &dbt,DS &data)
	{
		dbt.set_data(&data);
		dbt.set_ulen(sizeof(DS));
		dbt.set_flags(DB_DBT_USERMEM);
	};
	
	void setup_dbt(Dbt &dbt,E &data)
	{
		dbt.set_data(&data);
		dbt.set_ulen(sizeof(E));
		dbt.set_flags(DB_DBT_USERMEM);
	};

public:
//////////////

	bool remove(E &k) { 
		Dbt key(&k,sizeof(E));
		return db->del(NULL,&key,0)==0;
	};
	
	bool get(E k,DS &data) {
		Dbt key(&k,sizeof(E));
		Dbt val;
		
		setup_dbt(val,data);
		
		return db->get(NULL,&key,&val,0)==0;
	};

	Index_Base(Environment &env,char *name,DBTYPE type,
		DB_Base *primary = NULL, unique_t u=UNIQUE) 
		: DB_Base(env,name,type,u)
	{
		if(primary) {
			db_primary=primary->get_db();
		}
		else {
			db_primary=NULL;
		}
	};
	
	~Index_Base() {
	};
	
	static int keycmp(E &key1,E &key2)
	{
		int res;
		if(key1<key2) {
			res=-1;
		}
		else if(key1>key2) {
			res=1;
		}
		else {
			res=0;
		}
		return res;
	};
	static int cmp(Db *db, const Dbt *dbt1, const Dbt *dbt2)
	{
		E key1;
		memcpy(&key1,dbt1->get_data(),sizeof(E));
		E key2;
		memcpy(&key2,dbt2->get_data(),sizeof(E));
		return keycmp(key1,key2);
	}
	
	typedef cursor<DS,E> cursor_t;
	
	static int cmpcur(void const *p1,void const *p2)
	{
		return keycmp(*(E*)p1,*(E*)p2);
	}
	
	void init_cursor(cursor<DS,E> &cur) {
		if(this->db_type == DB_BTREE) {
			cur.cmp_fnc=cmpcur;
		}
		this->db->cursor(NULL,&cur.cur,0);
	}		
};

template<class DS,typename E,E (DS::*getmember)()>
class Index_Func: public Index_Base< DS , E  > {
	
public:
	
	bool insert(DS &data, bool overwrite = false) {
		E keyval((data.*getmember)());
		Dbt key(&keyval,sizeof(E));
		Dbt val(&data,sizeof(DS));
		return this->db->put(NULL,&key,&val,(overwrite ? 0 : DB_NOOVERWRITE))==0;
	};
	
	bool update(DS &data) {
		return insert(data,true);
	};
	
	static int get_key(Db *db, const Dbt *key,
				const Dbt *data,Dbt *secondary_key)
	{
		void *ptr=malloc(sizeof(E));
		DS *d=(DS*)data->get_data();
		E e_tmp=(d->*getmember)();
		memcpy(ptr,&e_tmp,sizeof(E));
		
		secondary_key->set_flags(DB_DBT_APPMALLOC);
		secondary_key->set_data(ptr);
		secondary_key->set_size(sizeof(E));
		return 0;
	}
	
	Index_Func(Environment &env,char *name,
			DBTYPE type,
			DB_Base *primary = NULL,
		  	unique_t u=UNIQUE):
		Index_Base<DS,E>(env,name,type,primary,u)
	{
		if(type == DB_BTREE) {
			this->db->set_bt_compare(this->cmp);
		}
		if(primary) {
			primary->get_db()->associate(NULL,this->db,get_key,0);
		}
	};

};

template<class DS,typename E,E DS::*member>
class Index_Var: public Index_Base< DS , E  > {
	
public:
	
	bool insert(DS &data, bool overwrite = false) {
		E keyval(data.*member);
		Dbt key(&keyval,sizeof(E));
		Dbt val(&data,sizeof(DS));
		return this->db->put(NULL,&key,&val,(overwrite ? 0 : DB_NOOVERWRITE))==0;
	};
	
	bool update(DS &data) {
		return insert(data,true);
	};
	
	static int get_key(Db *db, const Dbt *key,
				const Dbt *data,Dbt *secondary_key)
	{
		DS *d=(DS*)data->get_data();
		E *e_tmp=&(d->*member);
		
		secondary_key->set_data(e_tmp);
		secondary_key->set_size(sizeof(E));
		return 0;
	}
	
	Index_Var(Environment &env,char *name,
			DBTYPE type,
			DB_Base *primary = NULL,
			unique_t u=UNIQUE):
		Index_Base<DS,E>(env,name,type,primary,u)
	{
		if(type == DB_BTREE) {
			this->db->set_bt_compare(this->cmp);
		}
		if(primary) {
			primary->get_db()->associate(NULL,this->db,get_key,0);
		}
	};

};

template<class DS,typename E,E DS::*member>
class Index_Auto_Increment : Index_Var <DS,E,member> {
	Index_Auto_Increment(Environment &env,char *name,
			     DBTYPE type,
			     DB_Base *primary = NULL) : 
			Index_Var<DS,E,member>(env,name,type,primary,UNIQUE)
	{
		// Nothing
	};
	E add(DS &indata)
	{
		Dbc *cur;
		this->db->cursor(NULL,&cur,0);
		E id;
		Dbt key;
		
		key.set_data(&id);
		key.set_ulen(sizeof(E));
		key.set_flags(DB_DBT_USERMEM);
		
		Dbt tmpdat;
		char buf;
		
		tmpdat.set_dlen(1);
		tmpdat.set_doff(0);
		tmpdat.set_data(&buf);
		tmpdat.set_flags(DB_DBT_PARTIAL);
		
		if(cur->get(&key,&tmpdat,DB_LAST)==DB_NOTFOUND) {
			id=0;
		}
		
		while (true) {
			id++;
			(indata.*member)=id;
			Dbt data(&indata);
			Dbt key(&id,sizeof(E));
			int res=this->db->put(NULL,&key,&data,DB_NOOVERWRITE);
			if(res==0)
				break;
			if(res==DB_KEYEXIST)
				continue;
			cur->close();
			throw "Failed for unknown reason";
		}
		cur->close();
		return id;
	};
};


} // end of namespace fastcms

#endif
