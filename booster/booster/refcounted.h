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
#ifndef BOOSTER_REFCOUNTED_H
#define BOOSTER_REFCOUNTED_H

#include <booster/atomic_counter.h>

namespace booster {

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
