#include <iostream>
#include "main_thread.h"
#include "thread_pool.h"
#include "global_config.h"
#include "url.h"

#include "templates.h"

using namespace std;

Templates_Set templates;

int main(int argc,char **argv)
{
	try{
		global_config.load(argc,argv);
		
		templates.load();
		
		Run_Application<Main_Thread>(argc,argv);		
		
		cout<<"Exiting\n";
	}
	catch(HTTP_Error &s) {
		cerr<<s.get()<<endl;
		return 1;
	}
	return 0;
}
