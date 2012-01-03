///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_TCP_CONNECTOR_H
#define CPPCMS_TCP_CONNECTOR_H

#include <string>
#include <vector>
#include <booster/noncopyable.h>

namespace cppcms {

namespace impl {
class messenger;
struct tcp_operation_header;

class tcp_connector : private booster::noncopyable
{
public:
	messenger &get(std::string const &key);

	tcp_connector(std::vector<std::string> const &ip_list,std::vector<int> const &port_list);
	virtual ~tcp_connector();
	
	void broadcast(tcp_operation_header &h,std::string &data);

protected:
	messenger *tcp;
	int conns;
	virtual unsigned hash(std::string const &key);
};

} // impl
} // cppcms


#endif
