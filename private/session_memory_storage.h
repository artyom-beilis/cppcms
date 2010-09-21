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
#ifndef CPPCMS_SESSIOM_MEMORY_STORAGE_H
#define CPPCMS_SESSIOM_MEMORY_STORAGE_H

#include <cppcms/defs.h>
#include <cppcms/session_storage.h>
namespace cppcms {
namespace sessions {

	class CPPCMS_API session_memory_storage_factory : public session_storage_factory {
	public:
		session_memory_storage_factory();
		virtual booster::shared_ptr<session_storage> get();
		virtual bool requires_gc();
		virtual void gc_job();
		virtual ~session_memory_storage_factory();
	private:
		booster::shared_ptr<session_storage> storage_;
	};

} // sessions
} // cppcms 
#endif
