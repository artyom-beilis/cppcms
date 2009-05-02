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
		void content_type(std::string const &);
		void set_cookie(cookie const &);
		void redirect(std::string const &url,redirect_type how=temporary);
		void status(int code,std::string const &message);
		void header(std::string type,std::string content);
		void reset();
		ostream &out();
	};

} /* http */ } /* cppcms */


#endif
