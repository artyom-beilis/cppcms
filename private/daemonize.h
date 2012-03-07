///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_DAEMONIZE_H
#define CPPCMS_IMPL_DAEMONIZE_H

#include <cppcms/defs.h>
#include <string>

namespace cppcms {
namespace json { class value; }
namespace impl {
	
class CPPCMS_API daemonizer {
public:
	daemonizer(json::value const &conf);
	~daemonizer();
	static int global_urandom_fd;
private:
	int real_pid;
	std::string unlink_file;
	
	void daemonize(json::value const &conf);
	void cleanup();
};

} 
}


#endif
