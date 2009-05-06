#ifndef CPPCMS_HTTP_RESPONSE_H
#define CPPCMS_HTTP_RESPONSE_H

namespace cppcms { namespace http {

	class response {
		std::string content_type_;
		std::vector<cookie> cookies_;
		std::string redirect_;
		int status_;
		std::string status_message_;
		std::vector<std::string> headers_;
		bool ostream_requested_;
		streaming_device *device_;
		bool disable_compression_;
		
		void write_headers();
	public:
		typedef enum { temporary = 302 , permanently=303 } redirect_type;
		typedef enum {

		// syncronouse io
			normal, // write request, use buffering, compression,
			nogzip, // as normal but disable gzip
			direct, // use direct connection for transferring huge
				// amount of data, for example big csv, file download
		// async
			asynchronous,
				// the data is buffered and transferred asynchronously
				// in one chunk only, for long poll
			asynchronous_chunked,
				// allow many chunks being transferred, each 
				// push transferes one chunk
			asynchronous_multipart,
				// use multipart transfer encoding, requires parameter
				// content type, default is "multipart/mixed"
			asynchronous_raw
				// use your own asynchronous data trasfer

		} io_mode_type;

		io_mode_type io_mode();
		
		void io_mode(io_mode_type);

		void content_type(std::string const &);
		void set_cookie(cookie const &);
		void redirect(std::string const &url,redirect_type how=temporary);
		void status(int code,std::string const &message);
		void header(std::string type,std::string content);
		void reset();

		ostream &out(); // output buffer

		async_push(boost::function<void(bool)>);
		async_push(std::string const &content_type,boost::function<void(bool)>);
		async_complete();
		async_idle(boost::function<void(bool)>);
	};

} /* http */ } /* cppcms */


#endif
