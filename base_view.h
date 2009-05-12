#ifndef CPPCMS_BASEVIEW_H
#define CPPCMS_BASEVIEW_H

#include <ostream>
#include <sstream>
#include <string>
#include <map>
#include <boost/format/format_fwd.hpp>
#include "cppcms_error.h"
#include "config.h"

namespace cppcms {
using namespace std;

// Just simple polimorphic class
class base_content {
public:
	virtual ~base_content() {};
};

namespace transtext  { class trans; }

class format {
	mutable std::auto_ptr<boost::format> impl;
	friend ostream &operator<<(ostream &,format &);
public:
	format(std::string const &s);
	format(format const &f);
	format &operator % (int n);
	format &operator % (string const &);
	format &operator % (char const *);
	string str();
	~format();
};

ostream &operator<<(ostream &,format &);

class worker_thread;
class base_view {
public:
	struct settings {
		worker_thread *worker;
		ostream *output;
		settings(worker_thread *w);
		settings(worker_thread *w,ostream *o);
	};
protected:
	worker_thread &worker;
	ostream &cout;
	transtext::trans const *tr;

	base_view(settings s) :
		worker(*s.worker),
		cout(*s.output)
	{
	}

	::cppcms::format format(std::string const &s)
	{
		return ::cppcms::format(s);
	}

	void set_domain(char const *);
	char const *gettext(char const *);
	char const *ngettext(char const *,char const *,int n);

	template<typename T>
	string escape(T const &v)
	{
		ostringstream s;
		s<<v;
		return s.str();
	};

	string escape(string const &s);

	inline string raw(string s) { return s; };
	string intf(int val,string f);
	string strftime(std::tm const &t,string f);
	string date(std::tm const &t) { return strftime(t,"%Y-%m-%d"); };
	string time(std::tm const &t) { return strftime(t,"%H:%M"); };
	string timesec(std::tm const &t) { return strftime(t,"%T"); };
	string escape(std::tm const &t) { return strftime(t,"%Y-%m-%d %T"); }
	string urlencode(string const &s);

public:
	virtual void render() {};
	virtual ~base_view() {};
};

namespace details {

template<typename T,typename VT>
base_view *view_builder(base_view::settings s,base_content *c) {
	VT *p=dynamic_cast<VT *>(c);
	if(!p) throw cppcms_error("Incorrect content type");
	return new T(s,*p);
};

class views_storage {
public:
	typedef base_view *(*view_factory_t)(base_view::settings s,base_content *c);
private:
	typedef map<string,view_factory_t> template_views_t;
	typedef map<string,template_views_t> templates_t;

	templates_t storage;
public:

	void add_view(	string template_name,
			string view_name,
			view_factory_t);
	void remove_views(string template_name);
	base_view *fetch_view(string template_name,string view_name,base_view::settings ,base_content *c);
	static views_storage &instance();
};

}; // DETAILS


}; // CPPCMS

#define cppcms_view(X)			\
	do {				\
		void X##_symbol();	\
		X##_symbol();		\
	} while(0)			


#if defined(HAVE_CPP_0X_AUTO)
#	define CPPCMS_TYPEOF(x) auto
#elif defined(HAVE_CPP_0X_DECLTYPE)
#	define CPPCMS_TYPEOF(x) decltype(x)
#elif defined(HAVE_GCC_TYPEOF)
#	define CPPCMS_TYPEOF(x) typeof(x)
#elif defined(HAVE_WORKING_BOOST_TYPEOF)
#	include <boost/typeof/typeof.hpp>
#	define CPPCMS_TYPEOF(x) BOOST_TYPEOF(x)
#else
#	error "No useful C++0x auto/decltype/typeof method for this compiler"
#endif


#endif
