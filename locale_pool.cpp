#define CPPCMS_SOURCE
#include "locale_pool.h"
#include "cppcms_error.h"
#include "locale_gettext.h"
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

namespace cppcms { namespace locale {

struct pool::data {
	std::vector<std::string> domains;
	typedef std::map<std::string,boost::shared_ptr<std::locale> > locales_type;
	locales_type locales;
	std::string path;
};

pool::pool(std::string path) :
	d(new data)
{
	d->path=path;
}

void pool::add_gettext_domain(std::string name)
{
	if(d->locales.empty())
		d->domains.push_back(name);
	else 
		throw cppcms_error("Can't define new domain after creation of locales");
}

void pool::load(std::string name)
{
	std::auto_ptr<gettext> gt(new gettext());
	for(unsigned i=0;i < d->domains.size();i++) {
		gt->load(name,d->path,d->domains[i]);
	}
	boost::shared_ptr<std::locale> base;
	try {
		base.reset(new std::locale(name.c_str()));
	}
	catch(std::runtime_error const &e) {
		base.reset(new std::locale("C"));
	}
	boost::shared_ptr<std::locale> combined(new std::locale(*base,gt.release()));
	d->locales[name]=combined;
}

std::locale const &pool::get(std::string const &name) const
{
	data::locales_type::const_iterator p;
	if((p=d->locales.find(name))==d->locales.end()) {
	std::map<std::string,boost::shared_ptr<std::locale> > locales;
		return std::locale::classic();
	}
	return *p->second;
}


pool::~pool()
{
}




} } // cppcms::locale
