#include "base_view.h"
#include "worker_thread.h"
#include "util.h"
#include <boost/format.hpp>

namespace cppcms {


base_view::settings::settings(worker_thread *w) : worker(w) , output(&w->get_cout()) {};
base_view::settings::settings(worker_thread *w,ostream *o) : worker(w), output(o) {};

string base_view::escape(string const &s)
{
	return cppcms::escape(s);
}

string base_view::urlencode(string const &s)
{
	return cppcms::urlencode(s);
}

string base_view::intf(int val,string f)
{
	return (boost::format(f) % val).str();
}

string base_view::strftime(std::tm const &t,string f)
{
	char buf[128];
	buf[0]=0;
	std::strftime(buf,sizeof(buf),f.c_str(),&t);
	return buf;
}

void base_view::set_domain(char const *s)
{
	tr=worker.domain_gettext(s);
}

char const *base_view::gettext(char const *s)
{
	return tr->gettext(s);
}

char const *base_view::ngettext(char const *s,char const *s1,int m)
{
	return tr->ngettext(s,s1,m);
}


namespace details {

views_storage &views_storage::instance() {
	static views_storage this_instance;
	return this_instance;
};

void views_storage::add_view(string t,string v,view_factory_t f)
{
	storage[t][v]=f;
}

void views_storage::remove_views(string t)
{
	storage.erase(t);
}

base_view *views_storage::fetch_view(string t,string v,base_view::settings s,base_content *c)
{
	templates_t::iterator p=storage.find(t);
	if(p==storage.end()) return NULL;
	template_views_t::iterator p2=p->second.find(v);
	if(p2==p->second.end()) return NULL;
	view_factory_t &f=p2->second;
	return f(s,c);
}

};

}// CPPCMS
