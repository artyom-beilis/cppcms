#ifndef CPPCMS_BASE_VIEW_H
#define CPPCMS_BASE_VIEW_H

#include "defs.h"

#include <ostream>
#include <sstream>
#include <string>
#include <map>
#include <ctime>
#include <memory>

#include "hold_ptr.h"
#include "base_content.h"
#include "noncopyable.h"

namespace cppcms {

class CPPCMS_API base_view : util::noncopyable {
public:
	virtual void render();
	virtual ~base_view();
	

protected:

	base_view(std::ostream &out);
	std::ostream &out();

private:
	struct data;
	util::hold_ptr<data> d;

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
