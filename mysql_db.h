#ifndef _MYSQL_DB_H
#define _MYSQL_DB_H

#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdarg.h>
#include <boost/scoped_array.hpp>
#include <mysql/mysql.h>
#include <vector>
#include <iostream>

namespace mysql_wrapper {
	
using namespace std;

#define DB_ERR_MAX_LEN 128

typedef MYSQL_ROW MySQL_DB_Row;

class MySQL_DB_Escape {
	typedef enum { LONG, DOUBLE, STRING } types;
	struct element {
		types type;
		int length;
		union {
			char const *str_val;
			long long_val;
			double double_val;
		} data;
	};
	
	vector<element> parameters;
	char *query;
	char const *format;
	int calc_len();
	void escape_str(MYSQL *conn);
public:
	MySQL_DB_Escape &operator<<(char const *str);
	MySQL_DB_Escape &operator<<(string const &str)
	{
		return *this<<str.c_str();
	};
	MySQL_DB_Escape &operator<<(int v)
	{
		return *this<<(long)v;
	};
	MySQL_DB_Escape &operator<<(long);
	MySQL_DB_Escape &operator<<(double);
	char *get(MYSQL *conn);
	MySQL_DB_Escape(char const *format);
	~MySQL_DB_Escape() { delete [] query; };
};

typedef MySQL_DB_Escape escape;


	
class MySQL_DB_Err {
	char message[DB_ERR_MAX_LEN];
public:
	char const *get() { return message; };
	MySQL_DB_Err(char const *text)
	{
		strncpy(message,text,DB_ERR_MAX_LEN); 
		message[DB_ERR_MAX_LEN-1]=0;
	};
	MySQL_DB_Err(string const &str) 
	{ 
		strncpy(message,str.c_str(),DB_ERR_MAX_LEN); 
		message[DB_ERR_MAX_LEN-1]=0;
	};
};

class MySQL_DB_Res {
	MYSQL_RES *res;
public:
	MySQL_DB_Res(MYSQL_RES *result) {
		res=result;
	};
	void free() { mysql_free_result(res); };
	~MySQL_DB_Res() { free(); };
	MySQL_DB_Row next() { return mysql_fetch_row(res); };
	void operator=(MYSQL_RES *result) { free(); res=result; };
	int cols() { return mysql_num_fields(res); };
	int rows() { return mysql_num_rows(res); };
};


class MySQL_DB {
	bool setup;
	MYSQL *conn;
	string host;
	string username;
	string password;
	string database;
	void connect();
	void exec_query(char const *q);
public:
	MYSQL_RES *query(char const *);
	MYSQL_RES *query(MySQL_DB_Escape &e) { return query(e.get(conn)); };
	MYSQL_RES *query(string const &s) { return query(s.c_str()); };

	void exec(char const *);
	void exec(MySQL_DB_Escape &e) { exec(e.get(conn)); };
	void exec(string const &s) { exec(s.c_str()); };

	
	void close() { if(conn) mysql_close(conn); conn=NULL; };
	void open(string const &h,string const &u,string const &p,string const &d);
	void open();
	
	MySQL_DB() { conn=NULL; setup=false; };
	~MySQL_DB() { if(conn) mysql_close(conn); };
	
};

}
#endif /* _MYSQL_DB_H */
