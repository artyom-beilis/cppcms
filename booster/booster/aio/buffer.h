//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_BUFFER_H
#define BOOSTER_AIO_BUFFER_H

#include <vector>
#include <string>

namespace booster {
	namespace aio {
		///
		/// \brief This is a base class that represents a buffer - a set of
		/// contiguous chunks of memory that can be transfered over network.
		///
		/// The \a Pointer parameter should be either <tt>char const *</tt> or
		/// <tt>char *</tt>
		///
		template<typename Pointer>
		class buffer_impl {
		public:
			///
			/// One chunk - pointer and its size
			///
			struct entry {
				Pointer ptr;
				size_t size;
			};
			
			///
			/// Create an empty buffer
			///
			buffer_impl() : 
				size_(0)
			{
			}
		
			/// A pair that defined the chunk	
			typedef std::pair<entry const *,size_t> buffer_data_type;

			///
			/// Get a first chunk
			///
			std::pair<entry const *,size_t> get() const
			{
				if(size_ == 0) {
					buffer_data_type bt;
					bt.first=0;
					bt.second=0;
					return bt;
				}
				else if(size_ == 1)
					return buffer_data_type(&entry_,1);
				else
					return buffer_data_type(&vec_.front(),vec_.size());
			}
			///
			/// Append a chunk to the buffer
			///
			void add(Pointer p,size_t s)
			{
				if(s==0)
					return;
				if(size_ == 0) {
					entry_.ptr = p;
					entry_.size = s;
					size_ = 1;
					return;
				}
				else if(size_ == 1) {
					vec_.push_back(entry_);
				}
				entry tmp = { p,s };
				vec_.push_back(tmp);
				size_ = vec_.size();
			}
			///
			/// Check if the buffer is empty
			///
			bool empty() const
			{
				return size_ == 0;
			}
		private:
			int size_;
			entry entry_;
			std::vector<entry> vec_;
		};

		///
		/// \brief A mutable buffer - a buffer for read operations
		///
		class mutable_buffer : public buffer_impl<char *> {
		public:
			mutable_buffer() {}
		};

		///
		/// \brief An immutable buffer - buffer for write operations
		///
		class const_buffer : public buffer_impl<const char *> {
		public:
			const_buffer() {}
			///
			/// Create a const buffer from mutable buffer
			///
			const_buffer(mutable_buffer const &other) 
			{
				mutable_buffer::buffer_data_type data = other.get();
				for(unsigned i=0;i<data.second;i++)
					add(data.first[i].ptr,data.first[i].size);
			}
		};

		///
		/// Create a buffer from POD object of size \a n
		///
		inline const_buffer buffer(void const *p,size_t n)
		{
			const_buffer tmp;
			tmp.add(reinterpret_cast<char const *>(p),n);
			return tmp;
		}
		///
		/// Create a buffer from POD object of size \a n
		///
		inline mutable_buffer buffer(void *p,size_t n)
		{
			mutable_buffer tmp;
			tmp.add(reinterpret_cast<char *>(p),n);
			return tmp;
		}
		///
		/// Create a buffer from std::vector<char>
		///
		inline const_buffer buffer(std::vector<char> const &d)
		{
			return buffer(&d.front(),d.size());
		}
		///
		/// Create a buffer from std::vector<char>
		///
		inline mutable_buffer buffer(std::vector<char> &d)
		{
			return buffer(&d.front(),d.size());
		}
		///
		/// Create a buffer from std::string
		///
		inline const_buffer buffer(std::string const &d)
		{
			return buffer(d.c_str(),d.size());
		}

		/// \cond internal

		namespace details {
			template<typename Buffer>
			Buffer advance(Buffer const &buf,size_t n)
			{
				Buffer res;
				typename Buffer::buffer_data_type data=buf.get();
				while(data.second > 0 && n > 0) {
					if(data.first->size <= n) {
						n-= data.first->size;
						data.second--;
						data.first++;
					}
					else {
						res.add(data.first->ptr + n,data.first->size - n);
						n=0;
						data.second--;
						data.first++;
					}
				}
				while(data.second > 0) {
					res.add(data.first->ptr,data.first->size);
					data.second--;
					data.first++;
				}
				return res;
			}
			template<typename Buffer>
			void add(Buffer &left,Buffer const &right)
			{
				typename Buffer::buffer_data_type data=right.get();
				for(unsigned i=0;i<data.second;i++)
					left.add(data.first[i].ptr,data.first[i].size);
			}
		} // details

		/// \endcond

		///
		/// Create a new buffer by "consuming" \a n bytes. For example if the
		/// buffer has 100 bytes then adding 50 to it would create a buffer
		/// with remaining 50 bytes
		///
		inline const_buffer operator+(const_buffer const &buf,size_t n)
		{
			return details::advance(buf,n);
		}
		///
		/// Create a new buffer by "consuming" \a n bytes. For example if the
		/// buffer has 100 bytes then adding 50 to it would create a buffer
		/// with remaining 50 bytes
		///
		inline mutable_buffer operator+(mutable_buffer const &buf,size_t n)
		{
			return details::advance(buf,n);
		}
		///
		/// Consume \a n bytes from the buffer.
		///
		inline const_buffer &operator+=(const_buffer &buf,size_t n)
		{
			buf = details::advance(buf,n);
			return buf;
		}
		///
		/// Consume \a n bytes from the buffer.
		///
		inline mutable_buffer &operator+=(mutable_buffer &buf,size_t n)
		{
			buf = details::advance(buf,n);
			return buf;
		}
		///
		/// Append two buffers
		///
		inline const_buffer operator+(const_buffer const &b1,const_buffer const &b2)
		{
			const_buffer tmp=b1;
			details::add(tmp,b2);
			return tmp;
		}
		///
		/// Append one buffer to another
		///
		inline const_buffer &operator+=(const_buffer &b1,const_buffer const &b2)
		{
			details::add(b1,b2);
			return b1;
		}
		///
		/// Append two buffers
		///
		inline mutable_buffer operator+(mutable_buffer const &b1,mutable_buffer const &b2)
		{
			mutable_buffer tmp=b1;
			details::add(tmp,b2);
			return tmp;
		}
		///
		/// Append one buffer to another
		///
		inline mutable_buffer &operator+=(mutable_buffer &b1,mutable_buffer const &b2)
		{
			details::add(b1,b2);
			return b1;
		}
	
	} // aio
} // booster





#endif
