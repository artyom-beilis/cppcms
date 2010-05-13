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
#ifndef CPPCMS_BASE_VIEW_H
#define CPPCMS_BASE_VIEW_H

#include <cppcms/defs.h>

#include <ostream>
#include <sstream>
#include <string>
#include <map>
#include <ctime>
#include <memory>

#include <booster/hold_ptr.h>
#include <cppcms/base_content.h>
#include <booster/noncopyable.h>
#include <cppcms/config.h>

namespace cppcms {

///
/// \brief This class is base class for all views (skins) rendered by CppCMS template engine.
/// 
/// Users are not expected to derive from this class or use it directly. CppCMS template compiler
/// create skins that are usable with template engine and each template is derived from the \a base_view
/// class.
///

class CPPCMS_API base_view : booster::noncopyable {
public:
	///
	/// The main rendering function -- render the main HTML page. It is usually overridden in template engine.
	///
	virtual void render();
	virtual ~base_view();
	

protected:

	base_view(std::ostream &out);
	std::ostream &out();

private:
	struct data;
	booster::hold_ptr<data> d;

};

} // cppcms


#if defined(HAVE_CPP_0X_AUTO)
#	define CPPCMS_TYPEOF(x) auto
#elif defined(HAVE_CPP_0X_DECLTYPE)
#	define CPPCMS_TYPEOF(x) decltype(x)
#elif defined(HAVE_GCC_TYPEOF)
#	define CPPCMS_TYPEOF(x) typeof(x)
#elif defined(HAVE_UNDERSCORE_TYPEOF)
#	define CPPCMS_TYPEOF(x) __typeof__(x)
#else
#	define CPPCMS_TYPEOF(x) automatic_type_identification_is_not_supported_by_this_compiler
#endif


#endif
