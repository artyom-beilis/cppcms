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
