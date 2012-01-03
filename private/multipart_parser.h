///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_MULTIPART_PARSER_H
#define CPPCMS_IMPL_MULTIPART_PARSER_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/shared_ptr.h>
#include <cppcms/http_file.h>
#include <cppcms/http_content_type.h>
#include <vector>

#include "http_protocol.h"


namespace cppcms {
	namespace impl {
		class multipart_parser : public booster::noncopyable {
		public:
			typedef booster::shared_ptr<http::file> file_ptr;
			typedef std::vector<file_ptr> files_type;
			
			files_type get_files()
			{
				files_type result;
				result.swap(files_);
				return result;
			}
			bool set_content_type(std::string const &ct)
			{
				http::content_type t(ct);
				return set_content_type(t);
			}
			bool set_content_type(http::content_type const &content_type)
			{
				std::string bkey = content_type.parameter_by_key("boundary");
				if(bkey.empty())
					return false;
				boundary_ = "\r\n--" + bkey;
				crlfcrlf_ = "\r\n\r\n";
				position_ = 2; // First CRLF does not appear if first state
				state_ = expecting_first_boundary;
				file_.reset(new http::file());
				file_->set_temporary_directory(temp_dir_);
				if(memory_limit_ != -1) {
					file_->set_memory_limit(memory_limit_);
				}
				return true;
			}
			
			multipart_parser(std::string const &temp_dir = std::string(),size_t memory_limit = -1) : 
				state_ ( expecting_first_boundary ),
				position_(0),
				temp_dir_(temp_dir),
				memory_limit_(memory_limit)
			{
			}
			~multipart_parser()
			{
			}
			
			typedef enum {
				parsing_error,
				continue_input,
				got_something,
				eof
			} parsing_result_type;

			parsing_result_type consume(char const *buffer,int size)
			{
				for(;size > 0;buffer++,size--) {
					#ifdef DEBUG_MULTIPART_PARSER
					std::cerr << "[";
					if(*buffer < 32) 
						std::cerr << int(*buffer);
					else
						std::cerr << *buffer ;
					std::cerr <<"]" << state_str_[state_] << ", pos="<<position_ << std::endl;
					#endif
					switch(state_) {
					case expecting_first_boundary:
						if(*buffer != boundary_[position_])
							return parsing_error;
						position_++;
						if(position_ == boundary_.size()) {
							state_ = expecting_one_crlf_or_eof;
							position_ = 0;
						}
						break;
					case expecting_one_crlf_or_eof:
						if(*buffer=='\r')
							state_=expecting_lf;
						else if(*buffer=='-')
							state_=expecting_minus;
						else
							return parsing_error;
						break;
					case expecting_minus:
						if(*buffer!='-')
							return parsing_error;
						state_=expecting_eof_cr;
						break;
					case expecting_eof_cr:
						if(*buffer!='\r')
							return parsing_error;
						state_=expecting_eof_lf;
						break;
					case expecting_eof_lf:
						if(*buffer!='\n')
							return parsing_error;
						if(size == 1)
							return eof;
						else
							return parsing_error;
					case expecting_lf:
						if(*buffer!='\n')
							return parsing_error;
						state_=expecting_crlfcrlf;
						break;
					case expecting_crlfcrlf:
						header_+=*buffer;
						if(*buffer == crlfcrlf_[position_])
							position_++;
						else
							position_=0;
						if(position_ == crlfcrlf_.size()) {
							if(!process_header(header_))
								return parsing_error;
							header_.clear();
							position_ = 0;
							state_ = expecting_separator_boundary;
						}
						break;
					case expecting_separator_boundary:
						if(*buffer == boundary_[position_])
							position_++;
						else if(position_ > 0) {
							file_->write_data().write(boundary_.c_str(),position_);
							position_ = 0;
							if(*buffer == boundary_[0])
								position_=1;
						}
						if(position_ == 0) {
							file_->write_data() << *buffer;
						}
						else if(position_ == boundary_.size()) {
							state_ = expecting_one_crlf_or_eof;
							position_ = 0;
							file_->data().seekg(0);
							files_.push_back(file_);
							file_.reset(new http::file());
							file_->set_temporary_directory(temp_dir_);
							if(memory_limit_ != -1) {
								file_->set_memory_limit(memory_limit_);
							}
						}
						break;
					}
				}
				if(!files_.empty())
					return got_something;
				else
					return continue_input;
			}

		private:
			
			bool process_header(std::string const &hdr)
			{
				size_t pos = 0;
				while(pos < hdr.size()) {
					size_t next = hdr.find("\r\n",pos);
					if(next==std::string::npos) {
						return false;
					}
					if(next == pos) {
						return true;
					}
					std::string one_header=hdr.substr(pos,next-pos);
					pos=next+2;
					std::string::iterator p=one_header.begin(),e=one_header.end();
					p=http::protocol::skip_ws(p,e);
					std::string::iterator start=p;
					std::string::iterator end=http::protocol::tocken(p,e);
					std::string header_name(start,end);
					p=http::protocol::skip_ws(end,e);
					if(p==e || *p!=':')
						return false;
					p=http::protocol::skip_ws(p+1,e);
					
					if(http::protocol::compare(header_name,"Content-Disposition")==0) {
						start = p;
						end=http::protocol::tocken(p,e);
						if(http::protocol::compare(std::string(start,end),"form-data")!=0)
							return false;
						p=end;
						if(!parse_content_disposition(p,e))
							return false;
					}
					else if(http::protocol::compare(header_name,"Content-Type")==0) {
						http::content_type ct(std::string(p,e));
						file_->mime(ct.media_type());
					}
				}
				return false;

			}

			bool parse_content_disposition(std::string::iterator p,std::string::iterator e)
			{
				p=http::protocol::skip_ws(p,e);
				while(p!=e) {
					std::string name,value;
					if(!parse_pair(p,e,name,value))
						return false;
					for(unsigned i=0;i<name.size();i++)
						name[i]=http::protocol::ascii_to_lower(name[i]);
					if(name=="filename")
						file_->filename(value);
					else if(name=="name")
						file_->name(value);
					p=http::protocol::skip_ws(p,e);
				}
				return true;
			}

			bool parse_pair(std::string::iterator &p,std::string::iterator e,std::string &name,std::string &value)
			{
				if(p==e)
					return false;
				if(*p!=';')
					return false;
				p=http::protocol::skip_ws(p+1,e);
				if(p==e)
					return false;
				std::string::iterator start = p;
				std::string::iterator end = http::protocol::tocken(p,e);
				if(start==end)
					return false;
				if(end==e)
					return false;
				if(*end!='=')
					return false;
				name.assign(start,end);
				p=http::protocol::skip_ws(end+1,e);
				if(p==e)
					return false;
				if(*p=='"') {
					std::string::iterator tmp=p;
					value=http::protocol::unquote(tmp,e);
					if(p==tmp)
						return false;
					p=tmp;
					return true;
				}
				else {
					std::string::iterator tmp=http::protocol::tocken(p,e);
					if(tmp==p)
						return false;
					value.assign(p,tmp);
					tmp=p;
					return true;
				}
			}

			typedef enum {
				expecting_first_boundary,
				expecting_one_crlf_or_eof,
				expecting_minus,
				expecting_eof_cr,
				expecting_eof_lf,
				expecting_lf,
				expecting_crlfcrlf,
				expecting_separator_boundary
			} states_type;

			#ifdef DEBUG_MULTIPART_PARSER
			static char const * const state_str_[8];
			#endif

			states_type state_;
			file_ptr file_;
			files_type files_;
			size_t position_;
			std::string header_;
			std::string boundary_;
			std::string crlfcrlf_;

			std::string temp_dir_;
			int memory_limit_;
		};

			#ifdef DEBUG_MULTIPART_PARSER
			char const * const multipart_parser::state_str_[8] = {
				"expecting_first_boundary",
				"expecting_one_crlf_or_eof",
				"expecting_minus",
				"expecting_eof_cr",
				"expecting_eof_lf",
				"expecting_lf",
				"expecting_crlfcrlf",
				"expecting_separator_boundary"
			};
			#endif
		
	}
}


#endif
