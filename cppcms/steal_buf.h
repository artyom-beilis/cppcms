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
#include <istream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>

namespace cppcms {
namespace util {

	///
	/// Simple std::streambuf to create input from [char const *,char const *) range
	///
	/// \ver{v1_2}

	class const_char_buf : public std::streambuf {
	public:
		///
		/// Create Empty buffer
		///
		const_char_buf()
		{
			range(0,0);
		}
		///
		/// Create a buffer from a range
		///
		const_char_buf(char const *begin,char const *end)
		{
			range(begin,end);
		}
		///
		/// Define the range for existing buffer, pointer is reset to begin
		///
		void range(char const *cbegin,char const *cend)
		{
			char *begin = const_cast<char*>(cbegin);
			char *end   = const_cast<char*>(cend);
			setg(begin,begin,end);
		}
		///
		/// Begin of range
		///
		char const *begin() const
		{
			return eback();
		}
		///
		/// End of range
		///
		char const *end() const
		{
			return egptr();
		}
	};

	///
	/// Simple std::istream implementation for range of [char const *,char const *)
	///
	/// \ver{v1_2}
	class const_char_istream : public std::istream {
	public:
		///
		/// Create new empty stream
		///
		const_char_istream() : std::istream(0) 
		{
			init(&buf_);
		}
		///
		/// Create stream initialized with range [begin,end)
		///
		const_char_istream(char const *begin,char const *end) : std::istream(0) 
		{
			buf_.range(begin,end);
			init(&buf_);
		}
		///
		/// Get begin of the range
		///
		char const *begin() const
		{
			return buf_.begin();
		}
		///
		/// Get end of the range
		///
		char const *end() const
		{
			return buf_.end();
		}
		///
		/// Set range, resets pointer to start and clears flags
		///
		void range(char const *begin,char const *end)
		{
			buf_.range(begin,end);
			clear();
		}
	private:
		const_char_buf buf_;
	};

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

	template<typename Filter,int BufferSize=128>
	class filterbuf : public std::streambuf {
	public:
		filterbuf() : output_(0), output_stream_(0)
		{
			setp(buffer_,buffer_+BufferSize);
		}
		filterbuf(std::ostream &out) : output_(0), output_stream_(0)
		{
			setp(buffer_,buffer_+BufferSize);
			steal(out);
		}
		~filterbuf()
		{
			try {
				release();
			}
			catch(...) {}
		}
		void steal(std::ostream &out)
		{
			release();
			output_stream_ = &out;
			output_ = out.rdbuf(this);
		}
		int release()
		{
			int r=0;
			if(output_stream_) {
				if(write()!=0)
					r=-1;
				output_stream_->rdbuf(output_);
				output_=0;
				output_stream_=0;
			}
			return r;
		}
	protected:
		int overflow(int c)
		{
			if(write()!=0)
				return -1;
			if(c!=EOF) {
				*pptr()=c;
				pbump(1);
			}
			return 0;
		}
	private:
		int write()
		{
			if(static_cast<Filter*>(this)->convert(pbase(),pptr(),output_)!=0) {
				output_stream_->setstate(std::ios_base::failbit);
				return -1;
			}
			setp(buffer_,buffer_ + BufferSize);
			return 0;
		}
		char buffer_[BufferSize];
		std::streambuf *output_;
		std::ostream *output_stream_;
	};
	
	template<typename Filter>
	class filterbuf<Filter,0> : public std::streambuf {
	public:
		filterbuf() : output_(0), output_stream_(0)
		{
		}
		filterbuf(std::ostream &out) : output_(0), output_stream_(0)
		{
			steal(out);
		}
		~filterbuf()
		{
			release();
		}
		void steal(std::ostream &out)
		{
			release();
			output_stream_ = &out;
			output_ = out.rdbuf(this);
		}
		int release()
		{
			int r=0;
			if(output_stream_) {
				output_stream_->rdbuf(output_);
				output_=0;
				output_stream_=0;
			}
			return r;
		}
	protected:
		int overflow(int c)
		{
			if(c!=EOF) {
				char tmp=c;
				if(write(&tmp,&tmp+1)!=0)
					return -1;
			}
			return 0;
		}
		std::streamsize xsputn(char const *s,std::streamsize size)
		{
			if(write(s,s+size)!=0)
				return -1;
			return size;
		}
	private:
		int write(char const *begin,char const *end)
		{
			if(static_cast<Filter*>(this)->convert(begin,end,output_)!=0) {
				output_stream_->setstate(std::ios_base::failbit);
				return -1;
			}
			return 0;
		}
		std::streambuf *output_;
		std::ostream *output_stream_;
	};
	
} // util
} // cppcms

#endif
