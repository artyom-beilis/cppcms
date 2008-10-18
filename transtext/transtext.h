#ifndef CPPCMS_TRANSTEXT_H
#define CPPCMS_TRANSTEXT_H

#include <map>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace cppcms {

namespace transtext {
using namespace std;

namespace lambda {
	struct plural { // INTERFACE 
		virtual int operator()(int n) const = 0;
		virtual ~plural(){};
	};
	plural *compile(char const *expr);

}; // END OF NAMESPACE LAMBDA

class trans {
public:
	trans() {};
	virtual ~trans() {};
	virtual void load(char const * locale,char const *domain_name, char const * dirname) {};
	virtual char const *gettext(char const *s) const { return s; };
	virtual char const *ngettext(char const *s,char const *p,int n) const { return n==1 ? s:p; };
	char const *operator()(char const *str) const { return gettext(str); };
	char const *operator()(char const *single,char const *plural,int n) const { return ngettext(single,plural,n); };
};

class trans_thread_safe : public trans {
	lambda::plural *converter;
	void *data;
public:
	trans_thread_safe();
	virtual ~trans_thread_safe();
	virtual void load(char const * locale,char const *domain_name, char const * dirname);
	virtual char const *gettext(char const *s) const;
	virtual char const *ngettext(char const *single,char const *plural,int n) const;
	lambda::plural const & num2idx_conv() const { return *converter; };
	int num2idx(int n) const { return (*converter)(n); };
};

class trans_factory {
	map<string,map<string,boost::shared_ptr<trans> > > langs;
	string default_lang;
	string default_domain;
public:
	trans const &get(string lang,string domain) const;
	void load(	string const &locale_dir,
			vector<string> const &lang_list,
			string const &lang_def,
			vector<string> const &domain_list,
			string const &domain_def);
	~trans_factory();
};

} // namespace transtext
 
} // namespace cppcms


#endif
