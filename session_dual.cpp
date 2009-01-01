#include "session_dual.h"
#include "session_interface.h"

using namespace std;

namespace cppcms {

void session_dual::save(session_interface *session,string const &data,time_t timeout,bool isnew)
{
	if(data.size() > limit) {
		server->save(session,data,timeout,isnew);
	}
	else {
		if(session->get_session_cookie().size() == 32) {
			server->clear(session);
		}
		client->save(session,data,timeout,isnew);
	}
}

bool session_dual::load(session_interface *session,string &data,time_t &timeout)
{
	if(session->get_session_cookie().size()==32) {
		return server->load(session,data,timeout);
	}
	else {
		return client->load(session,data,timeout);
	}
}

void session_dual::clear(session_interface *session)
{
	if(session->get_session_cookie().size()==32) {
		server->clear(session);
	}
	else {
		client->clear(session);
	}
}

namespace {
struct builder {
	session_backend_factory client,server;
	size_t limit;
	builder(session_backend_factory c,session_backend_factory s,size_t l) :
		client(c),
		server(s),
		limit(l)
	{
	}
	boost::shared_ptr<session_api> operator()(worker_thread &w)
	{
		boost::shared_ptr<session_api> c,s;
		c=client(w);
		s=server(w);
		return boost::shared_ptr<session_api>(new session_dual(c,s,limit));
	}
};
}

session_backend_factory session_dual::factory(session_backend_factory c,session_backend_factory s,size_t l)
{
	return builder(c,s,l);
}
} // cppcms
