#include "aio_test.h"
#include <booster/aio/reactor.h>
#include <booster/system_error.h>
#include <booster/posix_time.h>

using namespace booster::aio;

int main()
{
	try {
		for(int type=reactor::use_default;type<=reactor::use_max;type++) {
			reactor r(type);

			std::cout << "Testing " << type << ": " << r.name() << std::endl;

			booster::system::error_code e;
			r.select(booster::aio::invalid_socket,booster::aio::reactor::in,e);
			TEST(e);

			native_type fds[2];
			pair(fds);
			native_type a=fds[0];
			native_type b=fds[1];
			r.select(a,reactor::in);
			r.select(b,reactor::in);
			reactor::event evs[3];
			TEST(r.poll(evs,3,0)==0);
			r.select(a,reactor::in | reactor::out);
			r.select(b,reactor::in | reactor::out);
			TEST(r.poll(evs,3,0)==2);
			TEST((evs[0].fd==a && evs[1].fd==b) || (evs[0].fd==b && evs[1].fd==a));
			TEST(evs[0].events==reactor::out);
			TEST(evs[1].events==reactor::out);
			r.remove(a);
			TEST(r.poll(evs,3,0)==1);
			TEST(evs[0].fd==b);
			TEST(evs[0].events==reactor::out);
			char c='x';
			send(a,&c,1,0);
			r.select(b,reactor::in);
			TEST(r.poll(evs,3,1000)==1);
			TEST(evs[0].fd==b);
			TEST(evs[0].events==reactor::in);
			r.select(b,reactor::in | reactor::out);
			TEST(r.poll(evs,3,0)==1);
			TEST(evs[0].fd==b);
			TEST(evs[0].events==(reactor::out | reactor::in));
			recv(b,&c,1,0);
			TEST(r.poll(evs,3,0)==1);
			TEST(evs[0].fd==b);
			TEST(evs[0].events==reactor::out);
			#ifdef SHUT_RDWR
			shutdown(a,SHUT_RDWR);
			#else
			shutdown(a,SD_BOTH);
			#endif
			close(a);
			booster::ptime::millisleep(100);
			TEST(r.poll(evs,3,0)==1);
			TEST(evs[0].fd==b);
			TEST(evs[0].events==(reactor::out | reactor::in));
			int res;
			TEST((res=recv(b,&c,1,0))==0);
			close(b);
		}
	}
	catch(std::exception const &e) {
		std::cerr << "Error: "<< e.what() << std::endl;
		return 1;
	}
	if(return_code == 0)
		std::cout << "Ok" <<std::endl;
	else
		std::cerr << "Fail " << std::endl;
	return return_code;
}

