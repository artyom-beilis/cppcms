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
