#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include <pthread.h>
#include <sstream>
#include <string>

#include "textstream.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTTPStatusHeader.h"
#include "cgicc/HTMLClasses.h"
#include <memory>

#include "FCgiIO.h"

#include "config.h"

using namespace std;
using namespace cgicc;

class HTTP_Error {
	bool State404;
	char message[MAX_ERROR_MESSAGE_LEN];
public:
	char const *get() { return message; };
	bool is_404() { return State404; };
	HTTP_Error(char const *text,bool NotFound = false)
	{
		strncpy(message,text,MAX_ERROR_MESSAGE_LEN); 
		State404 = NotFound;
	};
	HTTP_Error(string const &str,bool NotFound = false) 
	{ 
		strncpy(message,str.c_str(),MAX_ERROR_MESSAGE_LEN); 
		State404 = NotFound;
	};
};

class Worker_Thread {
	auto_ptr<FCgiIO> io;
protected:	
	auto_ptr<Cgicc> cgi;
	CgiEnvironment const *env;

	Text_Stream out;
	auto_ptr<HTTPHeader> response_header;
	void set_header(HTTPHeader*h){response_header=auto_ptr<HTTPHeader>(h);};
	virtual void main();
public:
	int id;
	pthread_t pid;


	void run(FCGX_Request *req);

	Worker_Thread();
	virtual ~Worker_Thread(){};
	virtual void init() {};
};

#endif
