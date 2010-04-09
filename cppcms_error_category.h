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
#ifndef CPPCMS_ERROR_CATEGORY_H
#define CPPCMS_ERROR_CATEGORY_H

#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/system/system_error.hpp>
#else // Internal Boost
#   include <cppcms_boost/system/system_error.hpp>
    namespace boost = cppcms_boost;
#endif

namespace cppcms {
	namespace impl {
		namespace errc {
			enum {
				ok,
    				protocol_violation
			};
		}
		class error_category : public boost::system::error_category {
		public:
			virtual char const *name() const;
			virtual std::string message(int cat) const;
		};

		extern const error_category cppcms_category;
	}
}



#endif
