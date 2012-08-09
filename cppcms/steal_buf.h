///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
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

	///
	/// \brief Very simple output stream buffer that uses stack for small chunks of 
	/// text and then allocates memory of the default buffer is too small.
	///
	/// It is something like std::stringbuf with small string optimization, it also
	/// allows to access the memory without actually creating the string itself.
	///
	/// The template parameter defines how many characters should be allocated
	/// on stack by default before heap is used.
	///
	template<size_t OnStackSize = 128>
	class stackbuf : public std::streambuf {
		stackbuf(stackbuf const &);
		void operator=(stackbuf const &);
	public:
		///
		/// get the pointer to the beginning of the output buffer
		///
		char *begin()
		{
			return pbase();
		}
		///
		/// get the pointer to the end of the output buffer
		///
		char *end()
		{
			return pptr();
		}
		///
		/// get the 0 terminated string from the buffer.
		///
		char *c_str()
		{
			*pptr() = 0;
			return begin();
		}
		///
		/// get the std::string from the buffer.
		///
		std::string str()
		{
			return std::string(begin(),size_t(end()-begin()));
		}
		stackbuf() 
		{
			init();
		}
		~stackbuf()
		{
			free(on_heap_);
		}
	protected:

		int overflow(int c)
		{
			size_t current_size;
			size_t new_size;
			if(pbase() == on_stack_) {
				current_size = OnStackSize;
				new_size = OnStackSize * 2;
				on_heap_ = (char *)malloc(new_size + 1);
				if(!on_heap_)
					throw std::bad_alloc();
				memcpy(on_heap_,on_stack_,current_size);
			}
			else {
				current_size = pptr() - pbase();
				new_size = current_size * 2;
				char *new_ptr = (char *)realloc(on_heap_,new_size + 1);
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
			setp(on_stack_,on_stack_+OnStackSize);
		}
		char *on_heap_;
		char on_stack_[OnStackSize + 1];
	};


	///
	/// \brief This is a special buffer that allows to "steal" some chunk
	/// of text from the output stream.
	///
	/// It does this by replacing stream's streambuf object with temporary 
	/// stream buffer that records all data written to the stream and then
	/// returns the original buffer upon call to release() member function
	/// it steal_buffer destruction.
	///
	/// The \a Size parameter defines the default chunk of memory allocated
	/// on the stack before heap is used.
	///	
	template<size_t Size = 128>
	class steal_buffer : public stackbuf<Size> {
	public:
		///
		/// Create the buffer and "Steal" the buffer from \a out
		///
		steal_buffer(std::ostream &out) 
		{
			stolen_ = 0;
			stream_ = 0;
			steal(out);
		}
		///
		/// Create an empty buffer
		///
		steal_buffer() 
		{
			stolen_ = 0;
			stream_ = 0;
		}
		///
		/// Steal the buffer from \a out
		///
		void steal(std::ostream &out)
		{
			release();
			stolen_ = out.rdbuf(this);
			stream_ = &out;
		}
		///
		/// Release the "stolen" buffer back, now the buffer contains all
		/// the data that was recorded.
		///
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
		}
	private:
		std::streambuf *stolen_;
		std::ostream *stream_;
	};

	///
	/// \brief Fast output stream object.
	///
	/// This is a special ostream that uses stack in order
	/// to receive the data and faster then std::stringstream for
	/// small chunks, also it is not limited for any particular size.
	///
	template<size_t Size = 128>
	class stackstream : public std::ostream {
	public:
		///
		/// Create a new stackstream
		///
		stackstream() : std::ostream(0)
		{
			rdbuf(&buf_);
		}
		///
		/// Get the pointer to the first character in the range
		///
		char *begin()
		{
			return buf_.begin();
		}
		///
		/// Get the pointer to the one past last character in the range
		///
		char *end()
		{
			return buf_.end();
		}
		///
		/// Get a NUL terminated recorded string
		///
		char *c_str()
		{
			return buf_.c_str();
		}
		///
		/// Get a recorded string
		///
		std::string str()
		{
			return buf_.str();
		}
	private:
		stackbuf<Size> buf_;
	};
	
} // util
} // cppcms

#endif
