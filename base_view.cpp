#include "base_view.h"
#include "worker_thread.h"
#include "util.h"

namespace cppcms {

string base_view::escape(string const &s)
{
	return cppcms::escape(s);
}

string base_view::urlencode(string const &s)
{
	return cppcms::urlencode(s);
}

namespace details {

views_storage &views_storage::instance() {
	static views_storage this_instance;
	return this_instance;
};

void views_storage::add_view(string t,string v,view_factory_t f)
{
	view_factory_t &func=storage[t][v];
	if(!func.empty())
		throw cppcms_error("Duplicate template " + t+"::"+v);
	func=f;
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
