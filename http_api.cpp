#define CPPCMS_SOURCE
#include "asio_config.h"
#include "cgi_api.h"
#include "cgi_acceptor.h"
#include "service.h"
#include "service_impl.h"
#include "cppcms_error_category.h"
#include "global_config.h"
#include "http_protocol.h"
#include "config.h"
#include <iostream>
#include <algorithm>
#include <boost/lexical_cast.hpp>

//#define DEBUG_HTTP_PARSER


namespace cppcms {
namespace impl {
namespace cgi {
	class http : public connection {
	public:
		http(cppcms::service &srv) :
			connection(srv),
			socket_(srv.impl().get_io_service()),
			input_body_ptr_(0),
			input_parser_(input_body_,input_body_ptr_),
			output_body_ptr_(0),
			output_parser_(output_body_,output_body_ptr_),
			headers_done_(false),
			first_header_observerd_(false),
			total_read_(0)
		{

			env_["SERVER_SOFTWARE"]=PACKAGE_NAME "/" PACKAGE_VERSION;
			env_["SERVER_NAME"]=srv.settings().str("service.ip","127.0.0.1");
			env_["SERVER_PORT"]=boost::lexical_cast<std::string>(srv.settings().integer("service.port"));
			env_["GATEWAY_INTERFACE"]="CGI/1.0";
			env_["SERVER_PROTOCOL"]="HTTP/1.0";

		}
		virtual void async_read_headers(handler const &h)
		{
			input_body_.reserve(8192);
			input_body_.resize(8192,0);
			input_body_ptr_=0;
			socket_.async_read_some(boost::asio::buffer(input_body_),
				boost::bind(	&http::some_headers_data_read,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred,
						h));
		}

		void some_headers_data_read(boost::system::error_code const &e,size_t n,handler const &h)
		{
			if(e) { h(e); return; }

			total_read_+=n;
			if(total_read_ > 16384) {
				h(boost::system::error_code(errc::protocol_violation,cppcms_category));
				return;
			}

			input_body_.resize(n);

			for(;;) {

				switch(input_parser_.step()) {
				case parser::more_data:
					// Assuming body_ptr == body.size()
					async_read_headers(h);
					return;
				case parser::got_header:
					if(!first_header_observerd_) {
						first_header_observerd_=true;
						std::string::iterator rmethod,query=input_parser_.header_.end();
						rmethod=std::find(	input_parser_.header_.begin(),
									input_parser_.header_.end(),
									' ');
						if(rmethod!=input_parser_.header_.end()) 
							query=std::find(rmethod+1,input_parser_.header_.end(),' ');
						if(query!=input_parser_.header_.end()) {
							request_method_.assign(input_parser_.header_.begin(),rmethod);
							request_uri_.assign(rmethod+1,query);
							first_header_observerd_=true;
						}
						else {
							h(boost::system::error_code(errc::protocol_violation,cppcms_category));
							return;
						}
					}
					else { // Any other header
						std::string name;
						std::string value;
						if(!parse_single_header(input_parser_.header_,name,value))  {
							h(boost::system::error_code(errc::protocol_violation,cppcms_category));
							return;
						}
						if(name=="CONTENT_LENGTH" || name=="CONTENT_TYPE")
							env_[name]=value;
						else
							env_["HTTP_"+name]=value;
					}
					break;
				case parser::end_of_headers:
					process_request(h);				
					break;
				case parser::error_observerd:
					h(boost::system::error_code(errc::protocol_violation,cppcms_category));
					return;
					break;
				}
			}


		}

		// should be called only after headers are read
		virtual std::string getenv(std::string const &key)
		{
			std::map<std::string,std::string>::const_iterator p;
			p=env_.find(key);
			if(p==env_.end())
				return std::string();
			return p->second;
		}
		virtual void async_read_some(void *p,size_t s,io_handler const &h)
		{
			if(input_body_ptr_==input_body_.size()) {
				input_body_.clear();
				input_body_ptr_=0;
			}
			if(!input_body_.empty()) {
				if(input_body_.size() - input_body_ptr_ < s) {
					s=input_body_.size() -  input_body_ptr_;
				}
				memcpy(p,&input_body_[input_body_ptr_],s);
				input_body_ptr_+=s;
				if(input_body_ptr_==input_body_.size()) {
					input_body_.clear();
					input_body_ptr_=0;
				}
				socket_.get_io_service().post(boost::bind(h,boost::system::error_code(),s));
				return;
			}
			socket_.async_read_some(boost::asio::buffer(p,s),h);
		}
		virtual void async_write_eof(handler const &h)
		{
			boost::system::error_code e;
			socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send,e);
			socket_.get_io_service().post(boost::bind(h,boost::system::error_code()));
		}
		virtual void async_write_some(void const *p,size_t s,io_handler const &h)
		{
			if(headers_done_)
				socket_.async_write_some(boost::asio::buffer(p,s),h);
			else
				process_output_headers(p,s,h);
		}
		virtual size_t write_some(void const *buffer,size_t n)
		{
			if(headers_done_)
				return socket_.write_some(boost::asio::buffer(buffer,n));
			return process_output_headers(buffer,n);
		}
		virtual boost::asio::io_service &get_io_service()
		{
			return socket_.get_io_service();
		}
		virtual bool keep_alive()
		{
			return false;
		}

		virtual void close()
		{
			boost::system::error_code e;
			socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_receive,e);
			socket_.close(e);
		}

	private:
		size_t process_output_headers(void const *p,size_t s,io_handler const &h=io_handler())
		{
			char const *ptr=reinterpret_cast<char const *>(p);
			output_body_.insert(output_body_.end(),ptr,ptr+s);

			for(;;) {
				switch(output_parser_.step()) {
				case parser::more_data:
					if(!h.empty())
						h(boost::system::error_code(),s);
					return s;
				case parser::got_header:
					{
						std::string name,value;
						if(!parse_single_header(output_parser_.header_,name,value))  {
							h(boost::system::error_code(errc::protocol_violation,cppcms_category),s);
							return s;
						}
						if(name=="STATUS") {
							response_line_ = "HTTP/1.0 "+value+"\r\n";

							return write_response(h,s);
						}
					}
					break;
				case parser::end_of_headers:
					response_line_ = "HTTP/1.0 200 Ok\r\n";

					return write_response(h,s);
				case parser::error_observerd:
					h(boost::system::error_code(errc::protocol_violation,cppcms_category),0);
					return 0;
				}
			}
	
		}
		size_t write_response(io_handler const &h,size_t s)
		{
			char const *addon = 
				"Server: CppCMS-Embedded/" PACKAGE_VERSION "\r\n"
				"Connection: close\r\n";

			response_line_.append(addon);

			boost::array<boost::asio::const_buffer,2> packet = {
				{
					boost::asio::buffer(response_line_),
					boost::asio::buffer(output_body_)
				}
			};
#ifdef DEBUG_HTTP_PARSER
			std::cerr<<"["<<response_line_<<std::string(output_body_.begin(),output_body_.end())
				<<"]"<<std::endl;
#endif
			headers_done_=true;
			if(h.empty()) {
				boost::system::error_code e;
				boost::asio::write(socket_,packet,boost::asio::transfer_all(),e);
				if(e) return 0;
				return s;
			}
			boost::asio::async_write(socket_,packet,
				boost::bind(&http::do_write,shared_from_this(),_1,h,s));
			return s;
		}

		void do_write(boost::system::error_code const &e,io_handler const &h,size_t s)
		{
			if(e) { h(e,0); return; }
			h(boost::system::error_code(),s);
		}

		void process_request(handler const &h)
		{
			if(request_method_!="GET" && request_method_!="POST" && request_method_!="HEAD") {
				response("HTTP/1.0 501 Not Implemented\r\n\r\n",h);
				return;
			}

			env_["REQUEST_METHOD"]=request_method_;
			env_["REMOTE_HOST"] = env_["REMOTE_ADDR"] = socket_.remote_endpoint().address().to_string();

			if(request_uri_.empty() || request_uri_[0]!='/') {
				response("HTTP/1.0 400 Bad Request\r\n\r\n",h);
				return;
			}
			
			std::vector<std::string> const &script_names=service().settings().slist("http.script_names");

			for(unsigned i=0;i<script_names.size();i++) {

				std::string const &name=script_names[i];

				if(	request_uri_.size() >= name.size() 
					&& memcmp(request_uri_.c_str(),name.c_str(),name.size())==0)
				{
					env_["SCRIPT_NAME"]=name;
					size_t pos=request_uri_.find('?');
					if(pos==std::string::npos)
						env_["PATH_INFO"]=request_uri_.substr(name.size());
					else {
						env_["PATH_INFO"]=request_uri_.substr(name.size(),pos-name.size());
						env_["QUERY_STRING"]=request_uri_.substr(pos+1);
					}
					h(boost::system::error_code());
					return;
				}
			}
			
			size_t pos=request_uri_.find('?');
			if(pos==std::string::npos)
				env_["PATH_INFO"]=request_uri_;
			else {
				env_["PATH_INFO"]=request_uri_.substr(0,pos);
				env_["QUERY_STRING"]=request_uri_.substr(pos+1);
			}
			
			h(boost::system::error_code());

		}
		void response(char const *message,handler const &h)
		{
			boost::asio::async_write(socket_,boost::asio::buffer(message,strlen(message)),
					boost::bind(h,boost::system::error_code()));
		}

		bool parse_single_header(std::string const &header,std::string &name,std::string &value)
		{
			std::string::const_iterator p=header.begin(),e=header.end(),name_end;

			p=cppcms::http::protocol::skip_ws(p,e);
			name_end=cppcms::http::protocol::tocken(p,e);
			if(name_end==p)
				return false;
			name.assign(p,name_end);
			p=name_end;
			p=cppcms::http::protocol::skip_ws(p,e);
			if(p==e || *p!=':')
				return false;
			++p;
			p=cppcms::http::protocol::skip_ws(p,e);
			value.assign(p,e);
			for(unsigned i=0;i<name.size();i++) {
				if(name[i] == '-') 
					name[i]='_';
				else if('a' <= name[i] && name[i] <='z')
					name[i]=name[i]-'a' + 'A';
			}
			return true;
		}

		class parser {
			enum {
				idle,
				input_observed,
				last_lf_exptected,
				lf_exptected,
				space_or_other_exptected,
				quote_expected,
				pass_quote_exptected,
				closing_bracket_expected,
				pass_closing_bracket_expected
			} state_;
			
			unsigned bracket_counter_;

			std::vector<char> &body_;
			unsigned &body_ptr_;

			inline int getc()
			{
				if(body_ptr_ < body_.size()) {
					return body_[body_ptr_++];
				}
				else {
					body_.clear();
					body_ptr_=0;
					return -1;
				}
			}
			inline void ungetc(int c)
			{
				if(body_ptr_ > 0) {
					body_ptr_--;
					body_[body_ptr_]=c;
				}
				else {
					body_.insert(body_.begin(),c);
				}
			}

			// Non copyable

			parser(parser const &);
			parser const &operator=(parser const &);

		public:
			std::string header_;

			parser(std::vector<char> &body,unsigned &body_ptr) :
				state_(idle),
				bracket_counter_(0),
				body_(body),
				body_ptr_(body_ptr)
			{
			}
			enum { more_data, got_header, end_of_headers , error_observerd };
			int step()
			{
#ifdef DEBUG_HTTP_PARSER
				static char const *states[]= {
					"idle",
					"input_observed",
					"last_lf_exptected",
					"lf_exptected",
					"space_or_other_exptected",
					"quote_expected",
					"pass_quote_exptected",
					"closing_bracket_expected",
					"pass_closing_bracket_expected"
				};
#endif
				for(;;) {
					int c=getc();
#if defined DEBUG_HTTP_PARSER
					std::cerr<<"Step("<<body_ptr_<<":"<<body_.size()<<": "<<std::flush;
					if(c>=32)
						std::cerr<<"["<<char(c)<<"] "<<states[state_]<<std::endl;
					else
						std::cerr<<c<<" "<<states[state_]<<std::endl;
#endif

					if(c<0)
						return more_data;


					switch(state_)  {
					case idle:
						header_.clear();
						switch(c) {
						case '\r':
							state_=last_lf_exptected;
							break;
						case '"':
							state_=quote_expected;
							break;
						case '(':
							state_=closing_bracket_expected;
							bracket_counter_++;
							break;
						default:
							state_=input_observed;
						}
						break;
					case last_lf_exptected:
						if(c!='\n') return error_observerd;
						header_.clear();
						return end_of_headers;
					case lf_exptected:
						if(c!='\n') return error_observerd;
						state_=space_or_other_exptected;
						break;
					case space_or_other_exptected:
						if(c==' ' || c=='\t') {
							// Convert LWS to space as required by
							// RFC, so remove last CRLF
							header_.resize(header_.size() - 2);
							state_=input_observed;
							break;
						}
						ungetc(c);
						header_.resize(header_.size()-2);
						state_=idle;
#ifdef DEBUG_HTTP_PARSER
						std::cerr<<"["<<header_<<"]"<<std::endl;
#endif
						return got_header;					
					case input_observed:
						switch(c) {
						case '\r':
							state_=lf_exptected;
							break;
						case '"':
							state_=quote_expected;
							break;
						case '(':
							state_=closing_bracket_expected;
							bracket_counter_++;
							break;
						default:
							state_=input_observed;
						}
						break;
					case quote_expected:
						switch(c) {
						case '"':
							state_=input_observed;
							break;
						case '\\':
							state_=pass_quote_exptected;
							break;
						}
						break;
					case pass_quote_exptected:
						if(c < 0 || c >=127)
							return error_observerd;
						state_=quote_expected;
						break;
					case closing_bracket_expected:
						switch(c) {
						case ')':
							bracket_counter_--;
							if(bracket_counter_==0)
								state_=input_observed;
							break;
						case '\\':
							state_=pass_closing_bracket_expected;
							break;
						}
						break;
					case pass_closing_bracket_expected:
						if(c < 0 || c >=127)
							return error_observerd;
						state_=closing_bracket_expected;
						break;
					}

					header_+=char(c);
				}
			}

		};

		boost::shared_ptr<http> shared_from_this()
		{
			return boost::static_pointer_cast<http>(connection::shared_from_this());
		}
		
		friend class socket_acceptor<boost::asio::ip::tcp,http>;
		
		boost::asio::ip::tcp::socket socket_;

		std::vector<char> input_body_;
		unsigned input_body_ptr_;
		parser input_parser_;
		std::vector<char> output_body_;
		unsigned output_body_ptr_;
		parser output_parser_;


		std::map<std::string,std::string> env_;
		std::string script_name_;
		std::string response_line_;
		std::string request_method_;
		std::string request_uri_;
		bool headers_done_;
		bool first_header_observerd_;
		unsigned total_read_;
	};

	typedef tcp_socket_acceptor<http>    http_acceptor;

	std::auto_ptr<acceptor> http_api_factory(cppcms::service &srv,std::string ip,int port,int backlog)
	{
		std::auto_ptr<acceptor> a(new http_acceptor(srv,ip,port,backlog));
		return a;
	}



} // cgi
} // impl
} // cppcms


