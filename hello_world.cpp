#include "worker_thread.h"
#include "manager.h"
using namespace cppcms;

class my_hello_world : public worker_thread {
public:
	my_hello_world(worker_settings const &s) :  worker_thread(s) {};
	virtual void main();
};

void my_hello_world::main()
{
	out+="<h1>Simple Hello World test application.</h1>\n";
}

int main(int argc,char ** argv)
{
	try {
		run_application(argc,argv,simple_factory<my_hello_world>());
	}
	catch(std::exception const &e) {
		cerr<<e.what()<<endl;
	}
}
