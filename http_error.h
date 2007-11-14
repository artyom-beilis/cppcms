#ifndef _HTTP_ERROR_H
#define _HTTP_ERROR_H

#include <string>
using namespace std;

#define HTTP_ERROR_MAX_ERROR_MESSAGE_LEN 256


class HTTP_Error {
	bool State404;
	char message[HTTP_ERROR_MAX_ERROR_MESSAGE_LEN];
public:
	char const *get() { return message; };
	bool is_404() { return State404; };
	HTTP_Error(char const *text,bool NotFound = false)
	{
		strncpy(message,text,HTTP_ERROR_MAX_ERROR_MESSAGE_LEN); 
		State404 = NotFound;
	};
	HTTP_Error(string const &str,bool NotFound = false) 
	{ 
		strncpy(message,str.c_str(),HTTP_ERROR_MAX_ERROR_MESSAGE_LEN); 
		State404 = NotFound;
	};
};

#endif /* _HTTP_ERROR_H */
