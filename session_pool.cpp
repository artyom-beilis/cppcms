#define CPPCMS_SOURCE
#include "session_pool.h"

namespace cppcms {
struct session_pool::data 
{
};

session_pool::session_pool(service &srv)
{
/*	std::string location=srv.settings().get("session.location","none");
	std::string encryptor=srv.settings().get("session.cookies_encryptor","");
	std::string storage=srv.settings().get("session.backend","");

	if(encryptor=="hmac" || encryptor=="aes") {
		std::string key = srv.settings().get<std::string>("session.cookies_key");
	}*/
}

session_pool::~session_pool()
{
}

intrusive_ptr<session_api> session_pool::get()
{
	return 0;
}

} /// cppcms
