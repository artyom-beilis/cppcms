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
		/// Abort upload progress, thrown from classes derived from basic_content_filter
		/// to abort the upload progress. status_code is the HTTP error code returned, for example 413
		/// requested entity is too large
		///
		abort_upload(int status_code);
		virtual ~abort_upload() throw();
		///
		/// Get the code
		/// 
		int code() const;
	private:
		int code_;
	};

	///
	/// Class that represent the limits on the input content sizes
	///
	class CPPCMS_API content_limits : public booster::noncopyable {
		friend class request;
	public:
		/// \cond INTERNAL
		content_limits(impl::cached_settings const &);
		content_limits();
		~content_limits();
		/// \endcond 

		///
		/// Get the size limit in bytes of any non multipart/form-data content 
		///
		/// Note form fields without content-type would be limited by this 
		/// size even if the multipart_form_data_limit is much larger
		///
		long long content_length_limit() const;
		///
		/// Set the size limit of any non multipart/form-data content 
		///
		/// Note form fields without content-type would be limited by this 
		/// size even if the multipart_form_data_limit is much larger
		///
		void content_length_limit(long long size);

		///
		/// Get the size limit of multipart/form-data content in bytes 
		///
		/// Note form fields without content-type would be limited by content_length_limit
		/// size even if the multipart_form_data_limit is much larger
		///
		long long multipart_form_data_limit() const;
		///
		/// Set the size limit of multipart/form-data content in bytes 
		///
		/// Note form fields without content-type would be limited by content_length_limit
		/// size even if the multipart_form_data_limit is much larger
		///
		void multipart_form_data_limit(long long size);

		///
		/// Get the maximal size of file that is still hold in memory rather than disk
		///
		size_t file_in_memory_limit() const;
		///
		/// Set the maximal size of file that is still hold in memory rather than disk
		///
		void file_in_memory_limit(size_t size);

		///
		/// Get a location of a temporary directory that files are uploaded to, if empty system default is used
		///
		std::string uploads_path() const;
		///
		/// Set a location of a temporary directory that files are uploaded to, if empty system default is used
		///
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
