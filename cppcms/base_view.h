///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
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

	/// \cond INTERNAL

	base_view(std::ostream &out);
	std::ostream &out();

	/// \endcond

private:
	struct _data;
	booster::hold_ptr<_data> d;

};

} // cppcms


#if defined __clang__
#	if __has_feature(cxx_auto_type)
#		define CPPCMS_HAVE_AUTO_TYPE
#	endif
#elif defined __GNUC__
#	if (__GNUC__ >= 4 || (__GNUC__ == 4 &&  __GNUC_MINOR__ >= 4)) && defined(__GXX_EXPERIMENTAL_CXX0X__)
#		define CPPCMS_HAVE_AUTO_TYPE
#	endif
#elif defined _MSC_VER
#	if _MSC_VER >= 1600
#		define CPPCMS_HAVE_AUTO_TYPE
#	endif
#elif defined __INTEL_COMPILER
#	if __INTEL_COMPILER >= 1200 && defined(__GXX_EXPERIMENTAL_CXX0X__)
#		define CPPCMS_HAVE_AUTO_TYPE
#	endif
#elif defined CPPCMS_HAVE_CPP_0X_AUTO // detected at compilation stage
#	define CPPCMS_HAVE_AUTO_TYPE
#endif


#if defined(CPPCMS_HAVE_AUTO_TYPE)
#	define CPPCMS_TYPEOF(x) auto
#elif defined(CPPCMS_HAVE_CPP_0X_DECLTYPE)
#	define CPPCMS_TYPEOF(x) decltype(x)
#elif defined(CPPCMS_HAVE_GCC_TYPEOF)
#	define CPPCMS_TYPEOF(x) typeof(x)
#elif defined(CPPCMS_HAVE_UNDERSCORE_TYPEOF)
#	define CPPCMS_TYPEOF(x) __typeof__(x)
#else
#	define CPPCMS_TYPEOF(x) automatic_type_identification_is_not_supported_by_this_compiler
#endif


#endif
