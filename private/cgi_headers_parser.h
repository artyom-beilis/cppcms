#ifndef CPPCMS_IMPL_CGI_HEADERS_PARSER_H
#define CPPCMS_IMPL_CGI_HEADERS_PARSER_H
#include "response_headers.h"

namespace cppcms {
namespace impl {
class cgi_headers_parser {
public:
	cgi_headers_parser() : completed_(false)
	{
	}
	
	bool headers_done() const
	{
		return completed_;
	}
	// just to prevent knowing cgi::connection
	template<typename Conn>
	void consume(char const *&data,size_t &length,Conn &conn)
	{
		if(completed_)
			return;
		while(length > 0) {
			header_ += *data++;
			length--;
			size_t size = header_.size();
			if(size >= 2 && header_[size-2] == '\r' && header_[size-1] == '\n') {
				if(size == 2) {
					conn.set_response_headers(h_);
					completed_ = true;
					return;
				}
				add_header();
			}
		}
	}
private:
	void add_header()
	{
		auto start = header_.begin();
		auto end = start + header_.size() - 2;
		using namespace cppcms::http::protocol;
		start = skip_ws(start,end);
		auto key_end = tocken(start,end);
		auto colon=skip_ws(key_end,end);
		auto value_start=(colon != end) ? skip_ws(colon+1,end) : end;
		if(key_end != start && colon < end && *colon == ':')
			h_.add_header(std::string(start,key_end),std::string(value_start,end));
		else
			h_.add_header(header_.substr(0,header_.size()-2));
		header_.clear();
	}

	std::string header_;
	response_headers h_;
	bool completed_;
}; 
}
}

#endif
