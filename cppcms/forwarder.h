#ifndef CPPCMS_FORWARDER_H
#define CPPCMS_FORWARDER_H

#include <cppcms/defs.h>
#include <booster/hold_ptr.h>
#include <booster/shared_ptr.h>
#include <booster/noncopyable.h>
#include <booster/thread.h>
#include <string>
#include <map>

namespace cppcms {
	class mount_point;
	namespace http {
		class context;
	}
	class CPPCMS_API forwarder {
	public:
		forwarder();
		~forwarder();
		void add_forwarding_rule(booster::shared_ptr<mount_point> p,std::string const &ip,int port);
		void remove_forwarding_rule(booster::shared_ptr<mount_point> p);

		typedef std::pair<std::string,int> address_type;
		address_type check_forwading_rules(std::string const &h,std::string const &s,std::string const &p);
	private:
		typedef std::map<booster::shared_ptr<mount_point> ,address_type> rules_type;
		rules_type rules_;
		booster::shared_mutex mutex_;
		struct _data;
		booster::hold_ptr<_data> d;
	};

	///
	/// Forward the connection handled by \a cont to other node at IP \a ip listenning to on port \a port over
	/// SCGI protocol. 
	///
	/// The context must be released first by calling cppcms::application::release_context() and it should not
	/// be used after this function call
	///
	void CPPCMS_API forward_connection(booster::shared_ptr<http::context> cont,std::string const &ip,int port);
}



#endif
