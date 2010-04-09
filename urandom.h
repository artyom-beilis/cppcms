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
#ifndef CPPCMS_URANDOM_H
#define CPPCMS_URANDOM_H

#include "defs.h"
#include "hold_ptr.h"
#include "noncopyable.h"

namespace cppcms {
	class CPPCMS_API urandom_device : public util::noncopyable {
	public:
		urandom_device();
		~urandom_device();
		
		void generate(void *,unsigned n);

	private:
		struct data;
		util::hold_ptr<data> d;
		

	};
}


#endif
