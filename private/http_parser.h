///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_HTTP_PARSER_H
#define CPPCMS_HTTP_PARSER_H
#include "http_protocol.h"
#include <vector>
#include <string>

namespace cppcms {
namespace http {
namespace impl {


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


	// Non copyable

	parser(parser const &);
	parser const &operator=(parser const &);
protected:
	inline int getc()
	{
		if(body_ptr_ < body_.size()) {
			return (unsigned char)body_[body_ptr_++];
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

public:
	std::string header_;

	parser(std::vector<char> &body,unsigned &body_ptr) :
		state_(idle),
		bracket_counter_(0),
		body_(body),
		body_ptr_(body_ptr)
	{
		header_.reserve(512);
	}
	void reset()
	{
		state_ = idle;
		bracket_counter_ = 0;
		header_.clear();
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

/*			switch(state_) {
			case idle:
			case input_observed:
			case last_lf_exptected:
			case lf_exptected:
			case space_or_other_exptected:
				if(separator(c)) {
					parsed_.push_back(element_type(c,std::string()));
				}
				else if(0x20 <= c && c<=0x7E) {
					if(parsed_.empty() || parsed_.back().first != token_element) {
						parsed.push_back(element_type(token_element,std::string()));
					}
					parsed.back().second+=c;
				}
				break;
			default:
				; // Nothing
			}*/
		}
	}

}; // class parser


}}} // cppcms::http::impl

#endif
