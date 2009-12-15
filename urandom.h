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
