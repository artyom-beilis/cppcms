#ifndef CPPCMS_IMPL_PREFORK_ACCEPTOR_H
#define CPPCMS_IMPL_PREFORK_ACCEPTOR_H

#include <cppcms/service.h>
#include <booster/thread.h>
#include <booster/noncopyable.h>
#include <booster/aio/socket.h>
#include <booster/system_error.h>
#include <booster/log.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>

#include <algorithm>
#include <memory>

#include "cgi_api.h"

#include <cppcms_boost/bind.hpp>

namespace boost = cppcms_boost;

namespace cppcms {
namespace impl {


class prefork_acceptor : public booster::noncopyable {
public:
	prefork_acceptor(cppcms::service *srv) 
	{
		service_ = srv;
		stop_ = false;
		read_interrupter_=-1;
		write_interrupter_=-1;

	}
	void add_acceptor(booster::shared_ptr<cgi::acceptor> acc)
	{
		acceptors_.push_back(acc);
	}
	~prefork_acceptor()
	{
		if(thread_.get()) {
			if(!stop_)
				stop();
			thread_->join();
			thread_.reset();
		}
		if(read_interrupter_!=-1)
			::close(read_interrupter_);
		if(write_interrupter_!=-1)
			::close(write_interrupter_);
	}
	void start()
	{
		int fds[2];
		if(::pipe(fds) < 0) {
			service_->shutdown();
			throw booster::system::system_error(
				booster::system::error_code(errno,booster::system::system_category));
		}
		read_interrupter_=fds[0];
		write_interrupter_=fds[1];
		thread_.reset(new booster::thread(boost::bind(&prefork_acceptor::run,this)));
	}
private:
	void stop()
	{
		stop_ = true;
		int res = -1;
		do {
			res = ::write(write_interrupter_,"A",1);
		} while(res  < 0 && errno==EINTR);
	}
	void run()
	{
		typedef booster::shared_ptr< cppcms::http::context> context_ptr;
		fd_set lset;
		FD_ZERO(&lset);
		int max = -1;
		for(unsigned i=0;i<acceptors_.size();i++) {
			int native = acceptors_[i]->socket().native();
			if(native < 0) {
				throw cppcms_error("Invalid acceptor");
			}
			else if(native >= FD_SETSIZE) {
				throw cppcms_error("Socket descriptor is bigger then FD_SETSIZE");
			}
			FD_SET(native,&lset);
			max = std::max(native,max);
		}
		FD_SET(read_interrupter_,&lset);
		max = std::max(read_interrupter_,max);

		while(!stop_) {
			std::vector<context_ptr> connections_;
			std::vector<int> accepted_fds_(acceptors_.size(),-1);;
			try
			{ 	
				// Critical section
				{
					booster::unique_lock<booster::fork_shared_mutex> l(mutex_);

					fd_set r=lset;

					int n = ::select(max+1,&r,0,0,0);

					if(n < 0 && errno == EINTR)
						continue;
					if(n < 0) {
						booster::system::error_code e(errno,booster::system::system_category);
						BOOSTER_CRITICAL("cppcms") << "select failed:" << e.message();
						service_->shutdown();
						return;
					}

					if(FD_ISSET(read_interrupter_,&r)) {
						static char buf[32];
						::read(read_interrupter_,buf,32);
					}
					for(unsigned i=0;i<acceptors_.size();i++) {
						int fd = acceptors_[i]->socket().native();
						if(!FD_ISSET(fd,&r))
							continue;
						int new_fd = ::accept(fd,0,0);
						if(new_fd < 0) {
							if(	errno==EINTR 
								|| errno==EAGAIN 
								|| errno==EWOULDBLOCK 
								|| errno==ECONNABORTED
								|| errno==EPROTO)
							{
								continue;
							}
							booster::system::error_code e(errno,booster::system::system_category);
							if(e.value()==EMFILE || e.value()==ENFILE) {
								BOOSTER_ERROR("cppcms_prefork") 
									<< "Accept failed:" <<	e.message();
								continue;
							}
							else {
								BOOSTER_CRITICAL("cppcms_prefork") 
									<< "Accept failed:" << e.message();
								service_->shutdown();
								throw booster::system::system_error(e);
							}
						}
						else {
							accepted_fds_[i]=new_fd;
						}
					}
				} // End critical section
				for(unsigned i=0;i<accepted_fds_.size();i++) {
					if(accepted_fds_[i]!=-1) {
						int tmp = accepted_fds_[i];
						accepted_fds_[i]=-1;
						connections_.push_back(acceptors_[i]->accept(tmp));
					}
				}
			}
			catch(...) {
				for(unsigned i=0;i<accepted_fds_.size();i++) {
					if(accepted_fds_[i]!=-1) {
						::close(accepted_fds_[i]);
						accepted_fds_[i]=-1;
					}
				}
				throw;
			}

			for(unsigned i=0;i<connections_.size();i++) {
				service_->post(boost::bind(&cppcms::http::context::run,connections_[i]));
			}
			connections_.clear();
		} // while loop
	} // run  function
	
	friend struct runner;

	std::vector<booster::shared_ptr<cgi::acceptor> > acceptors_;
	int read_interrupter_,write_interrupter_;
	bool stop_;
	cppcms::service *service_;
	std::auto_ptr<booster::thread> thread_;
	booster::fork_shared_mutex mutex_;
};

} /// impl
} /// cppcms

#endif
