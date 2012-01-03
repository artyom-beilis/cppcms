///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_PRIVATE_SESSION_TCP_STORAGE_H
#define CPPCMS_PRIVATE_SESSION_TCP_STORAGE_H

#include <cppcms/session_storage.h>
#include <booster/thread.h>
#include "tcp_connector.h"

namespace cppcms {
namespace sessions {

class CPPCMS_API tcp_storage : public session_storage {
public:
	tcp_storage(std::vector<std::string> const &ips,std::vector<int> const &ports) :
		ips_(ips),
		ports_(ports)
	{
	}
	virtual void save(std::string const &sid,time_t timeout,std::string const &in);
	virtual bool load(std::string const &sid,time_t &timeout,std::string &out);
	virtual void remove(std::string const &sid);
	virtual bool is_blocking();
	cppcms::impl::tcp_connector &tcp();
private:
	booster::thread_specific_ptr<cppcms::impl::tcp_connector> tcp_;
	std::vector<std::string> ips_;
	std::vector<int> ports_;
};

class CPPCMS_API tcp_factory : public session_storage_factory {
public:
	tcp_factory(std::vector<std::string> const &ips,std::vector<int> const &ports) :
		storage_(new tcp_storage(ips,ports))
	{
	}
	virtual booster::shared_ptr<session_storage> get() 
	{
		return storage_;
	}

	virtual bool requires_gc() 
	{
		return false;
	}
	virtual ~tcp_factory() {}
private:
	booster::shared_ptr<session_storage> storage_;
};

} // sessions
} // cppcms
#endif
