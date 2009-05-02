#ifndef CPPCMS_HTTP_COOKIE_H
#define CPPCMS_HTTP_COOKIE_H

namespace cppcms { namespace http {
class cookie {
	std::string name_,value_,path_,domain_;
	bool secure_;
	unsigned max_age_;
	bool has_age_;
public:
	std::string name() const { return name_ }
	std::string value() const { return value_; }
	std::string path() const { return path_; }
	std::string domain() const { return domain_; }
	bool secure() const { return secure_; }

	void name(std::string n) { name_=n; }
	void value(std::string v) { value_=v; }
	void path(std::string p) { path_=p; }
	void domain(std::string) { domain_=d; }
	void max_age(unsigned a) { max_age_=a; has_age_=true; }
	void browser_age() { has_age_=false; }
	void secure(bool v) {secure_=v; }

	cookie(std::string name,std::string value) :
		name_(name),value_(value),secure_(false),max_age_(0),has_age_(false)
	{
	}	
	cookie() : secure_(false),max_age_(0),has_age_(0)
	{
	}
	friend std::ostream &operator<<(std::ostream,cookie);
};


} } //::cppcms::http


#endif
