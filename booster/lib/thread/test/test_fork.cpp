//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/thread.h>
#include "test.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <errno.h>

int dowrite(int fd)
{
	char c='A';
	int r;
	while((r=::write(fd,&c,1))==-1 && errno==EINTR)
		;
	return r;
}


int doread(int fd)
{
	char c;
	int r;
	while((r=::read(fd,&c,1))==-1 && errno==EINTR)
		;
	return r;
}

int main()
{
	booster::fork_shared_mutex mutex_;
	int fd[2];
	int res =  pipe(fd);
	(void)(res);
	int pid = fork();
	try {
		if(pid!=0) { // parent

			std::cout << "Test try_lock" << std::endl;
			close(fd[0]);
			usleep(100000);
			dowrite(fd[1]);
			usleep(100000);
			TEST(!mutex_.try_lock());
			dowrite(fd[1]);
			usleep(100000);
			TEST(mutex_.try_lock());
			mutex_.unlock();
			std::cout << "Test shared/unique" << std::endl;
			mutex_.shared_lock();
			dowrite(fd[1]);
			usleep(100000);
			close(fd[1]);
			mutex_.unlock();
			std::cout << "Test unlock on exit" << std::endl;
			usleep(100000);
			TEST(!mutex_.try_lock());
			usleep(500000);
			TEST(mutex_.try_lock());
		}
		else { // child
			close(fd[1]);
			TEST(doread(fd[0])==1);
			TEST(mutex_.try_lock());
			TEST(doread(fd[0])==1);
			mutex_.unlock();
			// test 2
			TEST(doread(fd[0])==1);
			TEST(mutex_.try_shared_lock());
			mutex_.unlock();
			TEST(!mutex_.try_unique_lock());
			mutex_.unique_lock();
			TEST(doread(fd[0])==0);
			usleep(200000);
			return 0;
		}
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		if(pid!=0) {
			int res;
			::wait(&res);
		}
		return 1;
	}
	if(pid!=0) {
		int res;
		::wait(&res);
		if(WIFEXITED(res) && WEXITSTATUS(res)==0) {
			std::cerr << "Ok" <<std::endl;
			return 0;
		}
		return 1;
	}
	return 0;
}
