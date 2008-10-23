#include "base_view.h"
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
	widgets::checkbox ok;
	widgets::select fruit;
	//widgets::radio fruit;
	widgets::select_multiple meat;
	my_form(worker_thread *w) :
		username("user",w->gettext("Username")),
		name("name",w->gettext("Real Name")),
		mail("mail","Mail"),
		p1("pass",w->gettext("Password")),
		p2("passcopy",w->gettext("Confirm")),
		ok("ok",w->gettext("Never save")),
		fruit("fr",w->gettext("Fruit")),
		meat("mt",2,w->gettext("Meat"))
	{
		*this & username & mail & name & p1 & p2 & ok & fruit & meat ;
		username.set_nonempty();
		name.set_nonempty();
		p2.set_equal(p1);
		p2.help=w->gettext("(Same as above)");
		p1.set_nonempty();
		p2.set_nonempty();
		fruit.add(1,"Orange");
		fruit.add(2,"Palm");
		meat.add(1,"Beef");
		meat.add(2,"Chicken");
		meat.add(3,"Duck");
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
	hello(worker_thread *w) : form(w) {}
};

};

