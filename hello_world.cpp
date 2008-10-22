#include "worker_thread.h"
#include "manager.h"
#include "hello_world_view.h"
#include "form.h"
using namespace cppcms;

struct my_form : public base_form {
	widgets::text username;
	widgets::text name;
	widgets::password p1;
	widgets::password p2;
	my_form() :
		username("user","Username"),
		name("name","Real Name"),
		p1("pass","Password"),
		p2("passcopy","Confirm")
	{
		*this & username & name & p1 & p2;
		username.set_nonempty();
		username.error_msg="Empty username";
		name.set_nonempty();
		name.error_msg="Empty name";
		p2.set_equal(p1);
	}
};

class my_hello_world : public worker_thread {
public:
	my_hello_world(manager const &s) :  worker_thread(s)
	{
		use_template("view2");
	};
	virtual void main();
};

void my_hello_world::main()
{
	view::hello v;

	v.title="Cool";
	v.msg=gettext("Hello World");

	for(int i=0;i<15;i++)
		v.numbers.push_back(i);
	v.lst.push_back(view::data("Hello",10));
	v.ok=true;
	render("hello",v);
}

int main(int argc,char ** argv)
{
	try {
		manager app(argc,argv);
		app.set_worker(new simple_factory<my_hello_world>());
		app.execute();
	}
	catch(std::exception const &e) {
		cerr<<e.what()<<endl;
	}
}
