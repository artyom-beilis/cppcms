#define CPPCMS_SOURCE
#include "locale_environment.h"
#include "service.h"
#include "locale_pool.h"
#include "global_config.h"

namespace cppcms {
namespace locale {

struct environment::data {
	std::locale const *locale;
	gettext::tr const *current;
	std::string domain_name;
	std::string locale_name;
};

environment::environment(cppcms::service &srv) :
	service_(srv),
	d(new data)
{
	d->locale_name=service_.settings().str("locale.default","C");
	setup();
}

environment::~environment()
{
}


void environment::setup()
{
	d->locale=&service_.locale_pool().get(d->locale_name);

	if(std::has_facet<cppcms::locale::gettext>(*d->locale)){
		cppcms::locale::gettext const &gt=std::use_facet<cppcms::locale::gettext>(*d->locale);
		if(d->domain_name.empty())
			d->current=&gt.dictionary();
		else
			d->current=&gt.dictionary(d->domain_name.c_str());
	}
	else {
		static cppcms::locale::gettext::tr const tr;
		d->current=&tr;
	}
}

void environment::locale(std::string l)
{
	d->locale_name=l;
	setup();
}

std::string environment::locale()
{
	return d->locale_name;
}

void environment::gettext_domain(std::string s)
{
	d->domain_name=s;
	setup();
}

std::string environment::gettext_domain()
{
	return d->domain_name;
}


char const *environment::ngt(char const *s,char const *p,int n)
{
	return d->current->ngettext(s,p,n);
}
char const *environment::gt(char const *s)
{
	return d->current->gettext(s);
}

std::locale const &environment::get()
{
	return *d->locale;
}


} // locale
} // cppcms


