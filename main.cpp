#include <boost/scoped_array.hpp>
#include <pthread.h>
#include <sstream>


#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"
#include "fcgio.h"

#include "main_thread.h"
#include "thread_pool.h"

#include "global_config.h"


using namespace std;
using namespace boost;


int main(int argc,char **argv)
{
	try{
		global_config.load(argc,argv);
		
		int n,max;
		
		char const *socket=global_config.sval("server.socket","").c_str();
		
		if((n=global_config.lval("server.threads",0))==0) {		
			FastCGI_ST<Main_Thread> app(socket);
			app.execute();
		}
		else {
			max=global_config.lval("server.buffer",1);
			FastCGI_MT<Main_Thread> app(n,max,socket);
			app.execute();
			long long const *stats=app.get_stats();

			int i;

			for(i=0;i<n;i++){
				cout<<"Thread "<<i<<" executed "<<stats[i]<< " times\n";
			}
		}
		cout<<"Finished\n"<<endl;


		
	}
	catch(HTTP_Error &s) {
		cerr<<s.get()<<endl;
	}
		
	return 0;
}
