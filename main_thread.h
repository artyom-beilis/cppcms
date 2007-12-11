#ifndef _MAIN_THREAD_H
#define _MAIN_THREAD_H


#include "worker_thread.h"
#include "http_error.h"

#include <cgicc/HTTPStatusHeader.h>
#include <cgicc/HTTPRedirectHeader.h>
#include "easy_bdb.h"
#include "data.h"

using namespace cgicc;
using namespace ebdb;

extern auto_ptr<Users> users;
extern auto_ptr<Messages> all_messages;
extern auto_ptr<Texts_Collector> texts;


class Main_Thread : public Worker_Thread {
	URL_Parser url;
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
	void show_main_page(string from);
	void show_login();
	void show_logout();
	void get_post_message();
	void load_cookies();
	void load_inputs();
	void do_login();
	void show_post_form();
	void edit_message(string s);
protected:

	void check_athentication();
	void check_athentication_by_name(string,string);
	void get_parameters();

	virtual void main();

	string protect(string const &str);

public:
	virtual void init();
	Main_Thread() {};
	virtual ~Main_Thread() {};
};


#endif /* _MAIN_THREAD_H */
