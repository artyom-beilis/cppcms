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
#ifndef CPPCMS_BASE_CONTENT_H
#define CPPCMS_BASE_CONTENT_H

#include "defs.h"

namespace cppcms {
	///
	/// \brief This is a simple polymorphic class that every content for templates rendering should be derided from it.
	/// It does not carry much information with exception of RTTI that allows type-safe casting of user provided
	/// content instances to target content class that is used by specific template.
	///
	class CPPCMS_API base_content {
	public:
		virtual ~base_content() {};
	};

}


#endif
