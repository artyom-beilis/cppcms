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
#ifndef CPPCMS_UTIL_SIGNAL0_H
#define CPPCMS_UTIL_SIGNAL0_H

#include "callback0.h"
#include <list>


namespace cppcms { namespace util {
	class signal0 {
		typedef std::list<callback0> callbacks_type;
		callbacks_type signals_;
	public:
		void connect(callback0 h)
		{
			signals_.push_back(callback0());
			signals_.back().swap(h);
		}
		void operator()() const
		{
			for(callbacks_type::const_iterator p=signals_.begin();p!=signals_.end();++p) {
				(*p)();
			}
		}
	};

}} // cppcms::util

#endif
