#include <boost/scoped_array.hpp>
#include <pthread.h>
#include <sstream>
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"
#include "fcgio.h"

#include "main_thread.h"
#include "thread_pool.h"


using namespace std;
using namespace boost;

int  global_counter = 0;

All_Configuration config = { "localhost","root","root","cppcms" };


int main(int argc,char **argv)
{
	char *socket="/tmp/php-fastcgi.socket";

	try{
		#define SS
		
		#ifdef SS
		FastCGI_ST<Main_Thread> app(socket);
		# else 
		FastCGI_MT<Main_Thread> app(5,socket);
		 
		#endif
	
		app.execute();
		
		#ifndef SS
		long long const *stats=app.get_stats();
		int i;
		for(i=0;i<5;i++){
			cout<<"Thread "<<i<<" executed "<<stats[i]<< " times\n";
		}
		#endif

		cout<<"Finished\n"<<global_counter<<endl;
		
	}
	catch(HTTP_Error &s) {
		cerr<<s.get()<<endl;
	}
		
	return 0;
}
