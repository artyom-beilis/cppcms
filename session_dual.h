#ifndef CPPCMS_SESSION_DUAL_H
#define CPPCMS_SESSION_DUAL_H

#include "session_api.h"
#include "session_backend_factory.h"
#include <boost/shared_ptr.hpp>

namespace cppcms {

class session_dual : public session_api {
	boost::shared_ptr<session_api> 	client;
	boost::shared_ptr<session_api>  server;
	size_t limit;
public:
	static session_backend_factory factory(session_backend_factory c,session_backend_factory s,size_t l);
	session_dual(boost::shared_ptr<session_api> c,boost::shared_ptr<session_api> s,size_t l) :
		client(c),
		server(s),
		limit(l)
	{
	}
	virtual void save(session_interface *,std::string const &data,time_t timeout,bool new_session);
	virtual bool load(session_interface *,std::string &data,time_t &timeout);
	virtual void clear(session_interface *);

};

}


#endif
