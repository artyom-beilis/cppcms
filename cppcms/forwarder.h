///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
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
	
	///
	/// \brief Class responsble for automaticall forwarding of HTTP/CGI requests to other hosts over SCGI.
	///
	/// This class allows you to strasfer connections transparently and efficiently for specific applications
	/// like single server responsible for Comet processing
	///
	class CPPCMS_API forwarder {
	public:
		/// \cond INTERNAL 
		forwarder();
		~forwarder();
		
		typedef std::pair<std::string,int> address_type;
		address_type check_forwading_rules(std::string const &h,std::string const &s,std::string const &p);
		address_type check_forwading_rules(char const *h,char const *s,char const *p);
		
		/// \endcond 

		///
		/// Add forwarding of request that match a mount_point \a p over SCGI \a ip and \a port.
		///
		void add_forwarding_rule(booster::shared_ptr<mount_point> p,std::string const &ip,int port);
		///
		/// Remove the forwarding request, you need to use smake pointer you used in add_forwarding_rule
		///
		void remove_forwarding_rule(booster::shared_ptr<mount_point> p);

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
	/// be used after this function call.
	///
	/// Please note: forwarding would not work for POST requests with multipart/form-data Content type as they
	/// are stored and managed differently.
	///
	void CPPCMS_API forward_connection(booster::shared_ptr<http::context> cont,std::string const &ip,int port);
}



#endif
