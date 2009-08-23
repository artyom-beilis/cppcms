#ifndef CPPCMS_REFCOUNTED_H
#define CPPCMS_REFCOUNTED_H

#include "atomic_counter.h"

namespace cppcms {

	class refcounted;
	void intrusive_ptr_add_ref(refcounted *ptr);
	void intrusive_ptr_release(refcounted *ptr);

	class refcounted {
	public:
		refcounted() :
			refs_(0)
		{
		}

		virtual ~refcounted()
		{
		}

	private:
		friend void intrusive_ptr_add_ref(refcounted *);
		friend void intrusive_ptr_release(refcounted *);
	
		refcounted(refcounted const &other);
		refcounted const &operator=(refcounted const &other);
		atomic_counter refs_;
	};
	
	inline void intrusive_ptr_add_ref(refcounted *p)
	{
		++p->refs_;
	}
	inline void intrusive_ptr_release(refcounted *p)
	{
		if(p && --p->refs_ == 0)
			delete p;
	}

}


#endif
