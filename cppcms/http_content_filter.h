///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2015  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_HTTP_CONTENT_FILTER_H
#define CPPCMS_HTTP_CONTENT_FILTER_H

#include <cppcms/defs.h>
#include <cppcms/cppcms_error.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>
#include <string>

namespace cppcms {

namespace impl {
	class cached_settings;
}

namespace http {
	class file;
	class context;

	///
	/// Exceptions that is thrown to abort content upload progress indicating an error
	///
	class CPPCMS_API abort_upload : public cppcms_error {
	public:
		///
		/// Abort 
		///
		abort_upload(int status_code);
		virtual ~abort_upload() throw();

		int code() const;
	private:
		int code_;
	};


	class CPPCMS_API content_limits : public booster::noncopyable {
		friend class request;
	public:
		/// \cond INTERNAL
		content_limits(impl::cached_settings const &);
		/// \endcond 

		content_limits();
		~content_limits();

		long long content_length_limit() const;
		void content_length_limit(long long size);

		long long multipart_form_data_limit() const;
		void multipart_form_data_limit(long long size);

		size_t file_in_memory_limit() const;
		void file_in_memory_limit(size_t size);

		std::string uploads_path() const;
		void uploads_path(std::string const &path);

	private:
		
		long long content_length_limit_;
		size_t file_in_memory_limit_;
		long long multipart_form_data_limit_;
		std::string uploads_path_;

		struct _data;
		booster::hold_ptr<_data> d;
	};

	class CPPCMS_API basic_content_filter {
		basic_content_filter(basic_content_filter const &);
		void operator=(basic_content_filter const &);
	public:
		basic_content_filter();
		virtual ~basic_content_filter();

		virtual void on_end_of_content();
		virtual void on_error();
	private:
		struct _data;
		booster::hold_ptr<_data> d;
	};

	class CPPCMS_API raw_content_filter : public basic_content_filter {
	public:
		virtual void on_data_chunk(void const *data,size_t data_size) = 0;
		virtual ~raw_content_filter();
	private:
		struct _raw_data;
		booster::hold_ptr<_raw_data> d;
	};

	class CPPCMS_API multipart_filter : public basic_content_filter {
	public:
		multipart_filter();
		virtual ~multipart_filter();
		virtual void on_new_file(http::file &input_file);
		virtual void on_upload_progress(http::file &input_file);
		virtual void on_data_ready(http::file &input_file);
	
	private:
		struct _mp_data;
		booster::hold_ptr<_mp_data> d;
	};

} // http
} // cppcms

#endif
