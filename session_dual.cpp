#define CPPCMS_SOURCE
#include "session_dual.h"
#include "session_interface.h"
#include "session_cookies.h"
#include "session_sid.h"

using namespace std;

namespace cppcms {
namespace sessions {

struct session_dual::data {};

session_dual::session_dual(std::auto_ptr<encryptor> enc,intrusive_ptr<session_server_storage> storage,size_t limit) :
	client_(new session_cookies(enc)),
	server_(new session_sid(storage)),
	data_size_limit_(limit)
{
}

void session_dual::save(session_interface &session,string const &data,time_t timeout,bool isnew,bool on_server)
{
	if(on_server || data.size() > data_size_limit_) {
		server_->save(session,data,timeout,isnew,true);
	}
	else {
		std::string cookie=session.get_session_cookie();
		if(!cookie.empty() && cookie[0]=='C') {
			server_->clear(session);
		}
		client_->save(session,data,timeout,isnew,false);
	}
}

bool session_dual::load(session_interface &session,string &data,time_t &timeout)
{
	std::string cookie = session.get_session_cookie();
	if(!cookie.empty() && cookie[0]=='C') {
		return server_->load(session,data,timeout);
	}
	else {
		return client_->load(session,data,timeout);
	}
}

void session_dual::clear(session_interface &session)
{
	std::string cookie = session.get_session_cookie();
	if(!cookie.empty() && cookie[0]=='C') {
		server_->clear(session);
	}
	else {
		client_->clear(session);
	}
}

} // sessions
} // cppcms
