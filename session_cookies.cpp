#include "config.h"

#include "session_interface.h"
#include "session_cookies.h"
#include "hmac_encryptor.h"
#include "worker_thread.h"
#include "manager.h"
#include "session_backend_factory.h"
#ifdef EN_ENCR_SESSIONS
#include "aes_encryptor.h"
#endif

#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/shared_ptr.hpp>
#else // Internal Boost
#   include <cppcms_boost/shared_ptr.hpp>
    namespace boost = cppcms_boost;
#endif

using namespace std;

namespace cppcms {

namespace {
struct builder {
	boost::shared_ptr<session_api> operator()(worker_thread &w)
	{
		return boost::shared_ptr<session_api>(new session_cookies(w));
	}
};
}

session_backend_factory session_cookies::factory()
{
	return builder();
}

session_cookies::session_cookies(worker_thread &w,auto_ptr<encryptor> enc) :
	worker(w),
	encr(enc)
{
}

session_cookies::session_cookies(worker_thread &w) :
	worker(w)
{
#ifdef EN_ENCR_SESSIONS
	string default_type="aes";
#else
	string default_type="hmac";
#endif
	string type=w.app.config.sval("session.cookies_encryptor",default_type);
	string key=w.app.config.sval("session.cookies_key");
	if(type=="hmac") {
		encr.reset(new hmac::cipher(key));
		return;
	}
#ifdef EN_ENCR_SESSIONS
	if(type=="aes") {
		encr.reset(new aes::cipher(key));
		return;
	}
#endif
	throw cppcms_error("Unknown encryptor "+type);
}

void session_cookies::save(session_interface *session,string const &data,time_t timeout,bool not_used)
{
	string cdata=encr->encrypt(data,timeout);
	session->set_session_cookie(cdata);
}

bool session_cookies::load(session_interface *session,string &data,time_t &timeout_out)
{
	string cdata=session->get_session_cookie();
	if(cdata.empty()) return false;
	time_t timeout;
	string tmp;
	if(!encr->decrypt(cdata,tmp,&timeout))
		return false;
	time_t now;
	time(&now);
	if(timeout < now)
		return false;
	data.swap(tmp);
	timeout_out=timeout;
	return true;
}

void session_cookies::clear(session_interface *session)
{
	session->clear_session_cookie();
}


};
