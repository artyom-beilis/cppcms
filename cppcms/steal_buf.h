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
#ifndef CPPCMS_STEAL_BUF_H
#define CPPCMS_STEAL_BUF_H

#include <streambuf>
#include <ostream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>

namespace cppcms {
namespace util {

	template<size_t on_stack_size = 128>
	class steal_buffer : public std::streambuf {
		steal_buffer(steal_buffer const &);
		void operator=(steal_buffer const &);
	public:
		char *begin()
		{
			return pbase();
		}
		char *end()
		{
			return pptr();
		}
		steal_buffer(std::ostream &out) 
		{
			init();
			steal(out);
		}
		steal_buffer() 
		{
			init();
		}
		void steal(std::ostream &out)
		{
			release();
			stolen_ = out.rdbuf(this);
			stream_ = &out;
		}
		void release()
		{
			if(stream_ && stolen_) {
				stream_->rdbuf(stolen_);
			}
			stream_ = 0;
			stolen_ = 0;
		}
		~steal_buffer()
		{
			release();
			free(on_heap_);
		}
		int overflow(int c)
		{
			size_t current_size;
			size_t new_size;
			if(pbase() == on_stack_) {
				current_size = on_stack_size;
				new_size = on_stack_size * 2;
				on_heap_ = (char *)malloc(new_size);
				if(!on_heap_)
					throw std::bad_alloc();
				memcpy(on_heap_,on_stack_,current_size);
			}
			else {
				current_size = pptr() - pbase();
				new_size = current_size * 2;
				char *new_ptr = (char *)realloc(on_heap_,new_size);
				if(!new_ptr)
					throw std::bad_alloc();
				on_heap_ = new_ptr;

			}
			setp(on_heap_,on_heap_ + new_size);
			pbump(current_size);
			if(c!=EOF)
				sputc(c);
			return 0;
		}
	private:
		void init()
		{
			on_heap_ = 0;
			stolen_ = 0;
			stream_ = 0;
			setp(on_stack_,on_stack_+on_stack_size);
		}
		char *on_heap_;
		std::streambuf *stolen_;
		std::ostream *stream_;
		char on_stack_[on_stack_size];
	};
	
} // util
} // cppcms

#endif
