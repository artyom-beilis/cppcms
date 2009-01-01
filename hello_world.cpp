#include "application.h"
#include "manager.h"
#include "hello_world_view.h"
using namespace cppcms;

class my_hello_world : public application {
public:
	my_hello_world(worker_thread &w) :
		application(w)
	{
		url.add("^/?$",
			boost::bind(&my_hello_world::std,this));
		url.add("^/test?$",boost::bind(&my_hello_world::test,this));
		use_template("view2");
	};
	void test();
	void std();
};

void my_hello_world::test()
{
	if(!session.is_set("time")) {
		cout<<"No Time\n";
	}
	else {
		time_t given=session.get<time_t>("time");
		cout<<asctime(gmtime(&given))<<"<br/>\n";
		if(session.is_set("msg")) {
			cout<<session["msg"]<<"<br/>";
		}
		if(given % 3 == 0) {
			cout<<"SET LONG MESSAGE";
			session["msg"]="Looooooooooooooooooooooooooooooong msg";
		}
		else {
			cout<<"UNSET LONG MESSAGE";
			session.del("msg");
		}
		//session.clear();
	}
	session.set<time_t>("time",time(NULL));
}

void my_hello_world::std()
{
	view::hello v(this);

	if(env->getRequestMethod()=="POST") {
		v.form.load(*cgi);
		if(v.form.validate()) {
			session["name"]=v.form.username.get();
			v.username=v.form.username.get();
			v.realname=v.form.name.get();
			v.ok=v.form.ok.get();
			v.password=v.form.p1.get();
			v.form.clear();
		}
	}

	v.title="Cool";
	if(session.is_set("name"))
		v.title+=":"+session["name"];

	v.msg=gettext("Hello World");

	for(int i=0;i<15;i++)
		v.numbers.push_back(i);
	v.lst.push_back(view::data("Hello",10));
	render("hello",v);
}

int main(int argc,char ** argv)
{
	try {
		manager app(argc,argv);
		app.set_worker(new application_factory<my_hello_world>());
		app.execute();
	}
	catch(std::exception const &e) {
		cerr<<e.what()<<endl;
	}
}
