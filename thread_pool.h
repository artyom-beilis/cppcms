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
#ifndef CPPCMS_THREAD_POOL_H
#define CPPCMS_THREAD_POOL_H

#include "defs.h"
#include "noncopyable.h"
#include <booster/function.h>
#include <booster/hold_ptr.h>

namespace cppcms {

	namespace impl {
		class thread_pool;
	}

	class CPPCMS_API thread_pool : public util::noncopyable {
	public:

		int post(booster::function<void()> const &job);	
		bool cancel(int id);
		thread_pool(int threads);
		void stop();
		~thread_pool();

	private:

		booster::hold_ptr<impl::thread_pool> impl_;
	};


} // cppcms



#endif

