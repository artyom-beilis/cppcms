//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE

#include <booster/config.h>

#ifndef BOOSTER_WIN32
#include <sys/socket.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif


#include <booster/aio/io_service.h>
#include <booster/aio/types.h>
#include <booster/aio/reactor.h>
#include <booster/aio/aio_category.h>
#include <booster/thread.h>
#include <booster/posix_time.h>
#include <vector>
#include <map>
#include <deque>
#include <list>
#include <algorithm>

#include "select_iterrupter.h"

#include <iostream>

#include <errno.h>
#include <assert.h>

#include "category.h"

namespace booster {
namespace aio {

typedef unique_lock<recursive_mutex> lock_guard;

#ifndef BOOSTER_WIN32

	///
	/// This class is a simple mapping between file descriptor and its assosiated object.
	///
	/// Under posix platforms where file descriptors are kept as small numbers it uses vector
	/// and under Windows it uses std::map for this purpose.
	///

	template<typename Cont>
	class socket_map {
	public:
		typedef Cont value_type;

		bool is_valid(native_type fd)
		{
			return 0 <= fd && fd <=65535;
		}

		Cont &operator[](native_type fd)
		{
			if(fd >= int(map_.size()))
				map_.resize(fd+1);

			return map_.at(fd);
		}
		void clear()
		{
			map_.clear();
		}
		void erase(native_type /*fd*/) {}
	private:
		std::vector<Cont> map_;
	};


#else

	template<typename Cont>
	class socket_map {
	public:
		typedef Cont value_type;

		bool is_valid(native_type fd)
		{
			return fd!=invalid_socket;
		}

		Cont &operator[](native_type fd)
		{
			return map_[fd];
		}
		void clear()
		{
			map_.clear();
		}
		void erase(native_type fd)
		{
			map_.erase(fd);
		}
	private:
		std::map<native_type,Cont> map_;
	};


#endif

class event_loop_impl {
public:
	
	struct completion_handler {
		booster::intrusive_ptr<booster::refcounted> h;
		booster::system::error_code e;
		size_t n;
		void (*type)(completion_handler *self);

		completion_handler() : 
			n(0),
			type(&completion_handler::op_none) 
		{
		}
		completion_handler(handler const &inh) :
			h(inh.get_pointer()),
			n(0),
			type(&completion_handler::op_handler)
		{
		}
		completion_handler(handler &inh) :
			h(inh.get_pointer().release(),false),
			n(0),
			type(&completion_handler::op_handler)
		{
		}
		completion_handler(event_handler const &inh,booster::system::error_code const &ine) : 
			h(inh.get_pointer()),
			e(ine),
			n(0),
			type(&completion_handler::op_event_handler) 
		{
		}
		completion_handler(event_handler &inh,booster::system::error_code const &ine) : 
			h(inh.get_pointer().release(),false),
			e(ine),
			n(0),
			type(&completion_handler::op_event_handler) 
		{
		}
		completion_handler(io_handler const &inh,booster::system::error_code const &ine,size_t inn) : 
			h(inh.get_pointer()),
			e(ine),
			n(inn),
			type(&completion_handler::op_io_handler) 
		{
		}
		completion_handler(io_handler &inh,booster::system::error_code const &ine,size_t inn) : 
			h(inh.get_pointer().release(),false),
			e(ine),
			n(inn),
			type(&completion_handler::op_io_handler) 
		{
		}

		void operator()() 
		{
			type(this);
		}
		static void op_none(completion_handler *)
		{
		}
		static void op_handler(completion_handler *self)
		{
			booster::refcounted &call = *self->h;
			static_cast<handler::callable_type &>(call)();
		}
		static void op_event_handler(completion_handler *self)
		{
			booster::refcounted &call = *self->h;
			static_cast<event_handler::callable_type &>(call)(self->e);
		}
		static void op_io_handler(completion_handler *self)
		{
			booster::refcounted &call = *self->h;
			static_cast<io_handler::callable_type &>(call)(self->e,self->n);
		}





		void swap(completion_handler &other) {
			h.swap(other.h);
			std::swap(e,other.e);
			std::swap(n,other.n);
			std::swap(type,other.type);
		}
	};
	
	void set_io_event(native_type fd,int event,event_handler const &h)
	{
		if(event != io_events::in && event !=io_events::out)
			throw booster::invalid_argument("Invalid argument to set_io_event");
		io_event_setter setter = { fd,event,h,this };
		set_event(setter);
	}

	void cancel_io_events(native_type fd)
	{
		if(fd==invalid_socket)
			return;
		io_event_canceler canceler = {fd,this};
		set_event(canceler);
	}

	void run(system::error_code &e)
	{
		try {
			run();
		}
		catch(system::system_error const &err) {
			e=err.code();
		}
	}
	void run()
	{
		std::vector<reactor::event> evs(128);
		while(run_one(&evs.front(),evs.size()))
			;
	}
	void reset()
	{
		dispatch_queue_.clear();
		map_.clear();
		stop_ = false;
		reactor_.reset();
		interrupter_.close();
	}
	
	void stop()
	{
		lock_guard l(data_mutex_);
		stop_ = true;
		if(polling_)
			wake();
	}
	event_loop_impl(int type) :
		reactor_type_(type),
		stop_(false),
		polling_(false)
	{
	}
	void post(handler const &h)
	{
		lock_guard l(data_mutex_);
		dispatch_queue_.push_back(completion_handler(h));
		if(polling_)
			wake();
	}
	void post(event_handler const &h,booster::system::error_code const &e)
	{
		lock_guard l(data_mutex_);
		dispatch_queue_.push_back(completion_handler(h,e));
		if(polling_)
			wake();
	}
	void post(io_handler const &h,booster::system::error_code const &e,size_t n)
	{
		lock_guard l(data_mutex_);
		dispatch_queue_.push_back(completion_handler(h,e,n));
		if(polling_)
			wake();
	}
	
	std::string name() 
	{
		lock_guard l(data_mutex_);
		if(reactor_.get())
			return reactor_->name();
		reactor tmp(reactor_type_);
		return tmp.name();
	}
	
	~event_loop_impl()
	{
	}
	
	int set_timer_event(ptime point,event_handler const &h)
	{
		lock_guard l(data_mutex_);
		
		std::pair<ptime,timer_event> ev;
		ev.first = point;
		ev.second.h = h;
		timer_events_type::iterator end=timer_events_.end();

		if(timer_events_index_.size() < 1000) {
			timer_events_index_.resize(1000,end);
		}

		int attempts = 0;

		for(;;) {
			int pos = rand(timer_events_index_.size());
			if(timer_events_index_[pos] != end) {
				attempts++;
				if(attempts < 10 || timer_events_index_.size() >= rand_max)
					continue;
				// this must be empty so stop looping
				pos = timer_events_index_.size();
				timer_events_index_.resize(timer_events_index_.size()*2,end);
			}
			ev.second.event_id = pos;
			timer_events_index_[pos] = timer_events_.insert(ev);
			break;
		}

		if(polling_ && timer_events_.begin()->first >= point)
			wake();
		return ev.second.event_id;
	}

	void cancel_timer_event(int event_id)
	{
		lock_guard l(data_mutex_);

		if(timer_events_index_.at(event_id)==timer_events_.end())
			return;

		timer_events_type::iterator evptr = timer_events_index_[event_id];
		
		completion_handler evdisp(evptr->second.h,system::error_code(aio_error::canceled,aio_error_cat));
		dispatch_queue_.push_back(evdisp);
		timer_events_.erase(evptr);
		timer_events_index_[event_id]=timer_events_.end();

		if(polling_)
			wake();

	}

private:

	struct io_data {
		int current_event;
		event_handler readable,writeable;
		io_data() : current_event(0) {}
	};

	//
	// The reactor itself so multiple threads
	// can dispatch events on same reactor
	//
	std::auto_ptr<reactor> reactor_;
	
	//
	// Rest of the data protected by the
	// data_mutex
	//
	recursive_mutex data_mutex_;
	
	int reactor_type_;
	impl::select_interrupter interrupter_;;
	//
	// Stop flag identifies that
	// the service should go down
	//
	bool stop_;

	// 
	// Marks that pollong in progress so we can't
	// just set parameters we need also to wake the 
	// polling loop
	//
	bool polling_;

	//
	// I/O - selectable events
	//
	socket_map<io_data> map_;
	// events dispatch queue
	std::deque<completion_handler> dispatch_queue_;

	void closesocket(native_type fd)
	{
#ifndef BOOSTER_WIN32
		::close(fd);
#else
		::closesocket(fd);
#endif
	}



	struct io_event_canceler;
	friend struct io_event_canceler;

	struct io_event_canceler {
		native_type fd;
		event_loop_impl *self_;
		bool cancelation_is_needed_with_data_mutex_locked()
		{
			if(!self_->dispatch_queue_.empty())
				return true;
			io_data &cont=self_->map_[fd];
			if(cont.current_event == 0 && !cont.readable && !cont.writeable) {
				self_->map_.erase(fd);
				return false;
			}
			return true;
		}
		void operator()() const
		{
			lock_guard l(self_->data_mutex_);
			
			io_data &cont=self_->map_[fd];
			cont.current_event = 0;
			system::error_code e;
			self_->reactor_->remove(fd,e);
			e = system::error_code(aio_error::canceled,aio_error_cat);
			// Maybe it is closed
			if(cont.readable)
				self_->dispatch_queue_.push_back(completion_handler(cont.readable,e));
			if(cont.writeable)
				self_->dispatch_queue_.push_back(completion_handler(cont.writeable,e));
			self_->map_.erase(fd);
		}
	};
	struct io_event_setter;
	friend struct io_event_setter;

	struct io_event_setter {
		native_type fd;

		int event;
		event_handler h;
		event_loop_impl *self_;
		void operator()()
		{
			lock_guard l(self_->data_mutex_);
			
			if(!self_->map_.is_valid(fd))
			{
				#ifdef BOOSTER_WIN32
				system::error_code e(WSAEBADF,syscat);
				#else
				system::error_code e(EBADF,syscat);
				#endif
				self_->dispatch_queue_.push_back(completion_handler(h,e));
				return;
			}

			int new_event = self_->map_[fd].current_event | event;
			system::error_code e;
			self_->reactor_->select(fd,new_event,e);
			if(!e) {
				self_->map_[fd].current_event = new_event;
				if(event == io_events::in)
					self_->map_[fd].readable = h;
				else
					self_->map_[fd].writeable = h;
			}
			else {
				self_->dispatch_queue_.push_back(completion_handler(h,e));
			}
		}
	};

	struct timer_event {
		int event_id;
		event_handler h;
	};

	//
	// Timer events
	//
	typedef std::multimap<ptime,timer_event> timer_events_type;
	typedef std::vector<timer_events_type::iterator> timer_events_index_type;

	//
	// The events semself
	//
	timer_events_type timer_events_;
	timer_events_index_type timer_events_index_;

	//
	// Random number generator
	//
	unsigned seed_;
	static const unsigned rand_max = 32768;

	unsigned rand(unsigned limit)
	{
		seed_ = seed_ * 1103515245 + 12345;
		unsigned rv = ((unsigned)(seed_/65536) % 32768);
		return rv * limit / rand_max;
	}

	void wake()
	{
		interrupter_.notify();
	}

	template<typename Functor>
	void set_event(Functor &f)
	{
		lock_guard l(data_mutex_);
		if(polling_ || !reactor_.get()) {
			dispatch_queue_.push_back(completion_handler(f));
			if(reactor_.get())
				wake();
		}
		else {
			f();
		}
	}
	void set_event(io_event_canceler &f)
	{
		lock_guard l(data_mutex_);
		if(!f.cancelation_is_needed_with_data_mutex_locked())
			return;
		if(polling_ || !reactor_.get()) {
			dispatch_queue_.push_back(completion_handler(f));
			if(reactor_.get())
				wake();
		}
		else {
			f();
		}
	}

	bool run_one(reactor::event *evs,size_t evs_size)
	{
		lock_guard l(data_mutex_);
		if(!reactor_.get()) {
			reactor_.reset(new reactor(reactor_type_));
		}
		if(interrupter_.open()) {
			reactor_->select(interrupter_.get_fd(),reactor::in);
		}

		int counter = dispatch_queue_.size();
		while(!stop_ && !dispatch_queue_.empty() && counter > 0) {
			completion_handler exec;
			exec.swap(dispatch_queue_.front());
			dispatch_queue_.pop_front();
			
			data_mutex_.unlock();
			try {
				
				exec();
			}
			catch(...) {
				data_mutex_.lock();
				throw;
			}
			data_mutex_.lock();
			counter --;
		}

		ptime now = ptime::now();

		while(!stop_ && !timer_events_.empty() && timer_events_.begin()->first <= now) {
			timer_events_type::iterator evptr = timer_events_.begin();
			timer_events_index_[evptr->second.event_id] = timer_events_.end();
			completion_handler disp(evptr->second.h,system::error_code());
			dispatch_queue_.push_back(disp);
			timer_events_.erase(evptr);
		}


		//
		// Restart -- dispatch timed-out timers, we also need to read now - once again
		//

		if(stop_)
			return false;

		ptime wait_time = dispatch_queue_.empty() ? ptime::hours(1) : ptime::zero;

		if(!timer_events_.empty()) {
			ptime diff = timer_events_.begin()->first - now;
			if(diff < wait_time)
				wait_time = diff;
			assert(wait_time >= ptime::zero);
		}

		
		int n = 0;

		{
			system::error_code poll_error;
			polling_ = true;
			try {
				data_mutex_.unlock();
				n = reactor_->poll(evs,evs_size,int(ptime::milliseconds(wait_time)),poll_error);
			}
			catch(...) {
				data_mutex_.lock();
				polling_ = false;
				throw;
			}
			data_mutex_.lock();
			polling_ = false;
		
			//
			// We may get EBADF, so if we do not handle it we may loop
			// forever. However, maybe there is a handler that handles this
			// in dipatch queue (for example close was executed).
			// So let's try again
			//
			// But if it empty - no handlers, abort.
			//
			if(poll_error && poll_error.value()!=EINTR && dispatch_queue_.empty()) {
				throw system::system_error(poll_error);
			}

		}

		if( n > int(evs_size) )
			n=evs_size;
		randomize_events(evs,n);
		for(int i=0;i<n && i<int(evs_size);i++) {
			
			if(evs[i].fd == interrupter_.get_fd()) {
				interrupter_.clean();
				continue;
			}
			
			using booster::system::error_code;

			io_data &cont = map_[evs[i].fd];
			
			int new_events = cont.current_event;

			error_code dispatch_error;

			if(evs[i].events & reactor::err) {
				dispatch_error = error_code(aio_error::select_failed,aio_error_cat);
				new_events = 0;
			}
			if(evs[i].events & reactor::in)
				new_events &= ~reactor::in;
			if(evs[i].events & reactor::out)
				new_events &= ~reactor::out;
			
			error_code select_error;
			reactor_->select(evs[i].fd,new_events,select_error);
			if(select_error) {
				new_events = 0;
				if(!dispatch_error)
					dispatch_error=select_error;
			}
			
			cont.current_event = new_events;

			if(cont.readable && (new_events & reactor::in) == 0) {
				dispatch_queue_.push_back(completion_handler(cont.readable,dispatch_error));
			}
			if(cont.writeable && (new_events & reactor::out) == 0) {
				dispatch_queue_.push_back(completion_handler(cont.writeable,dispatch_error));
			}
			
			if(new_events == 0)
				map_.erase(evs[i].fd);
		}

		if(stop_) {
			wake();
		}

		return true;

	}

	void randomize_events(reactor::event *evs,int n)
	{
		if(n < 2)
			return;
		for(int i=0;i<n;i++) {
			int new_pos = rand(n-i);
			std::swap(evs[i],evs[i+new_pos]);
		}
	}

}; 

// For future use
struct io_service::data{};

io_service::io_service(int type) : impl_(new event_loop_impl(type))
{
}

io_service::io_service() : impl_(new event_loop_impl(reactor::use_default))
{
}
io_service::~io_service()
{
}

void io_service::set_io_event(native_type fd,int event,event_handler const &h)
{
	impl_->set_io_event(fd,event,h);
}
void io_service::cancel_io_events(native_type fd)
{
	impl_->cancel_io_events(fd);
}

void io_service::run(system::error_code &e)
{
	impl_->run(e);
}

void io_service::run()
{
	impl_->run();
}
void io_service::reset()
{
	impl_->reset();
}
void io_service::stop()
{
	impl_->stop();
}
void io_service::post(handler const &h)
{
	impl_->post(h);
}

void io_service::post(event_handler const &h,booster::system::error_code const &e)
{
	impl_->post(h,e);
}

void io_service::post(io_handler const &h,booster::system::error_code const &e,size_t n)
{
	impl_->post(h,e,n);
}


int io_service::set_timer_event(ptime const &t,event_handler const &h)
{
	return impl_->set_timer_event(t,h);
}

void io_service::cancel_timer_event(int id)
{
	impl_->cancel_timer_event(id);
}

std::string io_service::reactor_name()
{
	return impl_->name();
}

} // aio
} // booster
