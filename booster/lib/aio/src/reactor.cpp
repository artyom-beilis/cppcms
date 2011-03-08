//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE

#include "reactor_config.h"

#ifdef AIO_HAVE_POSIX_SELECT
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#endif

#if defined AIO_HAVE_WIN32_SELECT || defined AIO_HAVE_WSAPOLL
#undef FD_SETSIZE
#define FD_SETSIZE 1024
#include <winsock2.h>
#include <map>
#include <algorithm>
#endif

#ifdef AIO_HAVE_POLL
#include <poll.h>
#endif
#ifdef AIO_HAVE_DEVPOLL
#include <sys/devpoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef AIO_HAVE_EPOLL
#include <sys/epoll.h>
#endif

#ifdef AIO_HAVE_KQUEUE
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif

#include <booster/aio/reactor.h>
#include <booster/system_error.h>

#include <vector>
#include <iostream>

#include "category.h"

namespace booster {
namespace aio {
	class reactor_impl {
	public:
		virtual void select(native_type fd,int flags,int &error) = 0;
		virtual int poll(reactor::event *events,int n,int timeout,int &error) = 0;
		virtual std::string name() const = 0;
		virtual ~reactor_impl() {};
	};


	#if defined AIO_HAVE_POLL  || defined AIO_HAVE_DEVPOLL
	class base_poll_reactor : public reactor_impl {
	protected:
		int to_poll_events(int event)
		{
			int pe=0;
			if(event & reactor::in)
				pe|=POLLIN;
			if(event & reactor::out)
				pe|=POLLOUT;
			return pe;
		}
		int to_user_events(int event)
		{
			int ue=0;
			if(event & POLLIN)
				ue|=reactor::in;
			if(event & POLLOUT)
				ue|=reactor::out;
			if(event & (POLLERR | POLLHUP | POLLPRI))
				ue|=reactor::err;
			return ue;
		}
	};
	#endif

	#ifdef AIO_HAVE_POLL

	class poll_reactor : public base_poll_reactor {
	public:
		std::string name() const { return "poll"; }
		virtual void select(native_type fd,int flags,int &error)
		{
			if(!check(fd,error))
				return;
			if(flags == 0) {
				remove(fd);
				return;
			}
			entry(fd).events=to_poll_events(flags);
		}

		struct pollfd &entry(native_type fd)
		{
			if(map_.size() <= unsigned(fd)) {
				map_.resize(fd+1,-1);
			}
			if(map_[fd]==-1) {
				map_[fd]=pollfds_.size();
				pollfd tmp=pollfd();
				tmp.fd=fd;
				pollfds_.push_back(tmp);
			}
			return pollfds_[map_[fd]];
			
		}
		
		void remove(native_type fd)
		{
			if(fd >= int(map_.size()) || map_[fd]==-1)
				return;

			int index=map_[fd];
			std::swap(pollfds_[index],pollfds_.back());
			map_[pollfds_[index].fd]=index;
			pollfds_.resize(pollfds_.size()-1);
			map_[fd]=-1;
		}
		
		
		virtual int poll(reactor::event *events,int n,int timeout,int &error)
		{
			struct pollfd *pfd = pollfds_.size() > 0 ? &pollfds_.front() : 0;
			int size = pollfds_.size();

			int count = ::poll(pfd,size,timeout);
			if(count < 0)
				error = errno;
			int read=0;
			for(unsigned i=0;i<pollfds_.size() && read < count && read < n;i++) {
				if(pollfds_[i].revents == POLLNVAL) {
					remove(pollfds_[i].fd);
					count --;
					continue;
				}
				if(pollfds_[i].revents != 0) {
					events[read].events = to_user_events(pollfds_[i].revents);
					events[read].fd = pollfds_[i].fd; 
					read ++;
				}
			}
			return read;
		}
	private:
		bool check(native_type fd,int &error)
		{
			if(fd < 0 || fd >65536) {
				error=EBADF;
				return false;
			}
			return true;
		}
		std::vector<int> map_;
		std::vector< ::pollfd> pollfds_;
	};

	#endif // poll
	
	#if defined(AIO_HAVE_DEVPOLL) || defined(AIO_HAVE_EPOLL) || defined(AIO_HAVE_KQUEUE)
	class base_fast_reactor {
	protected:
		bool check(int fd,int &error)
		{
			if(fd < 0 || fd >65536) {
				error=EINVAL;
				return false;
			}
			if(fd >= int(events_.size())) {
				events_.resize(fd+1,0);
			}
			return true;
		}

		std::vector<int> events_;
	};
	
	#endif

	#ifdef AIO_HAVE_DEVPOLL

	class devpoll_reactor : public base_poll_reactor, protected base_fast_reactor {
	public:
		std::string name() const { return "devpoll"; }

		devpoll_reactor() 
		{
			pollfd_=::open("/dev/poll",O_RDWR);
			if(!pollfd_)
				throw system::system_error(system::error_code(errno,syscat));
		}
	
		
		virtual ~devpoll_reactor()
		{
			::close(pollfd_);
		}
		virtual void select(native_type fd,int flags,int &error)
		{
			if(!check(fd,error))
				return;
			struct pollfd evs[2] = {0};
			int evs_count = 0;
			if(flags == events_[fd])
				return;

			if(events_[fd]) {
				evs[evs_count].fd=fd;
				evs[evs_count].events=POLLREMOVE;
				evs_count++;
			}
			if(flags!=0) {
				evs[evs_count].fd=fd;
				evs[evs_count].events=to_poll_events(flags);
				evs_count++;
			}
			if(evs_count > 0) {
				int size = sizeof(evs[0])*evs_count;
				if(::write(pollfd_,evs,size)!=size) {
					error=errno;
					return;
				}
			}
			events_[fd]=flags;
		}
		
		virtual int poll(reactor::event *events,int n,int timeout,int &error)
		{
			struct pollfd fds[128] = {{0}};
			struct dvpoll dvp={0};
			dvp.dp_timeout = timeout;
			dvp.dp_nfds = n > 128 ? 128 : n;
			dvp.dp_fds=fds;

			int size = ::ioctl(pollfd_,DP_POLL,&dvp);
			
			if(size < 0) {
				error = errno;
				return -1;
			}
			int read = 0;
			for(int i=0;i<size;i++) {
				events[read].events = to_user_events(fds[i].revents);
				events[read].fd = fds[i].fd; 
				read ++;
			}
			return read;
		}
	private:

		int pollfd_;
	};
	
	#endif // /dev/poll

	#ifdef AIO_HAVE_EPOLL
	class epoll_reactor : public base_poll_reactor, public base_fast_reactor {
	public:
		std::string name() const { return "epoll"; }

		epoll_reactor() 
		{
			pollfd_=::epoll_create(16);
			if(!pollfd_)
				throw system::system_error(system::error_code(errno,syscat));
		}
		virtual ~epoll_reactor()
		{
			::close(pollfd_);
		}
		virtual void select(native_type fd,int flags,int &error)
		{
			if(!check(fd,error))
				return;
			if(events_[fd]!=0 && flags==0)
				write_flag(fd,EPOLL_CTL_DEL,0,error);
			else if(events_[fd]==0 && flags!=0)
				write_flag(fd,EPOLL_CTL_ADD,to_poll_events(flags),error);
			else if(events_[fd]!=flags)
				write_flag(fd,EPOLL_CTL_MOD,to_poll_events(flags),error);
			events_[fd]=flags;

		}
		
		virtual int poll(reactor::event *events,int n,int timeout,int &error)
		{
			struct epoll_event fds[128] = {};
			if (n >128) n=128;

			int size = 0;
			size = ::epoll_wait(pollfd_,fds,n,timeout);
			
			if(size < 0) {
				error = errno;
				return -1;
			}
			int read = 0;
			for(int i=0;i<size;i++) {
				events[read].events = to_user_events(fds[i].events);
				events[read].fd = fds[i].data.fd; 
				read ++;
			}
			return read;
		}
	private:
		int to_poll_events(int event)
		{
			int pe=0;
			if(event & reactor::in)
				pe|=EPOLLIN;
			if(event & reactor::out)
				pe|=EPOLLOUT;
			return pe;
		}

		int to_user_events(int event)
		{
			int ue=0;
			if(event & EPOLLIN)
				ue|=reactor::in;
			if(event & EPOLLOUT)
				ue|=reactor::out;
			if(event & (EPOLLERR | EPOLLPRI | EPOLLHUP))
				ue|=reactor::err;
			return ue;
		}

		void write_flag(int fd,int op,int flags,int &error)
		{
			struct epoll_event efd=epoll_event();
			efd.events=flags;
			efd.data.fd=fd;
			
			if(::epoll_ctl(pollfd_,op,fd,&efd) < 0) {
				error=errno;
				return;
			}
		}
		int pollfd_;
	};
	#endif // epoll

	#if defined AIO_HAVE_POSIX_SELECT
	class select_reactor : public reactor_impl {
	public:
		std::string name() const { return "select"; }
		select_reactor() :
			map_(FD_SETSIZE,-1)
		{
		}
		virtual void select(native_type fd,int flags,int &error)
		{
			if(!check(fd,error))
				return;
			if(flags == 0) {
				remove(fd);
				return;
			}
			entry(fd).events=flags;
		}
		
		void remove(native_type fd)
		{
			if(fd >= int(map_.size()) || map_[fd]==-1)
				return;
			int index=map_[fd];
			std::swap(pollfds_[index],pollfds_.back());
			map_[pollfds_[index].fd]=index;
			pollfds_.resize(pollfds_.size()-1);
			map_[fd]=-1;
		}
		virtual int poll(reactor::event *events,int n,int timeout,int &error)
		{
			fd_set rd,wr,er;
			FD_ZERO(&rd);
			FD_ZERO(&wr);
			FD_ZERO(&er);
			int max_fd=0;
			for(unsigned i=0;i<pollfds_.size();i++) {
				int flags=pollfds_[i].events;
				int fd=pollfds_[i].fd;
				if(flags & reactor::in)
					FD_SET(fd,&rd);
				if(flags & reactor::out)
					FD_SET(fd,&wr);
				FD_SET(fd,&er);
				if(fd +1> max_fd)
					max_fd = fd+1;
			}
			struct timeval tv,*ptv=0;
			if(timeout >=0) {
				tv.tv_sec = timeout / 1000;
				tv.tv_usec = (timeout % 1000)*1000;
				ptv=&tv;
			}
			int read=::select(max_fd,&rd,&wr,&er,ptv);
			if(read < 0) {
				error=errno;
				return -1;
			}

			int count = 0;

			for(unsigned i=0;read > 0 && i < pollfds_.size() ;i++) {
				int fd=pollfds_[i].fd;
				bool r = FD_ISSET(fd,&rd);
				read-=int(r);
				bool w = FD_ISSET(fd,&wr);
				read-=int(w);
				bool e = FD_ISSET(fd,&er);
				read-=int(e);
				if(r || w || e) {
					if(count >=n) {
						count ++;
						continue;
					}
					reactor::event &ev=events[count];
					ev.fd = fd;
					ev.events=0;
					if(r)
						ev.events |= reactor::in;
					if(w)
						ev.events |= reactor::out;
					if(e) 
						ev.events |= reactor::err;
					count++;
				}
			}
			return count;
		}
	private:
		bool check_exists(native_type fd,int &error)
		{
			if(!check(fd,error))
				return false;
			if(map_[fd]==-1) {
				error = ENOENT;
				return false;
			}
			return true;
		}
		bool check(native_type fd,int &error)
		{
			if(fd < 0 || fd>=native_type(FD_SETSIZE)) {
				error=EBADF;
				return false;
			}
			return true;

		}
		struct ev {
			int fd;
			int events;
		};

		ev &entry(native_type fd)
		{
			if(map_.size() <= unsigned(fd)) {
				map_.resize(fd+1,-1);
			}
			if(map_[fd]==-1) {
				map_[fd]=pollfds_.size();
				ev tmp=ev();
				tmp.fd=fd;
				pollfds_.push_back(tmp);
				return pollfds_.back();
			}
			return pollfds_[map_[fd]];
			
		}


		std::vector<int> map_;
		std::vector<ev> pollfds_;
	};
	#endif // POSIX_SELECT

	#ifdef AIO_HAVE_KQUEUE
	class kqueue_reactor : public reactor_impl, protected base_fast_reactor  {
	public:
		std::string name() const { return "kqueue"; }
		kqueue_reactor() :
			kev_(::kqueue())
		{
			if(kev_ < 0)
				throw system::system_error(system::error_code(errno,syscat));
		}
		void select(int fd,int new_flags,int &error)
		{
			if(!check(fd,error))
				return;
			
			struct kevent evs[2];
			int evs_count = 0;

			if((events_[fd] & reactor::in) && ! (new_flags & reactor::in)) {
				EV_SET(&evs[evs_count],fd,EVFILT_READ,EV_DELETE,0,0,0);
				evs_count++;
			}
			if((events_[fd] & reactor::out) && ! (new_flags & reactor::out)) {
				EV_SET(&evs[evs_count],fd,EVFILT_WRITE,EV_DELETE,0,0,0);
				evs_count++;
			}
			if(!(events_[fd] & reactor::in) && (new_flags & reactor::in)) {
				EV_SET(&evs[evs_count],fd,EVFILT_READ,EV_ADD,0,0,0);
				evs_count++;
			}
			if(!(events_[fd] & reactor::out) && (new_flags & reactor::out)) {
				EV_SET(&evs[evs_count],fd,EVFILT_WRITE,EV_ADD,0,0,0);
				evs_count++;
			}
			if(evs_count > 0) {
				if(::kevent(kev_,evs,evs_count,0,0,0) < 0) {
					error=errno;
					return;
				}
			}
			events_[fd]=new_flags;
		}
		
		virtual int poll(reactor::event *events,int n,int timeout,int &error)
		{
			struct kevent evs[128];
			struct timespec tv,*ptv=0;
			if(timeout >=0) {
				tv.tv_sec = timeout / 1000;
				tv.tv_nsec = (timeout % 1000)*1000000L;
				ptv=&tv;
			}
			int read=::kevent(kev_,0,0,evs,128,ptv);
			if(read < 0) {
				error=errno;
				return -1;
			}
			int count = 0;
			for(int i=0;i<read;i++) {
				if(evs[i].flags & EV_EOF)
					add(evs[i].ident,events,n,count,reactor::in);
				if(evs[i].flags & EV_ERROR)
					add(evs[i].ident,events,n,count,reactor::err);
				if(evs[i].filter == EVFILT_READ)
					add(evs[i].ident,events,n,count,reactor::in);
				if(evs[i].filter == EVFILT_WRITE)
					add(evs[i].ident,events,n,count,reactor::out);
			}
			
			return count;
		}
		virtual ~kqueue_reactor() { ::close(kev_); };
	private:
		void add(int fd,reactor::event *events,int n,int &count,int flags)
		{
			for(int i=0;i<count;i++) {
				if(events[i].fd==fd) {
					events[i].events|=flags;
					return;
				}
			}
			if(count < n) {
				events[count].fd=fd;
				events[count].events=flags;
				count++;
			}
		}
		
		int kev_;
	};
	#endif
	

	#if defined AIO_HAVE_WIN32_SELECT
	class w32_select_reactor : public reactor_impl {
	public:
		std::string name() const { return "select"; }
		w32_select_reactor()
		{
		}
		virtual void select(native_type fd,int flags,int &error)
		{
			if(fd==invalid_socket) {
				error=WSAEBADF;
				return;
			}
			if(flags==0)
				pollfds_.erase(fd);
			else
				pollfds_[fd]=flags;
		}
		
		void set(native_type fd,fd_set *s)
		{
			s->fd_array[s->fd_count++]=fd;
		}
		
		bool isset(native_type fd,fd_set *s)
		{
			return std::binary_search(s->fd_array,s->fd_array+s->fd_count,fd);
		}

		virtual int poll(reactor::event *events,int n,int timeout,int &error)
		{
			fd_set rd,wr,er;
			FD_ZERO(&rd);
			FD_ZERO(&wr);
			FD_ZERO(&er);
			int count=0;
			for(pollfds_type::iterator p=pollfds_.begin(),e=pollfds_.end();p!=e;++p) {
				int flags=p->second;
				native_type fd=p->first;
				if(flags==0)
					continue;
				count ++;
				if(count >= FD_SETSIZE) {
					error = WSAEINVAL;
					return -1;
				}
				if(flags & reactor::in)
					set(fd,&rd);
				if(flags & reactor::out)
					set(fd,&wr);
				set(fd,&er);
			}
			struct timeval tv,*ptv=0;
			if(timeout >=0) {
				tv.tv_sec = timeout / 1000;
				tv.tv_usec = (timeout % 1000)*1000;
				ptv=&tv;
			}

			int read=::select(0,&rd,&wr,&er,ptv);
			if(read < 0) {
				error=WSAGetLastError();
				return -1;
			}
			std::sort(rd.fd_array,rd.fd_array+rd.fd_count);
			std::sort(wr.fd_array,wr.fd_array+wr.fd_count);
			std::sort(er.fd_array,er.fd_array+er.fd_count);

			count = 0;

			for(pollfds_type::iterator p=pollfds_.begin(),end=pollfds_.end();read > 0 && p!=end;++p) {
				int fd=p->first;
				bool r = isset(fd,&rd);
				read-=int(r);
				bool w = isset(fd,&wr);
				read-=int(w);
				bool e = isset(fd,&er);
				read-=int(e);
				if(r || w || e) {
					if(count >=n) {
						count ++;
						continue;
					}
					reactor::event &ev=events[count];
					ev.fd = fd;
					ev.events=0;
					if(r)
						ev.events |= reactor::in;
					if(w)
						ev.events |= reactor::out;
					if(e) 
						ev.events |= reactor::err;
					count++;
				}
			}
			return count;
		}
	private:
		typedef std::map<native_type,int> pollfds_type;
		pollfds_type pollfds_;
	};
	#endif // W32_SELECT

	#ifdef BOOSTER_WIN32
	namespace  {
		struct loader {
			loader() {
				WORD ver=MAKEWORD(2, 2);
				WSADATA data;
				if(WSAStartup(ver,&data) < 0) {
					throw booster::runtime_error("Failed to init winsock");
				}
			}
		} instance;
	}
	#endif



	reactor::reactor(int hint)
	{
		static const int default_poll = 
		#if defined AIO_HAVE_EPOLL
			use_epoll;
		#elif defined AIO_HAVE_DEVPOLL
			use_dev_poll;
		#elif defined AIO_HAVE_KQUEUE
			use_kqueue;
		#elif defined AIO_HAVE_POLL
			use_poll;
		#else
			use_select;
		#endif

		if(hint==use_default)
			hint=default_poll;

		for(;;) {
			switch(hint) {
			// select
			#if defined AIO_HAVE_POSIX_SELECT
			case use_select:
				impl_.reset(new select_reactor());
				return;
			#elif defined AIO_HAVE_WIN32_SELECT
			case use_select:
				impl_.reset(new w32_select_reactor());
				return;
			#endif
			// poll
			#if defined AIO_HAVE_POLL
			case use_poll:
				impl_.reset(new poll_reactor());
				return;
			#endif
			// epoll
			#if defined AIO_HAVE_EPOLL
			case use_epoll:
				impl_.reset(new epoll_reactor());
				return;
			#endif
			// devpoll
			#if defined AIO_HAVE_DEVPOLL
			case use_dev_poll:
				impl_.reset(new devpoll_reactor());
				return;
			#endif
			// kqueue
			#if defined AIO_HAVE_KQUEUE
			case use_kqueue:
				impl_.reset(new kqueue_reactor());
				return;
			#endif
			default:
				if(hint!=default_poll)
					hint=default_poll;
				else
					throw booster::runtime_error("Internal error - no poller found");
			}
		}

	}
	reactor::~reactor() {}

	std::string reactor::name() const
	{
		return impl_->name();
	}
	
	void reactor::select(native_type fd,int flags)
	{
		system::error_code e;
		select(fd,flags,e);
		if(e) throw system::system_error(e);
	}
	
	void reactor::select(native_type fd,int flags,system::error_code &e)
	{
		int error = 0;
		impl_->select(fd,flags,error);
		if(error)
			e=system::error_code(error,syscat);
	}

	int reactor::poll(reactor::event *events,int n,int timeout) 
	{
		system::error_code e;
		int r=poll(events,n,timeout,e);
		if(e)
			throw system::system_error(e);
		return r;
	}
	
	int reactor::poll(reactor::event *events,int n,int timeout,system::error_code &e) 
	{
		int error = 0;
		int r=impl_->poll(events,n,timeout,error);
		if(error)
			e=system::error_code(error,syscat);
		return r;
	}
	

	
} // aio
} // booster
