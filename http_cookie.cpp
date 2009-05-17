#include "http_cookie.h"

namespace cppcms { namespace http {

std::string cookie::name() const { return name_; }
std::string cookie::value() const { return value_; }
std::string cookie::path() const { return path_; }
std::string cookie::domain() const { return domain_; }
void cookie::max_age(unsigned age)
{ 
	has_age_=1;
	max_age_=age;
}
void cookie::browser_age()
{
	has_age_=0;
}
bool cookie::secure() const { return secure_; }
void cookie::secure(bool secure) { return secure_ = secure ? 1: 0; }

cookie::cookie() :
	secure_(0),
	has_age_(0)
		
{
	has_age_;
}




} } // cppcms::http

