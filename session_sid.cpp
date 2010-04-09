///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "session_sid.h"
#include "md5.h"
#include "session_storage.h"
#include "session_interface.h"
#include <fstream>
#include "cppcms_error.h"
#include "urandom.h"
#include <stdio.h>
#include <time.h>

#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/thread.hpp>
#else // Internal Boost
#   include <cppcms_boost/thread.hpp>
    namespace boost = cppcms_boost;
#endif

#ifndef CPPCMS_WIN_NATIVE
#include <sys/time.h>
#else
#include <windows.h>
#endif

using namespace std;

namespace cppcms {
namespace sessions {
namespace impl {
	using namespace cppcms::impl;

	class sid_generator : public util::noncopyable {
	public:
		sid_generator()
		{
		}
		
		void check_hash_ptr()
		{
			if(!hash_ptr.get()) {
				for_hash hashed;
				hashed.session_counter=0;
				urandom_device urand;
				urand.generate(hashed.uid,16);
				urand.generate(hashed.uid2,16);
				hash_ptr.reset(new for_hash(hashed));
			}
		}

		std::string get()
		{
			check_hash_ptr();
			for_hash &hashed = *hash_ptr;
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
		struct for_hash {
			char uid[16];
			uint64_t session_counter;
			#ifndef CPPCMS_WIN_NATIVE
			struct timeval tv;
			#else
			uint64_t tv;
			#endif
			char uid2[16];
		};
	private:
		static boost::thread_specific_ptr<for_hash> hash_ptr;
	};
	
	boost::thread_specific_ptr<sid_generator::for_hash> sid_generator::hash_ptr;


} // namespace impl



struct session_sid::data {};

session_sid::session_sid(intrusive_ptr<session_storage> st) :
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
			impl::sid_generator generator;
			id=generator.get(); // if id not valid create new one
		}
	}
	else {
		impl::sid_generator generator;
		id=generator.get(); 
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
	if(!storage_->load(id,timeout,data))
		return false;
	if(time(0) > timeout) {
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
