#include "worker_thread.h"
#include "manager.h"
using namespace cppcms;

class my_hello_world : public worker_thread {
public:
	my_hello_world(manager const &s) :  worker_thread(s) {};
	virtual void main();
};

void my_hello_world::main()
{
	out+="<h1>Simple Hello World test application.</h1>\n";
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
