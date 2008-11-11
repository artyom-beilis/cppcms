#include "base_view.h"
#include "application.h"
#include <list>
#include "form.h"
using namespace cppcms;
class my_hello_world;
namespace view {

struct master : public cppcms::base_content {
	std::string title;
	bool ok;
};

struct data {
	std::string name;
	int val;
	data(char const *n="",int v=0) : name(n),val(v){}
};

struct my_form : public form {
	widgets::text username;
	widgets::textarea name;
	widgets::email mail;
	widgets::password p1;
	widgets::password p2;
	widgets::number<int> integer;
	widgets::number<double> real;
	widgets::checkbox ok;
	widgets::select fruit;
	//widgets::radio fruit;
	widgets::select_multiple meat;
	widgetset my_set;
	my_form(application *w) :
		username("user",w->gettext("Username")),
		name("name",w->gettext("Real Name")),
		mail("mail"),
		p1("pass",w->gettext("Password")),
		p2("passcopy",w->gettext("Confirm")),
		integer("int",w->gettext("Integer")),
		real("real",w->gettext("Real")),
		ok("ok",w->gettext("Never save")),
		fruit("fr",w->gettext("Fruit")),
		meat("mt",2,w->gettext("Meat"))
	{
		*this & username & mail & name & p1 & p2 &
			integer & real & ok & fruit & meat ;
		my_set<< username<<mail<<name<<p1<<p2<<integer<<real<<ok<<fruit<<meat;
		username.set_nonempty();
		name.set_nonempty();
		p2.set_equal(p1);
		p2.help=w->gettext("(Same as above)");
		p1.set_nonempty();
		p2.set_nonempty();
		real.set_range(-1.0,1.5);
		fruit.add("Orange");
		fruit.add("Palm");
		meat.add("Beef");
		meat.add("<<Chicken>>");
		meat.add("Duck");
		meat.set_min(2);
		meat.help=w->gettext("At least two choises");
	}
};


struct hello : public master {
	string username,realname,password;
	bool ok;
	std::string msg;
	std::list<int> numbers;
	std::list<data> lst;
	my_form form;
	hello(application *w) : form(w) {}
};

};

