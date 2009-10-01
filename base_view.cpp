#define CPPCMS_SOURCE
#include "base_view.h"
#include "util.h"
#include "locale_gettext.h"
#include "locale_environment.h"
#include "locale_convert.h"
#include "cppcms_error.h"

#include <vector>
#include <boost/format.hpp>

namespace cppcms {

struct base_view::data {
	std::ostream *out;
	locale::gettext::tr const *tr;
};

base_view::base_view(std::ostream &out) :
	d(new data)
{
	d->out=&out;
}

std::ostream &base_view::out()
{
	return *d->out;
}

base_view::~base_view()
{
}

void base_view::render()
{
}


namespace details {

struct views_storage::data {};

views_storage &views_storage::instance() {
	static views_storage this_instance;
	return this_instance;
};

void views_storage::add_view(std::string t,std::string v,view_factory_type f)
{
	storage[t][v]=f;
}

void views_storage::remove_views(std::string t)
{
	storage.erase(t);
}

std::auto_ptr<base_view> views_storage::fetch_view(std::string t,std::string v,std::ostream &s,base_content *c)
{
	templates_type::iterator p=storage.find(t);
	if(p==storage.end())
		throw cppcms_error("Can't find skin "+t);
	template_views_type::iterator p2=p->second.find(v);
	if(p2==p->second.end())
		throw cppcms_error("Can't find template "+v);
	view_factory_type &f=p2->second;
	return f(s,c);
}
views_storage::views_storage()
{
}

views_storage::~views_storage()
{
}


} // details
}// cppcms
