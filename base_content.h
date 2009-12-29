#ifndef CPPCMS_BASE_CONTENT_H
#define CPPCMS_BASE_CONTENT_H

#include "defs.h"

namespace cppcms {
	///
	/// \brief This is a simple polymorphic class that every content for templates rendering should be dervided from it.
	/// It does not carry much information with exception of RTTI that allows type-safe casting of user provided
	/// content instances to target content class that is used by specific template.
	///
	class CPPCMS_API base_content {
	public:
		virtual ~base_content() {};
	};

}


#endif
