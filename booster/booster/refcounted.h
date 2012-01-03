///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2008-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//                                                                             
#ifndef BOOSTER_REFCOUNTED_H
#define BOOSTER_REFCOUNTED_H

#include <booster/atomic_counter.h>

namespace booster {

	class refcounted;
	void intrusive_ptr_add_ref(refcounted *ptr);
	void intrusive_ptr_release(refcounted *ptr);

	///
	/// \brief This class is used as base class for reference counted
	/// objects that use intrusive_ptr. Deriving from this class
	/// allows simple way to manage reference counting for single object
	///
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

	///
	/// Increase reference count
	///	
	inline void intrusive_ptr_add_ref(refcounted *p)
	{
		++p->refs_;
	}
	///
	/// Decrease reference count, if it goes to 0, destroy the object
	///
	inline void intrusive_ptr_release(refcounted *p)
	{
		if(p && --p->refs_ == 0)
			delete p;
	}

}


#endif
