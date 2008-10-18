#ifndef CPPCMS_BASEVIEW_H
#define CPPCMS_BASEVIEW_H

#include <boost/format.hpp>
#include <boost/function.hpp>
#include <ostream>
#include <sstream>
#include <string>
#include <map>
#include "worker_thread.h"
#include "cppcms_error.h"
#include "config.h"

namespace cppcms {
using namespace std;

// Just simple polimorphic class
class base_content {
public:
	virtual ~base_content() {};
};

class base_view {
public:
	struct settings {
		worker_thread *worker;
		ostream *output;
		settings(worker_thread *w) : worker(w) , output(&w->get_cout()) {};
		settings(worker_thread *w,ostream *o) : worker(w), output(o) {};
	};
protected:
	worker_thread &worker;
	ostream &cout;

	base_view(settings s) :
		worker(*s.worker),
		cout(*s.output)
	{
	}

	template<typename T>
	string escape(T const &v)
	{
		ostringstream s;
		s<<v;
		return s.str();
	};

	string escape(string const &s);

	inline string raw(string s) { return s; };
	inline string intf(int val,string f){
		return (format(f) % val).str();
	};
	string strftime(std::tm const &t,string f)
	{
		char buf[128];
		buf[0]=0;
		std::strftime(buf,sizeof(buf),f.c_str(),&t);
		return buf;
	};
	string date(std::tm const &t) { return strftime(t,"%Y-%m-%d"); };
	string time(std::tm const &t) { return strftime(t,"%H:%M"); };
	string timesec(std::tm const &t) { return strftime(t,"%T"); };
	string escape(std::tm const &t) { return strftime(t,"%Y-%m-%d %T"); }

	string urlencode(string const &s);

	inline boost::format format(string const &f){
		boost::format frm(f);
		frm.exceptions(0);
		return frm;
	};
public:
	virtual void render() {};
	virtual ~base_view() {};
};

namespace details {

template<typename T,typename VT>
struct view_builder {
        base_view *operator()(base_view::settings s,base_content *c) {
		VT *p=dynamic_cast<VT *>(c);
		if(!p) throw cppcms_error("Incorrect content type");
		return new T(s,*p);
	};
};

class views_storage {
public:
	typedef boost::function<base_view *(base_view::settings s,base_content *c)> view_factory_t;
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
