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
	struct cached_settings;
}

namespace http {
	class file;
	class context;

	///
	/// Exceptions that is thrown to abort content upload progress indicating an error
	///
	/// \ver{v1_2}
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
	/// \ver{v1_2}
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

	///
	/// Basic content filter that can be installed to request, all filters should be derived from this base class
	///
	/// Note that when `on_*` member functions of the basic_content_filter are called the original application that runs the filtering
	/// has temporary installed context that can be accessed from it.
	///
	/// \ver{v1_2}
	class CPPCMS_API basic_content_filter {
		basic_content_filter(basic_content_filter const &);
		void operator=(basic_content_filter const &);
	public:
		basic_content_filter();
		virtual ~basic_content_filter();

		///
		/// Member function that is called when entire content is read. By default does nothing.
		///
		/// The request can be aborted by throwing abort_upload
		///
		virtual void on_end_of_content();
		///
		/// Member function that is called in case of a error occuring during upload progress, user should not throw exception from this function but rather
		/// perform cleanup procedures if needed
		///
		virtual void on_error();
	private:
		struct _data;
		booster::hold_ptr<_data> d;
	};

	///
	/// Process of any kind of generic content data.
	///
	/// Note: when raw_content_filter is used no content data is actually saved to request, for example request().raw_post_data() would return
	/// an empty content, so it is your responsibility to store/parse whatever content you use
	///
	/// \ver{v1_2}
	class CPPCMS_API raw_content_filter : public basic_content_filter {
	public:
		///
		/// You must implement this member function to handle the data
		///
		/// A chunk of incoming data is avalible refered by data of size data_size
		///
		/// The request can be aborted by throwing abort_upload
		///
		virtual void on_data_chunk(void const *data,size_t data_size) = 0;

		raw_content_filter();
		virtual ~raw_content_filter();
	private:
		struct _raw_data;
		booster::hold_ptr<_raw_data> d;
	};

	///
	/// Filter for multipart/form-data - file upload
	///
	/// It allows to process/validate incomping data on the fly and make sure that for example the user is actually authorized to upload
	/// such a files
	///
	/// \ver{v1_2}
	class CPPCMS_API multipart_filter : public basic_content_filter {
	public:
		multipart_filter();
		virtual ~multipart_filter();
		///
		/// New file meta-data of a form field or file is ready: the mime-type, form name and file name if provided are known, the content wasn't processed yet
		///
		/// Notes:
		///
		/// - This is the point when you can change various file properties, like location of the temporary file or specifiy output file name and more
		/// - The request can be aborted by throwing abort_upload
		/// - By default does nothing
		///
		virtual void on_new_file(http::file &input_file);
		///
		/// Some of the file data is available, you can access it and run some validation during upload progress.
		///
		/// Notes:
		///
		/// - This is the point when you can perform some file content validation
		/// - The request can be aborted by throwing abort_upload
		/// - By default does nothing
		///
		virtual void on_upload_progress(http::file &input_file);
		///
		/// The entire file data was transfered, its size wouldn't change
		///
		/// Notes:
		///
		/// - This is the point when you can save file if needed or perform final validation
		/// - The request can be aborted by throwing abort_upload
		/// - By default does nothing
		///
		virtual void on_data_ready(http::file &input_file);
	
	private:
		struct _mp_data;
		booster::hold_ptr<_mp_data> d;
	};

} // http
} // cppcms

#endif
