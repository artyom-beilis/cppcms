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
#ifndef CPPCMS_ERROR_H
#define CPPCMS_ERROR_H

#include "defs.h"
#include <string>
#include <stdexcept>

namespace cppcms {

///
/// \brief Exception thrown by CppCMS framework.
///
/// Every exception that is thrown from CppCMS modules derived from this exception.
///

class CPPCMS_API cppcms_error : public std::runtime_error {
	std::string strerror(int err);
public:
	///
	/// Create an object with error code err (errno) and a message \a error
	///
	cppcms_error(int err,std::string const &error);
	///
	/// Create an object with message \a error
	///
	cppcms_error(std::string const &error) : std::runtime_error(error) {};
};

}
#endif /* _HTTP_ERROR_H */
