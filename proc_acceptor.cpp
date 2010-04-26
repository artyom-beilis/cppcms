#define CPPCMS_SOURCE
#include "proc_acceptor.h"
namespace cppcms { namespace impl {

	process_shared_acceptor::process_shared_acceptor(cppcms::service &srv)
	{
		break_fd_=wake_fd_=-1;
		stop_=false;
		FD_ZERO(&wait_set);
		callbacks_.resize(FD_SETSIZE);
		assigners_.resize(FD_SETSIZE);
		service_=&srv.impl().get_io_service();
		srv.after_fork(boost::bind(&process_shared_acceptor::on_fork,this));
	}

	void process_shared_acceptor::on_fork()
	{
		int fds[2];
		pipe(fds);
		break_fd_=fds[0];
		wake_fd_=fds[1];
		max_1_=std::max(break_fd_,wake_fd_)+1;
		FD_SET(break_fd_,&wait_set);
	}
	void process_shared_acceptor::wake()
	{
		char c='A';
		for(;;) {
			if(::write(wake_fd_,&c,1) < 0 && errno==EINTR)
				continue;
			break;
		}
	}

	bool process_shared_acceptor::run_one()
	{
		fd_set rd;
		{
			thread_guard guard(thread_mutex_);
			rd=wait_set;
		}
		int n;
		{
			mutex::guard guard(process_mutex_);
			n=::select(max_1_,&rd,0,0,0);
			if(n < 0 && errno==EINTR)
				return true;
			if(n < 0)
				return false;
			if(FD_ISSET(break_fd_,&rd)) {
				char c;
				for(;;) {
					if(::read(break_fd_,&c,1) < 0 && errno==EINTR)
						continue;
					break;
				}
				if(stop_)
					return false;
				FD_CLR(break_fd_,&rd);
			}
			for(int afd=0;afd<max_1_;afd++) {
				if(FD_ISSET(afd,&rd)) {
					int fd=::accept(afd,0,0);
					int err=errno;

					thread_guard tguard(thread_mutex_);

					boost::system::error_code e;
					if(fd > 0) {
						assigners_[afd](fd);
						assigners_[afd]=assign_function();
					}
					else {
						e=boost::system::error_code(err,boost::system::errno_ecat);
					}
					service_->post(boost::bind(callbacks_[afd],e));
					callbacks_[afd]=on_accept_function();
					FD_CLR(afd,&wait_set);
				}
			}
		}
		return true;
	}
	void process_shared_acceptor::run()
	{
		while(run_one())
			;
	}

	process_shared_acceptor::~process_shared_acceptor()
	{
		if(break_fd_!=-1)
			::close(break_fd_);
		if(wake_fd_!=-1)
			::close(wake_fd_);
	}


}} // cppcms::impl

