#include "mysql_db.h"
#include <boost/scoped_array.hpp>
#include <iostream>

#include "global_config.h"

namespace mysql_wrapper {

MySQL_DB_Escape::MySQL_DB_Escape(char const *format)
{
	query=NULL;
	this->format=format;
	parameters.reserve(16);
}

MySQL_DB_Escape &MySQL_DB_Escape::operator<<(long val)
{
	element e;
	e.data.long_val=val;
	e.type=LONG;
	e.length=snprintf(NULL,0,"%ld",val);
	parameters.push_back(e);
	return *this;
}

MySQL_DB_Escape &MySQL_DB_Escape::operator<<(double val)
{
	element e;
	e.data.double_val=val;
	e.type=DOUBLE;
	e.length=snprintf(NULL,0,"%e",val);
	parameters.push_back(e);
	return *this;
}

MySQL_DB_Escape &MySQL_DB_Escape::operator<<(char const *str)
{
	element e;
	e.data.str_val=str;
	e.type=STRING;
	e.length=2*strlen(str)+1;
	parameters.push_back(e);
	return *this;
}

char *MySQL_DB_Escape::get(MYSQL *conn)
{
	int overall_len=calc_len();
	query=new char [overall_len+1];
	escape_str(conn);
	return query;
}

int MySQL_DB_Escape::calc_len()
{
	int id,len=0,i,j;
	for(i=0;format[i];i++) {
		if(format[i]=='%' && format[i+1]!='%') {
			id=0;
			for(j=i+1;format[j]!='%';j++) {
				if('0'<format[j] && format[j]<'9') {
					id=id*10+format[j]-'0';
				}
				else {
					throw MySQL_DB_Err("Invalid format");
				}
			}
			i=j;
			id--;
			if(id>=0 && id<(int)parameters.size()) {
				len+=parameters[id].length;
			}
		}
		else if(format[i]=='%' && format[i+1]=='%') {
			i++;
		}
	}
	return i+len;
}

void MySQL_DB_Escape::escape_str(MYSQL *conn)
{
	char *q=query;
	int i,j,id;
	for(i=0;format[i];i++) {
		if(format[i]=='%' && format[i+1]!='%') {
			id=0;
			for(j=i+1;format[j]!='%';j++) {
				if('0'<format[j] && format[j]<'9') {
					id=id*10+format[j]-'0';
				}
				else {
					throw MySQL_DB_Err("Invalid format");
				}
			}
			i=j;
			id--;
			if(id>=0 && id<(int)parameters.size()) {
				char const *ptr;
				switch(parameters[id].type) {
				case LONG: 
					q+=sprintf(q,"%ld",parameters[id].data.long_val);
					break;
				case DOUBLE:
					q+=sprintf(q,"%e",parameters[id].data.double_val);
					break;
				case STRING:
					ptr=parameters[id].data.str_val;
					q+=mysql_real_escape_string(conn,
								q,
								ptr,
								strlen(ptr));
				}
			}
		}
		else if(format[i]=='%' && format[i+1]=='%') {
			*q='%';
			q++;
			i++;
		}
		else {
			*q=format[i];
			q++;
		}
	}
	*q=0;
}

void MySQL_DB::connect()
{
	conn=mysql_init(NULL);
	if(!conn){
		throw MySQL_DB_Err("No memory");
	}
	if(!mysql_real_connect(	conn,
				host.c_str(),
				username.c_str(),
				password.c_str(),
				database.c_str(),
				0,NULL,0))
	{
		close();
		throw MySQL_DB_Err("Failed to connect to database");
	}
}

void MySQL_DB::open(string const &h,string const &u,string const &p,string const &d)
{
	host=h;
	username=u;
	password=p;
	database=d;
	setup=true;
	connect();
}

void MySQL_DB::open()
{
	open(	global_config.sval("mysql.host"),
		global_config.sval("mysql.username"),
		global_config.sval("mysql.password"),
		global_config.sval("mysql.database"));
}
	
void MySQL_DB::exec_query(char const *q)
{
	bool not_try_once_more=false;
	if(!setup) throw MySQL_DB_Err("Date base must be open first");
	if(!conn){
		connect();
		not_try_once_more=true;
	}
	if(mysql_query(conn,q) && !not_try_once_more) {
		// MayBe we lost presistent connection
		close();
		connect();
		if(mysql_query(conn,q)) {
			throw MySQL_DB_Err(string("Failed to exectue the query:") + q);
		}
	}
}

void MySQL_DB::exec(char const *q)
{
	exec_query(q);
	MYSQL_RES *res=mysql_store_result(conn);
	if(res) {
		mysql_free_result(res);
		throw MySQL_DB_Err("You must not use query for operation that "
					"returns result");
	}
	if(mysql_errno(conn)) {
		throw MySQL_DB_Err(string("Error executing query:")+q);
	}
}

MYSQL_RES *MySQL_DB::query(char const *q)
{
	exec_query(q);
	MYSQL_RES *res=mysql_store_result(conn);
	if(!res || mysql_errno(conn)) {
		throw MySQL_DB_Err(string("Error executing query:")+q);
	}
	return res;
}

}
