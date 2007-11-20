#ifndef _MAIN_THREAD_H
#define _MAIN_THREAD_H


#include "worker_thread.h"
#include "mysql_db.h"
#include <mysql/mysql.h>
#include <cgicc/HTTPStatusHeader.h>
#include <cgicc/HTTPRedirectHeader.h>

using namespace mysql_wrapper;
using namespace cgicc;

class Main_Thread : public Worker_Thread {
// Internal Data	
	MySQL_DB db;
// User Data
	int user_id;
	string username;
	string visitor;
	string email;
	string vurl;
	string password;
	bool authenticated;
	string new_username;
	string new_password;
// Other
	int page;
	string message;
// Functions 	
	void show_page();
	void show_main_page();
	void show_login();
	void show_logout();
	void get_post_message();
	void load_cookies();
	void load_inputs();
	void do_login();
	void show_post_form();
	void printhtml(char const *text);
protected:
	
	void check_athentication();
	void get_parameters();

	virtual void main();

	string protect(string const &str);

public:
	virtual void init();
	Main_Thread() {};
	virtual ~Main_Thread() {};
};


#endif /* _MAIN_THREAD_H */
