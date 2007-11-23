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

namespace db_wrapper {
	
using namespace std;

#define DB_ERR_MAX_LEN 128

typedef char **DB_Row;

class DB_Escape {
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
	DB_Escape &operator<<(char const *str);
	DB_Escape &operator<<(string const &str)
	{
		return *this<<str.c_str();
	};
	DB_Escape &operator<<(int v)
	{
		return *this<<(long)v;
	};
	DB_Escape &operator<<(long);
	DB_Escape &operator<<(double);
	char *get(MYSQL *conn);
	DB_Escape(char const *format);
	~DB_Escape() { delete [] query; };
};

typedef DB_Escape escape;


	
class DB_Err {
	char message[DB_ERR_MAX_LEN];
public:
	char const *get() { return message; };
	DB_Err(char const *text)
	{
		strncpy(message,text,DB_ERR_MAX_LEN); 
		message[DB_ERR_MAX_LEN-1]=0;
	};
	DB_Err(string const &str) 
	{ 
		strncpy(message,str.c_str(),DB_ERR_MAX_LEN); 
		message[DB_ERR_MAX_LEN-1]=0;
	};
};

class DB_Res {
	MYSQL_RES *res;
public:
	DB_Res() { res=NULL; };
	DB_Res(MYSQL_RES *result) {
		res=result;
	};
	void free() { if(res) mysql_free_result(res); };
	~DB_Res() { free(); };
	DB_Row next() { return mysql_fetch_row(res); };
	void operator=(MYSQL_RES *result) { free(); res=result; };
	int cols() { return mysql_num_fields(res); };
	int rows() { return mysql_num_rows(res); };
};


class Data_Base {
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
	MYSQL_RES *query(DB_Escape &e) { return query(e.get(conn)); };
	MYSQL_RES *query(string const &s) { return query(s.c_str()); };

	void exec(char const *);
	void exec(DB_Escape &e) { exec(e.get(conn)); };
	void exec(string const &s) { exec(s.c_str()); };

	
	void close() { if(conn) mysql_close(conn); conn=NULL; };
	void open(string const &h,string const &u,string const &p,string const &d);
	void open();
	
	Data_Base() { conn=NULL; setup=false; };
	~Data_Base() { if(conn) mysql_close(conn); };
	
};

}
#endif /* _MYSQL_DB_H */
