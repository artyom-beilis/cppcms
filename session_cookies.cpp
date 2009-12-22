#define CPPCMS_SOURCE
#include "session_cookies.h"
#include "cppcms_error.h"
#include "session_interface.h"

#include <time.h>

namespace cppcms {
namespace sessions {

using namespace std;

struct session_cookies::data {};

session_cookies::session_cookies(std::auto_ptr<encryptor> enc) :
	encryptor_(enc)
{
}
session_cookies::~session_cookies()
{
}

void session_cookies::save(session_interface &session,string const &data,time_t timeout,bool not_used,bool on_server)
{
	if(on_server)
		throw cppcms_error("Can't use cookies backend when data should be stored on server");
	string cdata="C" + encryptor_->encrypt(data,timeout);
	session.set_session_cookie(cdata);
}

bool session_cookies::load(session_interface &session,string &data,time_t &timeout_out)
{
	string cdata=session.get_session_cookie();
	if(cdata.empty()) return false;
	if(cdata[0]!='C') {
		session.clear_session_cookie();
		return false;
	}
	time_t timeout;
	string tmp;
	if(!encryptor_->decrypt(cdata.substr(1),tmp,&timeout))
		return false;
	if(timeout < time(0))
		return false;
	data.swap(tmp);
	timeout_out=timeout;
	return true;
}

void session_cookies::clear(session_interface &session)
{
	session.clear_session_cookie();
}


} // sessions
} // cppcms
