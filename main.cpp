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

All_Configuration config = { "localhost","root","root","cppcms" };


int main(int argc,char **argv)
{
	char *socket="/tmp/php-fastcgi.socket";

	try{
		//#define SS
		#ifdef SS
		
		FastCGI_ST<Main_Thread> app(socket);
		
		app.execute();

		cout<<"Finished\n";

		# else 
		
		int n=5,max=100;
		
		FastCGI_MT<Main_Thread> app(n,max,socket);
		 
		app.execute();
		
		long long const *stats=app.get_stats();

		int i;

		for(i=0;i<n;i++){
			cout<<"Thread "<<i<<" executed "<<stats[i]<< " times\n";
		}
		cout<<"Finished\n"<<endl;
		#endif

		
	}
	catch(HTTP_Error &s) {
		cerr<<s.get()<<endl;
	}
		
	return 0;
}
