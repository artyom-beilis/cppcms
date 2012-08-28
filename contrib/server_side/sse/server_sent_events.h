//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef SERVER_SENT_EVENTS_H
#define SERVER_SENT_EVENTS_H

#include <booster/noncopyable.h>
#include <cppcms/http_context.h>
#include <booster/shared_ptr.h>
#include <booster/enable_shared_from_this.h>
#include <booster/intrusive_ptr.h>
#include <booster/aio/deadline_timer.h>
#include <booster/posix_time.h>
#include <set>
#include <map>
#include <vector>
#include <ostream>

namespace sse {

///
/// Basic function that allows to format a proper output for event id, data and event name
/// according to Server-Sent Events protocol
///
void write_event(std::ostream &output,
		 char const *id,
		 char const *data,
		 char const *event=0);

///
/// This class represent an output event stream
///
class event_stream {
public:
	explicit event_stream(booster::shared_ptr<cppcms::http::context> ctx);

	event_stream();
	event_stream(event_stream const &);
	event_stream &operator=(event_stream const &);
	~event_stream();

	bool operator<(event_stream const &other) const
	{
		return ctx_ < other.ctx_;
	}

	bool operator==(event_stream const &other) const
	{
		return ctx_ == other.ctx_;
	}
	
	bool operator!=(event_stream const &other) const
	{
		return ctx_ != other.ctx_;
	}

	///
	/// Get the last event id that was known to client, if not known (new connection)
	/// return false
	///
	char const *last_id();

	///
	/// Get last event id as integer, if not known returns 0
	///
	size_t last_integer_id();

	///
	/// Set last event id
	///
	void last_id(char const *id);
	///
	/// Set last event id as number
	///
	void last_integer_id(size_t id);
	
	///
	/// Write event to stream, note last_id() is updated automatically
	///
	void write(char const *data,char const *id,char const *event=0);
	///
	/// Write event to stream, note last_id() is updated automatically
	///
	void write(char const *data,size_t id,char const *event=0);

	///
	/// Write event to stream, note last_id() is updated automatically
	///
	void write(std::string const &data,std::string const &id,std::string const &event=std::string());
	///
	/// Write event to stream, note last_id() is updated automatically
	///
	void write(std::string const &data,size_t id,std::string const &event=std::string());

	///
	/// Get the context associated with the object
	///
	booster::shared_ptr<cppcms::http::context> context();

private:
	booster::shared_ptr<cppcms::http::context> ctx_;
	std::string last_id_;
};

namespace details {
	class post_send;
	class remove_poller;
	class remove_streamer;
	class keep_alive_updater;
};

///
/// This class represents an basic event source object that allows you handle multiple
/// connections simultaneously
///

class event_source : public booster::enable_shared_from_this<event_source> {
protected:
	///
	/// Create a new queue
	///
	/// Note you must always create it using booster::shared_ptr<new DrivedClassFromEventSource>
	///
	event_source(booster::aio::io_service &srv);
public:

	virtual ~event_source();

	///
	/// Set a header that would be used to detect that client uses XHR long polling rather
	/// than EventSource. The first parameter is CGI variable and the second the expected
	/// value, if the value is empty, any non-empty data coming with this header would be
	/// considered a long polling request.
	///
	/// For example for a header "X-Long-Polling: true" call polling_detector("HTTP_X_LONG_POLLING","true");
	///
	/// The default is: "HTTP_X_SSE_SIMULATION","long-polling" that requires HTTP Header "X-SSE-Simulation:long-polling"
	///
	void polling_detector(std::string const &cgi_variable,std::string const &value);

	///
	/// Accept new connection 
	///
	virtual void accept(booster::shared_ptr<cppcms::http::context> ctx);

	///
	/// Broadcast event to all connections waiting for event
	///
	/// this->send() would be called for all pending connections
	///
	virtual void broadcast();

	///
	/// Close all outstanding connections
	///
	/// If accept or broadcast is called after async_close is called, it would throw booster::logic_error error
	///
	virtual void close() ;

	///
	/// Get a number of connections waiting for new event to be broadcasted
	///
	size_t waiting();
	
	///
	/// Send keep alive notification to clients - basically send a single line comment
	/// to make sure that clients that do not work would be disconnected
	///
	virtual void keep_alive(char const *comment = "keep alive");
	
	///
	/// Send keep_alive notification to clients every \a idle seconds if no notifications were
	/// sent during this period. Disabled by default.
	///
	void enable_keep_alive(double idle = 30.0);

protected:
	///	
	/// This callback is called upon new connection without header Last-Event-ID
	///
	/// Return true if some data was sent to the stream
	///
	virtual bool on_connect(event_stream &ev) 
	{
		return on_sent(ev);
	}
	///
	/// This callback is called upon new connection with non-empty header Last-Event-ID
	/// 
	/// Return true if some data was sent to the stream
	///
	virtual bool on_reconnect(event_stream &ev)
	{
		return on_sent(ev);
	}
	
	///
	/// Send new events that got to stream. This function is called upon broadcast() call
	///
	/// Return true if new data was written, otherwise return false.
	///
	virtual bool send(event_stream &ev)
	{
		return on_sent(ev);
	}
	///
	/// After sending events to client is complete this function is called. You are expected
	/// to check if there some new events occurred using last_id() or last_integer_id() values
	/// and send them to client returning true, otherwise return false
	///
	/// Return true if some data was sent to the stream
	///
	///
	/// This is a minimal function that should be implemented for stream handing
	///
	virtual bool on_sent(event_stream &) = 0;


	///
	/// By default checks the HTTP header, you can override it to use your own method
	///
	virtual bool check_polling(booster::shared_ptr<cppcms::http::context> ctx);

private:
	friend class details::post_send;
	friend class details::remove_streamer;
	friend class details::remove_poller;
	friend class details::keep_alive_updater;


	typedef std::set<booster::intrusive_ptr<details::post_send> > streamers_type;
	typedef std::set<event_stream> long_pollers_type;

	streamers_type streamers_;
	long_pollers_type long_pollers_;

	std::string polling_env_var_;
	std::string polling_value_;
	bool closing_;
	booster::aio::deadline_timer timer_;
	booster::ptime last_broadcast_;
	booster::ptime idle_limit_;
};



///
/// State Stream a class that keeps the client updated about the latest events.
///
/// Consider stock prices. User needs to know the latest value, so if he connects
/// later or misses some of the values he receives all events or events he does 
/// not read yet.
///
class state_stream : public event_source {
protected:
	state_stream(booster::aio::io_service &srv) :
		event_source(srv)
	{
	}
public:
	static booster::shared_ptr<state_stream> create(booster::aio::io_service &srv)
	{
		booster::shared_ptr<state_stream> p(new state_stream(srv));
		return p;
	}

	///
	/// Update an event - default message. If \a send is false the messages are not dispatched
	/// immediately, you can dispatch them later by calling broadcast() or by updating an
	/// event with send=true
	///
	void update(std::string const &data,bool send=true)
	{
		update(std::string(),data,send);
	}
	///
	/// Update a named event. If \a send is false the messages are not dispatched
	/// immediately, you can dispatch them later by calling broadcast() or by updating an
	/// event with send=true
	///
	void update(std::string const &event,std::string const &data,bool send=true)
	{
		current_++;
		state &st = states_[event];
		st.updated = current_;
		st.data = data;
		if(send)
			broadcast();
	}
protected:

	bool on_sent(event_stream &es)
	{
		bool has_something = false;
		size_t last_id = es.last_integer_id();
		for(states_type::iterator p=states_.begin();p!=states_.end();++p) {
			if(p->second.updated > last_id) {
				if(p->first.empty()) 
					es.write(p->second.data,current_);
				else
					es.write(p->second.data,current_,p->first);
				has_something = true;
			}
		}
		return has_something;
	}

private:
	struct state {
		size_t updated;
		std::string data;
	};
	size_t current_;
	typedef std::map<std::string,state> states_type;
	states_type states_;
};


///
/// This is an event queue object. It keeps limited number of messages, such that if the
/// user hadn't received messages for a long time he would be able to receive only
/// the latest ones.
///
class bounded_event_queue : public event_source {
protected:
	bounded_event_queue(booster::aio::io_service &srv,size_t size,size_t limit = 256) :
		event_source(srv),
		start_(0),
		end_(0),
		capacity_(size),
		limit_(limit)
	{
		messages_.reserve(size);
	}
public:
	///
	/// Create a queue of maximal size \a size, such that user that connects too late
	/// it would be able to receive at most \a size latest messages
	///
	/// It is also possible to set the maximal number of messages send in one operation
	/// uning \a limit paeameter. See: \ref message_send_limit
	///
	static booster::shared_ptr<bounded_event_queue> create(	booster::aio::io_service &srv,
								size_t size,
								size_t limit=256)
	{
		booster::shared_ptr<bounded_event_queue> p(new bounded_event_queue(srv,size,limit));
		return p;
	}
	///
	/// message_send_limit defines a buffer limit for a single send operation such that
	/// when a user reads very long queue from the beginning he would not cause delays in the event loop.
	///
	void message_send_limit(size_t n)
	{
		limit_ = n;
	}

	///
	/// Enqueue a new event of default "message" type
	///
	/// If \a send is false the messages are not dispatched
	/// immediately, you can dispatch them later by calling broadcast() or by calling enqueue 
	/// event with send=true
	///
	void enqueue(std::string const &data,bool send=true)
	{
		enqueue(std::string(),data,send);
	}
	///
	/// Enqueue a new event of type \a event
	///
	/// If \a send is false the messages are not dispatched
	/// immediately, you can dispatch them later by calling broadcast() or by calling enqueue 
	/// event with send=true
	///
	void enqueue(std::string const &event,std::string const &data,bool send=true)
	{
		message *msg = 0;
		if(end_ >= capacity_) {
			msg = &messages_[end_ % capacity_];
			end_++;
			start_++;
		}
		else {
			messages_.push_back(message());
			msg = &messages_.back();
			end_++;
		}
		msg->event = event;
		msg->data = data;
		if(send)
			broadcast();
	}
protected:
	bool on_sent(event_stream &es)
	{
		size_t last_id = es.last_integer_id();
		if(last_id > end_) {
			es.last_integer_id(end_);
			return false;
		}
		if(last_id==end_)
			return false;
		if(last_id < start_) {
			last_id = start_;
		}

		for(size_t id = last_id + 1,counter=0;id<=end_ && counter<limit_;id++,counter++) {
			message &msg = messages_[ (id-1) % capacity_ ];
			es.write(msg.data,id,msg.event);
		}
		return true;
	}


private:

	struct message {
		std::string event;
		std::string data;
	};
	size_t start_;
	size_t end_;
	size_t capacity_;
	size_t limit_;
	std::vector<message> messages_;
};



} // namespace sse


#endif
