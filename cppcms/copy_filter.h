///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_COPY_FILTER_H
#define CPPCMS_COPY_FILTER_H


#include <booster/streambuf.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>
#include <cppcms/defs.h>
#include <list>
#include <ostream>

namespace cppcms {
	///
	/// \brief Copy the output stream part - "tee" filter
	///
	/// This simple class designed to "copy" all the output that comes to the stream
	/// to internal buffer and the stream itself and return the copied data on detaching 
	/// the filter.
	///
	/// It is very useful to use with caching, for example:
	///
	/// \code
	/// std::string frame;
	/// if(cache().fetch_frame("key",frame)) {
	///   out() << frame;
	/// }
	/// else {
	///   cppcms::copy_filter tee(out());
	///   ...
	///   // generate something heavy
	///   ...
	///   cache().store_frame("key",tee.detach());
	/// }
	/// \endcode
	///
	class CPPCMS_API copy_filter : public booster::noncopyable {
	public:
		///
		/// Create a filter copying all output to internal buffer
		///
		copy_filter(std::ostream &output);
		///
		/// When destroyed, if the stream wasn't detached aborts coping the data,
		/// making it exception safe.
		///
		~copy_filter();
		///
		/// Stop the coping process and return all collected data as string.
		///
		std::string detach();
		
	private:
		class tee_device;
		struct data;
		booster::hold_ptr<data> d;
		booster::streambuf copy_buffer_;
		std::ostream &output_;
		std::ostream real_output_stream_;
		std::list<std::string> data_;
		bool detached_;
	};
} // cppcms


#endif
