#include "worker_thread.h"
#include "manager.h"
#include "hello_world_view.h"
using namespace cppcms;

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

	if(env->getRequestMethod()=="POST") {
		v.form.load(*cgi);
		v.form.validate();
	}

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
