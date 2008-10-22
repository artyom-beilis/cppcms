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

struct my_form : public base_form {
	widgets::text username;
	widgets::text name;
	widgets::password p1;
	widgets::password p2;
	widgets::checkbox ok;
	my_form() :
		username("user","Username"),
		name("name","Real Name"),
		p1("pass","Password"),
		p2("passcopy","Confirm"),
		ok("ok","Never save")
	{
		*this & username & name & p1 & p2 & ok;
		username.set_nonempty();
		username.error_msg="Empty username";
		name.set_nonempty();
		name.error_msg="Empty name";
		p2.set_equal(p1);
		p1.set_nonempty();
		p2.set_nonempty();
		p1.error_msg=p2.error_msg="Empty passwords or not equal passwords";
	}
};


struct hello : public master {
	std::string msg;
	std::list<int> numbers;
	std::list<data> lst;
	my_form form;
};

};

