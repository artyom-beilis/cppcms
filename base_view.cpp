#define CPPCMS_SOURCE
#include "base_view.h"
#include "util.h"
#include "locale_gettext.h"
#include "locale_environment.h"
#include "base64.h"
#include "locale_info.h"
#include "utf_iterator.h"
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
	set_domain("");
}

void base_view::set_domain(std::string domain)
{
	if(std::has_facet<cppcms::locale::gettext>(out().getloc())) {
		if(!domain.empty())
			d->tr = &std::use_facet<cppcms::locale::gettext>(out().getloc()).dictionary(domain.c_str());
		else
			d->tr = &std::use_facet<cppcms::locale::gettext>(out().getloc()).dictionary();
	}
	else {
		static const locale::gettext::tr t;
		d->tr=&t;
	}
}

std::ostream &base_view::out()
{
	return *d->out;
}

char const *base_view::gettext(char const *s)
{
	return d->tr->gettext(s);
}

char const *base_view::ngettext(char const *s,char const *s1,int m)
{
	return d->tr->ngettext(s,s1,m);
}

base_view::~base_view()
{
}

void base_view::render()
{
}



void base_view::date(std::ostream &s,std::tm const &t)
{
	if(s.getloc().name()!="C")
		strftime("%x")(s,t);
	else
		strftime("%Y-%m-%d")(s,t);
}

void base_view::time(std::ostream &s,std::tm const &t)
{
	if(s.getloc().name()!="C")
		strftime("%X")(s,t);
	else
		strftime("%H:%M:%S")(s,t);
}

base_view::strftime::strftime(std::string f) : format_(f)
{
}

base_view::strftime::~strftime()
{
}

void base_view::strftime::operator()(std::ostream &out,std::tm const &time) const
{
	char const *begin=format_.data();
	char const *end=begin+format_.size();
	std::use_facet<std::time_put<char> >(out.getloc()).put(out,out,' ',&time,begin,end);
}

void base_view::urlencode(std::ostream &out,std::string const &s)
{
	out << util::urlencode(s); 
}

void base_view::base64_encode(std::ostream &os,std::string const &s)
{
	using namespace cppcms::b64url;
	unsigned char const *begin=reinterpret_cast<unsigned char const *>(s.c_str());
	unsigned char const *end=begin+s.size();
	std::vector<unsigned char> out(encoded_size(s.size())+1);
	encode(begin,end,&out.front());
	out.back()=0;

	char const *buf=reinterpret_cast<char const *>(out.front());
	os<<buf;
}

namespace {

void to_something(	std::ostream &out,
			std::string const &s,
			char (std::ctype<char>::*narop)(char) const,
			wchar_t (std::ctype<wchar_t>::*wideop)(wchar_t) const)
{
	using namespace std;

	std::locale l=out.getloc();
	if(!has_facet<locale::info>(l)
	   || !use_facet<locale::info>(l).is_utf8()
	   || !has_facet<ctype<wchar_t> >(l))
	{
		ctype<char> const &converter=use_facet<ctype<char> >(l);
		for(unsigned i=0;i<s.size();i++)
			out.put((converter.*narop)(s[i]));
		return;
	}
	ctype<wchar_t> const &converter=use_facet<ctype<wchar_t> >(l);
	
	std::string::const_iterator p=s.begin(),e=s.end();
	
	uint32_t code_point;

	while(p!=e && (code_point=cppcms::utf8::next(p,e,false,true))!=utf::illegal) {
		if(sizeof(wchar_t) == 4 || code_point <=0xFFFF)
			code_point=(converter.*wideop)(code_point);
		utf8::seq s=utf8::encode(code_point);
		unsigned i=0;
		do {
			out.put(s.c[i]);
			i++;
		}while(i<sizeof(s.c) && s.c[i]!=0);
	}
	if(p!=e) {
		out.write(s.c_str() + (p-s.begin()), (e-p));
	}

}
} // anonymous
void base_view::to_lower(std::ostream &out,std::string const &s)
{
	to_something(out,s,&std::ctype<char>::tolower,&std::ctype<wchar_t>::tolower);
}

void base_view::to_upper(std::ostream &out,std::string const &s)
{
	to_something(out,s,&std::ctype<char>::toupper,&std::ctype<wchar_t>::toupper);
}

base_view::doublef::~doublef(){}
base_view::intf::~intf(){}

base_view::doublef::doublef(std::string f) : format_(f) {}
base_view::intf::intf(std::string f) : format_(f) {}

void base_view::intf::operator()(std::ostream &out,int x) const
{
	boost::format f(format_);
	f.exceptions(0);
	out<<f % x;
}

void base_view::doublef::operator()(std::ostream &out,double x) const
{
	boost::format f(format_);
	f.exceptions(0);
	out<<f % x;
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
