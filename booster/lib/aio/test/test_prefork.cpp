#include <booster/thread.h>
#include "test.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <errno.h>

namespace io = booster::aio;
namespace sys = booster::system;


int main()
{
	int pid1 = 0;
	int pid2 = 0;
	try {
		io::io_service srv;
		srv.enable_prefork();
		io::socket s1(srv);
		s1.set_process_shared();
		s1.open(io::pf_inet,io::sock_stream);
		s1.set_option(io::socket::reuse_address,true);
		io::endpoint ep("127.0.0.1",8080);
		s1.bind(ep);
		s1.listen(5);
		io::socket s2;
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		if(pid1!=0) {
			kill(pid1,SIGKILL);
			int r;
			wait(&r);
		}
		if(pid2!=0) {
			kill(pid2,SIGKILL);
			int r;
			wait(&r);
		}
		return 1;
	}
	if(pid1!=0 && pid2!=0) {
		int r1=0,r2=0;
		::wait(&r1);
		::wait(&r2);
		if(	WIFEXITED(r1) && WEXITSTATUS(r1)==0 
			&& WIFEXITED(r2) && WEXITSTATUS(r2)==0) 
		{
			std::cerr << "Ok" <<std::endl;
			return 0;
		}
		return 1;
	}
	return 0l
}
}
