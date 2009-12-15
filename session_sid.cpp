#define CPPCMS_SOURCE
#include "session_sid.h"
#include "md5.h"
#include "session_storage.h"
#include "session_interface.h"
#include <fstream>
#include "cppcms_error.h"
#include "urandom.h"

#ifndef CPPCMS_WIN_NATIVE
#include <sys/time.h>
#include <time.h>
#else
#include <windows.h>
#endif

using namespace std;

namespace cppcms {
namespace sessions {
namespace impl {
	using namespace cppcms::impl;

	class sid_generator : public util::noncopyable {
		struct for_hash {
			char uid[16];
			uint64_t session_counter;
			#ifndef CPPCMS_WIN_NATIVE
			struct timeval tv;
			#else
			uint64_t tv;
			#endif
			char uid2[16];
		} hashed;
	public:
		sid_generator()
		{
			hashed.session_counter=0;
			urandom_device urand;
			urand.generate(hashed.uid,16);
			urand.generate(hashed.uid2,16);
		}
		std::string get()
		{
			hashed.session_counter++;
			#ifndef CPPCMS_WIN_NATIVE
			gettimeofday(&hashed.tv,NULL);
			#else
			hashed.tv = GetTickCount();
			#endif
			
			md5_byte_t md5[16];
			char res[33];
			md5_state_t st;
			md5_init(&st);
			md5_append(&st,(md5_byte_t*)&hashed,sizeof(hashed));
			md5_finish(&st,md5);

			for(int i=0;i<16;i++) {
			#ifdef HAVE_SNPRINTF
				snprintf(res+i*2,3,"%02x",md5[i]);
			#else
				sprintf(res+i*2,"%02x",md5[i]);
			#endif
			}
			return std::string(res);
		}
	};
} // namespace impl

struct session_sid::data {};

session_sid::session_sid(intrusive_ptr<session_storage> st) :
	sid_(new impl::sid_generator()),
	storage_(st)
{
}

session_sid::~session_sid()
{
}


bool session_sid::valid_sid(std::string const &cookie,std::string &id)
{
	if(cookie.size()!=33 || cookie[0]!='I')
		return false;
	for(int i=1;i<33;i++) {
		char c=cookie[i];
		bool is_low_x_digit=('0'<=c && c<='9') || ('a'<=c && c<='f');
		if(!is_low_x_digit)
			return false;
	}
	id=cookie.substr(1,32);
	return true;
}

void session_sid::save(session_interface &session,std::string const &data,time_t timeout,bool new_data,bool unused)
{
	string id;
	if(!new_data) {
		if(!valid_sid(session.get_session_cookie(),id)) {
			id=sid_->get(); // if id not valid create new one
		}
	}
	else {
		id=sid_->get();
	}

	storage_->save(id,timeout,data);
	session.set_session_cookie("I"+id); // Renew cookie or set new one
}

bool session_sid::load(session_interface &session,std::string &data,time_t &timeout)
{
	string id;
	if(!valid_sid(session.get_session_cookie(),id))
		return false;
	std::string tmp_data;
	time_t tmp_timeout;
	if(!storage_->load(id,timeout,data))
		return false;
	if(time(0) > tmp_timeout) {
		storage_->remove(id);
		return false;
	}
	return true;
}

void session_sid::clear(session_interface &session)
{
	string id;
	if(valid_sid(session.get_session_cookie(),id))
		storage_->remove(id);
}

} // sessions
} // namespace cppcms
