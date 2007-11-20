#include <iostream>
#include "main_thread.h"
#include "thread_pool.h"
#include "global_config.h"
#include "url.h"


using namespace std;

URL_Def urls[] = 
{	
	{ "^/login$",	1 },
	{ "^/logout$",	2 },
	{ "^/newpost$",	3 },
	{ "^/post$",	4 },
	{ "^/dologin$",	5 },
	{ "^.*$",	0 },
	{ NULL }
};

	
int main(int argc,char **argv)
{
	try{
		URL_Parser::add(urls);
		Run_Application<Main_Thread>(argc,argv);		
		cout<<"Exiting\n";
	}
	catch(HTTP_Error &s) {
		cerr<<s.get()<<endl;
		return 1;
	}
	return 0;
}
