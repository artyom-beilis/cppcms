#ifndef CPPCMS_SESSION_SID_H
#define CPPCMS_SESSION_SID_H

#include "session_api.h"
#include "defs.h"
#include "hold_ptr.h"
#include "intrusive_ptr.h"
#include "session_storage.h"

namespace cppcms {
namespace sessions {

	namespace impl { class sid_generator; }
	
	class CPPCMS_API session_sid : public session_api {
	public:
		session_sid(intrusive_ptr<session_storage> s);
		~session_sid();
		virtual void save(session_interface &,std::string const &data,time_t timeout,bool,bool);
		virtual bool load(session_interface &,std::string &data,time_t &timeout);
		virtual void clear(session_interface &);
	private:

		bool valid_sid(std::string const &cookie,std::string &id);
		std::string key(std::string sid);
		
		struct data;
		util::hold_ptr<data> d;
		util::hold_ptr<impl::sid_generator> sid_;
		intrusive_ptr<session_storage> storage_;
	};

} // sessions
} // cppcms


#endif
