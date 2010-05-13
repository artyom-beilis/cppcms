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
#include "cppcms_error_category.h"

namespace cppcms {
namespace impl {
	char const *error_category::name() const
	{
		return "cppcms::io";
	}
	std::string error_category::message(int cat) const
	{
		switch(cat) {
		case errc::ok: return "ok";
		case errc::protocol_violation: return "protocol violation";
		default:
			return "unknown";
		}
	}
	const error_category cppcms_category;

} // impl
} // cppcms
