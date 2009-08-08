#define CPPCMS_SOURCE
#include "locale.h"
#include "service.h"
#include "locale_pool.h"
#include "global_config.h"
#include <typeinfo>

namespace cppcms {
namespace locale {

struct l10n::data {
	std::locale const *locale;
	gettext::tr const *current;
	std::string domain_name;
	std::string locale_name;
};

l10n::l10n(cppcms::service &srv) :
	service_(srv),
	d(new data)
{
	d->domain_name=service_.settings().str("locale.domain","");
	d->locale_name=service_.settings().str("locale.default","C");
	setup();
}

l10n::~l10n()
{
}


void l10n::setup()
{
	d->locale=&service_.locale_pool().get(d->locale_name);
	try {
		cppcms::locale::gettext const &gt=std::use_facet<cppcms::locale::gettext>(*d->locale);
		d->current=&gt.dictionary(d->domain_name.c_str());
	}
	catch(std::bad_cast const &e) {
		static cppcms::locale::gettext::tr const tr;
		d->current=&tr;
	}
}

void l10n::locale(std::string l)
{
	d->locale_name=l;
	setup();
}

std::string l10n::locale()
{
	return d->locale_name;
}

void l10n::gettext_domain(std::string s)
{
	d->domain_name=s;
	setup();
}

std::string l10n::gettext_domain()
{
	return d->domain_name;
}


char const *l10n::ngt(char const *s,char const *p,int n)
{
	return d->current->ngettext(s,p,n);
}
char const *l10n::gt(char const *s)
{
	return d->current->gettext(s);
}

std::locale const &l10n::get()
{
	return *d->locale;
}


} // locale
} // cppcms


