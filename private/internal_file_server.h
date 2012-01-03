///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <string>
#include <map>

namespace cppcms {
	class service;
namespace impl {
	class file_server : public application
	{
	public:
		file_server(cppcms::service &srv);
		~file_server();
		virtual void main(std::string file_name);

	private:
		void list_dir(std::string const &url,std::string const &path);
		void show404();
		void load_mime_types(std::string);
		bool canonical(std::string normal,std::string &real);
		bool is_in_root(std::string const &normal,std::string const &root,std::string &real);
		bool check_in_document_root(std::string normal,std::string &real);
		int file_mode(std::string const &path);

		bool allow_deflate_;
		std::string document_root_;
		std::vector<std::pair<std::string,std::string> > alias_;
		typedef std::map<std::string,std::string> mime_type;
		mime_type mime_;
		bool list_directories_;
		std::string index_file_;
	};


}
}
