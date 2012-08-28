#include "server_sent_events.h"

#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include <booster/system_error.h>
#include <stdio.h>
#include <string.h>

namespace sse {

void write_event(std::ostream &out,
		 char const *id,
		 char const *data,
		 char const *event)
{
	out << "id:" << id <<"\n";
	if(event && *event)
		out<< "event:" << event <<"\n";
	out << "data:";
	for(char const *p=data;*p;) {
		char c=*p;
		if(c=='\r') {
			p++;
			continue;
		}
		else if(c=='\n') {
			p++;
			out << "\ndata:";
		}
		else {
			char const *e=p;
			while((c=*e)!=0 && c!='\r' && c!='\n')
				e++;
			out.write(p,e-p);
			p=e;
		}
	}
	out << "\n\n";
}

event_stream::event_stream() 
{
}

event_stream::event_stream(booster::shared_ptr<cppcms::http::context> ctx) : ctx_(ctx) 
{
}
				
event_stream::event_stream(event_stream const &other) : ctx_(other.ctx_),last_id_(other.last_id_)
{
}

event_stream &event_stream::operator=(event_stream const &other)
{
	ctx_ = other.ctx_;
	last_id_ = other.last_id_;
	return *this;
}


event_stream::~event_stream()
{
}

char const *event_stream::last_id()
{
	return last_id_.c_str();
}


size_t event_stream::last_integer_id()
{
	return atoi(last_id());
}

void event_stream::last_id(char const *id)
{
	last_id_ = id;
}

void event_stream::last_integer_id(size_t id)
{
	char buf[128];
	snprintf(buf,sizeof(buf),"%lld",static_cast<unsigned long long>(id));
	last_id(buf);
}


void event_stream::write(char const *data,char const *id,char const *event)
{
	write_event(ctx_->response().out(),id,data,event);
	last_id(id);
}

void event_stream::write(char const *data,size_t n,char const *event)
{
	char buf[32];
	snprintf(buf,sizeof(buf),"%ld",static_cast<unsigned long>(n));
	write(data,buf,event);
}

void event_stream::write(std::string const &data,std::string const &id,std::string const &event)
{
	if(event.empty())
		write(data.c_str(),id.c_str());
	else
		write(data.c_str(),id.c_str(),event.c_str());
}
void event_stream::write(std::string const &data,size_t id,std::string const &event)
{
	if(event.empty())
		write(data.c_str(),id);
	else
		write(data.c_str(),id,event.c_str());
}

booster::shared_ptr<cppcms::http::context> event_stream::context()
{
	return ctx_;
}



event_source::event_source(booster::aio::io_service &srv) : 
	polling_env_var_("HTTP_X_SSE_SIMULATION"),
	polling_value_("long-polling"),
	closing_(false),
	timer_(srv)
{
	last_broadcast_ = booster::ptime::now();
}

event_source::~event_source()
{
}

namespace details {

	typedef booster::intrusive_ptr<booster::callable<void(cppcms::http::context::completion_type status)> > comp_ptr;
	typedef booster::intrusive_ptr<booster::callable<void()> > disco_ptr;
	typedef booster::intrusive_ptr<booster::callable<void(booster::system::error_code const &)> > ka_ptr;

	class post_send : public booster::callable<void(cppcms::http::context::completion_type status)> {
	public:
		post_send(event_stream const &es,booster::weak_ptr<event_source> q) :
			stream_(es),
			queue_(q)
		{
		}
		
		event_stream &stream()
		{
			return stream_;
		}
		void operator()(cppcms::http::context::completion_type status)
		{
			if(status!=0)
				return;
			booster::shared_ptr<event_source> queue = queue_.lock();
			if(!queue)
				return;
			if(queue->closing_) {
				stream_.context()->async_complete_response();
				return;
			}
			if(queue->on_sent(stream_))
				stream_.context()->async_flush_output(comp_ptr(this));
			else {
				booster::intrusive_ptr<post_send> self(this);
				queue->streamers_.insert(self);
			}
		}
	private:
		event_stream stream_;
		booster::weak_ptr<event_source> queue_;
	};

	class remove_poller : public booster::callable<void()> {
	public:
		remove_poller(booster::shared_ptr<cppcms::http::context> ctx,booster::weak_ptr<event_source> q) :
			w_ctx_(ctx),
			queue_(q)
		{
		}
		virtual void operator()() 
		{
			booster::shared_ptr<event_source> self = queue_.lock();
			if(!self)
				return;
			booster::shared_ptr<cppcms::http::context> ctx = w_ctx_.lock();
			if(ctx) {
				event_stream es(ctx);
				self->long_pollers_.erase(es);
			}
		}
	private:
		booster::weak_ptr<cppcms::http::context> w_ctx_;
		booster::weak_ptr<event_source> queue_;
	};

	class remove_streamer : public booster::callable<void()> {
	public:
		remove_streamer(booster::intrusive_ptr<details::post_send> ev,booster::weak_ptr<event_source> q) :
			ev_(ev),
			queue_(q)
		{
		}
		virtual void operator()() 
		{
			booster::shared_ptr<event_source> self = queue_.lock();
			if(self) {
				self->streamers_.erase(ev_);
			}
		}
	private:
		booster::intrusive_ptr<details::post_send> ev_;
		booster::weak_ptr<event_source> queue_;
	};

	class keep_alive_updater : public booster::callable<void(booster::system::error_code const &e)> {
	public:
		keep_alive_updater(booster::shared_ptr<event_source> s) : queue_(s) 
		{
		}
		virtual void operator()(booster::system::error_code const &e)
		{
			if(e) 
				return;
			booster::ptime diff = booster::ptime::now() - queue_->last_broadcast_;
			if(diff >= queue_->idle_limit_) {
				queue_->keep_alive();
				queue_->timer_.expires_from_now(queue_->idle_limit_);
			}
			else {
				queue_->timer_.expires_from_now(queue_->idle_limit_ - diff);
			}
			queue_->timer_.async_wait(ka_ptr(this));
		}
	private:
		booster::shared_ptr<event_source> queue_;
	};

}

void event_source::enable_keep_alive(double idle)
{
	if(idle <= 0.001)
		throw booster::runtime_error("sse:enable_keep_alive the idle parameter is too small");
	booster::ptime limit = booster::ptime::from_number(idle);
	if(idle_limit_ == booster::ptime::zero) {
		idle_limit_ = limit;
		details::ka_ptr ptr(new details::keep_alive_updater(shared_from_this()));
		(*ptr)(booster::system::error_code()); // start it
	}
	else {
		idle_limit_ = limit; // just update
	}
}

void event_source::accept(booster::shared_ptr<cppcms::http::context> ctx)
{
	if(closing_)
		throw booster::logic_error("sse:accept is called after close operation started");
	event_stream es(ctx);
	
	ctx->response().content_type("text/event-stream"); // no need to specify UTF-8
	ctx->response().cache_control("no-cache");

	bool got_something = false;
	char const *env = ctx->request().cgetenv("HTTP_LAST_EVENT_ID");
	if(*env == 0) {
		got_something = on_connect(es);
	}
	else {
		es.last_id(env);
		got_something = on_reconnect(es);
	}
	bool polling = check_polling(ctx);
	if(polling) {
		if(got_something) 
			ctx->async_complete_response();
		else {
			ctx->async_on_peer_reset(details::disco_ptr(new details::remove_poller(ctx,shared_from_this())));
			long_pollers_.insert(es);
		}
	}
	else {
		booster::intrusive_ptr<details::post_send> ps = new details::post_send(es,shared_from_this());
		ctx->async_on_peer_reset(details::disco_ptr(new details::remove_streamer(ps,shared_from_this())));
		if(got_something) {
			ctx->async_flush_output(ps);
		}
		else {
			streamers_.insert(ps);
		}
	}
}

bool event_source::check_polling(booster::shared_ptr<cppcms::http::context> ctx)
{
	char const *val = ctx->request().cgetenv(polling_env_var_.c_str());
	if(val[0]==0) // undefined
		return false;
	if(polling_value_.empty()) // accept any value
		return true;
	return val == polling_value_;
}

void event_source::close()
{
	closing_ = true;
	for(streamers_type::iterator p=streamers_.begin();p!=streamers_.end();++p) {
		booster::intrusive_ptr<details::post_send> ptr = *p;
		ptr->stream().context()->async_complete_response();
	}
	streamers_.clear();

	for(long_pollers_type::iterator p=long_pollers_.begin();p!=long_pollers_.end();++p) {
		event_stream es = *p;
		es.context()->async_complete_response();
	}
	long_pollers_.clear();

	if(idle_limit_!=booster::ptime::zero)
		timer_.cancel();
}

size_t event_source::waiting()
{
	return long_pollers_.size() + streamers_.size();
}

void event_source::keep_alive(char const *comment)
{
	if(closing_) {
		return;
	}
	for(streamers_type::iterator it=streamers_.begin();it!=streamers_.end();) {
		booster::intrusive_ptr<details::post_send> ps = *it;
		streamers_type::iterator tmp = it++;
		streamers_.erase(tmp);

		ps->stream().context()->response().out() << ':' << comment << "\n\n";
		ps->stream().context()->async_flush_output(ps);
	}

	for(long_pollers_type::iterator it=long_pollers_.begin();it!=long_pollers_.end();) {
		event_stream stream = *it;
		long_pollers_type::iterator tmp = it++;
		long_pollers_.erase(tmp);
		stream.context()->response().out() << ':' << comment << "\n\n";
		stream.context()->async_complete_response();
	}
	
	last_broadcast_ = booster::ptime::now();
}


void event_source::broadcast()
{
	if(closing_) {
		throw booster::logic_error("sse:can't call broadcast after close is called");
	}
	for(streamers_type::iterator it=streamers_.begin();it!=streamers_.end();) {
		booster::intrusive_ptr<details::post_send> ps = *it;
		if(!send(ps->stream())) {
			++it;
		}
		else {
			streamers_type::iterator tmp = it++;
			streamers_.erase(tmp);
			ps->stream().context()->async_flush_output(ps);
		}
	}

	for(long_pollers_type::iterator it=long_pollers_.begin();it!=long_pollers_.end();) {
		event_stream stream = *it;
		if(!send(stream)) {
			++it;
		}
		else {
			long_pollers_type::iterator tmp = it++;
			long_pollers_.erase(tmp);
			stream.context()->async_complete_response();
		}
	}
	if(long_pollers_.empty() && streamers_.empty())
		last_broadcast_ = booster::ptime::now();
}

} // sse
