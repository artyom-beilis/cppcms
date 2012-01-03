///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_PRIVATE_SESSION_SQLITE_H
#define CPPCMS_PRIVATE_SESSION_SQLITE_H

#include <cppcms/defs.h>
#include <string>

namespace cppcms {
namespace sessions {
class session_storage_factory;
namespace sqlite_session {

CPPCMS_API session_storage_factory *factory(std::string const &database,std::string const &shared_object);
CPPCMS_API session_storage_factory *factory(std::string const &database);


}// sqlite_session
}// sessions
}// cppcms

#endif
