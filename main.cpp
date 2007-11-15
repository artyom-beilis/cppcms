#include <iostream>
#include "main_thread.h"
#include "thread_pool.h"
#include "global_config.h"


using namespace std;

int main(int argc,char **argv)
{
	try{
		Run_Application<Main_Thread>(argc,argv);		
		cout<<"Exiting\n";
	}
	catch(HTTP_Error &s) {
		cerr<<s.get()<<endl;
		return 1;
	}
	return 0;
}
