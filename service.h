#ifndef CPPCMS_SERVICE_H
#define CPPCMS_SERVICE_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"

namespace cppcms {
	namespace impl {
		class service_impl;
	}

	class CPPCMS_API service : public util::noncopyable
	{
	public:
		cppcms::impl::service_impl &impl();
		~service();
		service();
	private:
		util::hold_ptr<impl::service_impl> impl_;
	};

} //




#endif
