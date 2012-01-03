///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
// make sure we all defines are given
#include "dir.h"
#include <cppcms/config.h>
# if defined(CPPCMS_HAVE_CANONICALIZE_FILE_NAME) && !defined(_GNU_SOURCE)
# define _GNU_SOURCE
#endif

#include <stdlib.h>

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include "internal_file_server.h"
#include <cppcms/cppcms_error.h>
#include <cppcms/json.h>
#include <cppcms/util.h>
#include <sstream>
#include <booster/nowide/fstream.h>
#include <booster/locale/encoding.h>
#include <booster/locale/formatting.h>

#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifndef CPPCMS_WIN_NATIVE
#include <unistd.h>
#include <limits.h>
#endif


namespace cppcms {
namespace impl {

file_server::file_server(cppcms::service &srv) : application(srv)
{
	if(!canonical(settings().get("file_server.document_root","."),document_root_))
		throw cppcms_error("Invalid document root");

	list_directories_ = settings().get("file_server.listing",false);
	index_file_ = settings().get("file_server.index","index.html");

	std::string mime_file=settings().get("file_server.mime_types","");

	allow_deflate_ = settings().get("file_server.allow_deflate",false);

	if(settings().find("file_server.alias").type()==json::is_array) {
		json::array const &alias = settings().find("file_server.alias").array();
		for(unsigned i=0;i<alias.size();i++) {
			std::string url = alias[i].get<std::string>("url");
			if(url.size() < 2 || url[0]!='/') {
				throw cppcms_error("Invalid alias URL:" + url);
			}
			if(url[url.size()-1]=='/')
				url.resize(url.size()-1);
			std::string input_path = alias[i].get<std::string>("path");
			std::string canon_path;
			if(!canonical(input_path,canon_path)) {
				throw cppcms_error("Invalid alias path" + input_path);
			}
			alias_.push_back(std::make_pair(url,canon_path));
		}
	}

	if(mime_file.empty()) {
		mime_[".pdf"]	=      "application/pdf";
		mime_[".sig"]	=      "application/pgp-signature";
		mime_[".spl"]	=      "application/futuresplash";
		mime_[".ps"]	=      "application/postscript";
		mime_[".torrent"]=      "application/x-bittorrent";
		mime_[".dvi"]	=      "application/x-dvi";
		mime_[".gz"]	=      "application/x-gzip";
		mime_[".pac"]	=      "application/x-ns-proxy-autoconfig";
		mime_[".swf"]	=      "application/x-shockwave-flash";
		mime_[".tgz"]	=      "application/x-tgz";
		mime_[".tar"]	=      "application/x-tar";
		mime_[".zip"]	=      "application/zip";
		mime_[".mp3"]	=      "audio/mpeg";
		mime_[".m3u"]	=      "audio/x-mpegurl";
		mime_[".wma"]	=      "audio/x-ms-wma";
		mime_[".wax"]	=      "audio/x-ms-wax";
		mime_[".ogg"]	=      "application/ogg";
		mime_[".wav"]	=      "audio/x-wav";
		mime_[".gif"]	=      "image/gif";
		mime_[".jpg"]	=      "image/jpeg";
		mime_[".jpeg"]	=      "image/jpeg";
		mime_[".png"]	=      "image/png";
		mime_[".xbm"]	=      "image/x-xbitmap";
		mime_[".xpm"]	=      "image/x-xpixmap";
		mime_[".xwd"]	=      "image/x-xwindowdump";
		mime_[".css"]	=      "text/css";
		mime_[".html"]	=      "text/html";
		mime_[".htm"]	=      "text/html";
		mime_[".js"]	=      "text/javascript";
		mime_[".asc"]	=      "text/plain";
		mime_[".c"]	=      "text/plain";
		mime_[".cpp"]	=      "text/plain";
		mime_[".log"]	=      "text/plain";
		mime_[".conf"]	=      "text/plain";
		mime_[".text"]	=      "text/plain";
		mime_[".txt"]	=      "text/plain";
		mime_[".dtd"]	=      "text/xml";
		mime_[".xml"]	=      "text/xml";
		mime_[".mpeg"]	=      "video/mpeg";
		mime_[".mpg"]	=      "video/mpeg";
		mime_[".mov"]	=      "video/quicktime";
		mime_[".qt"]	=      "video/quicktime";
		mime_[".avi"]	=      "video/x-msvideo";
		mime_[".asf"]	=      "video/x-ms-asf";
		mime_[".asx"]	=      "video/x-ms-asf";
		mime_[".wmv"]	=      "video/x-ms-wmv";
		mime_[".bz2"]	=      "application/x-bzip";
		mime_[".tbz"]	=      "application/x-bzip-compressed-tar";
	}
	else {
		load_mime_types(mime_file);
	}

}

void file_server::load_mime_types(std::string file_name)
{
	booster::nowide::ifstream inp(file_name.c_str());
	if(!inp) {
		return;
	}
	std::string line;
	while(!inp.eof() && getline(inp,line)) {
		if(line.empty() || line[0]=='#')
			continue;
		std::istringstream ss(line);
		std::string mime;
		std::string ext;
		if(ss>>mime) {
			while(ss>>ext) {
				mime_["."+ext]=mime;
			}
		}
	}
}

file_server::~file_server()
{
}

bool file_server::canonical(std::string normal,std::string &real)
{
#ifndef CPPCMS_WIN_NATIVE


	#ifdef CPPCMS_HAVE_CANONICALIZE_FILE_NAME

		char *canon=::canonicalize_file_name(normal.c_str());
		if(!canon) return false;
		try { 
			real=canon;
		}
		catch(...)
		{
			free(canon);
			throw; 
		}
		free(canon);
		canon=0;

	#else
		#if defined(PATH_MAX)
			int len = PATH_MAX;
		#else
			int len = pathconf(normal.c_str(),_PC_PATH_MAX);
			if(len <= 0)
				len = 32768; // Hope it is enough
		#endif

		std::vector<char> buffer;
		try { 
			// Size may be not feasible for allocation according to POSIX
			buffer.resize(len,0);
		}
		catch(std::bad_alloc const &e) {
			buffer.resize(32768); 
		}

		char *canon = ::realpath(normal.c_str(),&buffer.front());
		if(!canon)
			return false;
		real = canon;
	#endif

#else
		wchar_t *wreal = 0;
		try {
			std::wstring wnormal = booster::locale::conv::utf_to_utf<wchar_t>(normal,booster::locale::conv::stop);
			wchar_t *wreal = _wfullpath(0,wnormal.c_str(),0);
			if(!wreal)
				return false;
			real = booster::locale::conv::utf_to_utf<char>(wreal,booster::locale::conv::stop);
			free(wreal);
			wreal = 0;
		}
		catch(booster::locale::conv::conversion_error const &) {
			if(wreal)
				free(wreal);
			return false;
		}
		// stat would not work on files like foo/ so remove the last slash as realpath
		// and canonicalize does
		if(real.size()>1 && real[real.size()-1]=='\\')
			real.resize(real.size()-1);
#endif
	return true;
}

static bool is_directory_separator(char c)
{
#ifdef CPPCMS_WIN32
	return c=='\\'  || c=='/';
#else
	return c=='/';
#endif
}

static bool is_file_prefix(std::string const &prefix,std::string const &full)
{
	size_t prefix_size = prefix.size();
	if(prefix_size > full.size())
		return false;
	if(memcmp(prefix.c_str(),full.c_str(),prefix_size) != 0)
		return false;
	if(prefix_size == 0 || is_directory_separator(prefix[prefix_size-1]))
		return true;
	if(full.size() > prefix_size && !is_directory_separator(full[prefix_size]))
		return false;
	return true;
}

bool file_server::is_in_root(std::string const &input_path,std::string const &root,std::string &real)
{
	std::string normal=root + "/" + input_path;
	if(!canonical(normal,real))
		return false;
	if(!is_file_prefix(root,real))
		return false;
	return true;
}

bool file_server::check_in_document_root(std::string normal,std::string &real) 
{
	// Use only Unix file names
	for(size_t i=0;i<normal.size();i++)
		if(is_directory_separator(normal[i]))
			normal[i]='/';

	std::string root = document_root_;
	for(unsigned i=0;i<alias_.size();i++) {
		std::string const &ref=alias_[i].first;
		if(is_file_prefix(ref,normal))
		{
			root = alias_[i].second;
			normal = normal.substr(ref.size());
			if(normal.empty())
				normal="/";
			break;
		}
	}
	if(normal.empty())
		return false;
	if(normal[0]!='/')
		return false;
	// Prevent the access to any valid file below like
	// detecting that the files placed in /var/www
	// by providing a path /../../var/www/known.txt 
	// whuch would be valid as known is placed in /var/www
	// but yet we don't want user to detect that files
	// exist in /var/www
	for(size_t pos = 1;pos != std::string::npos; pos = normal.find('/',pos)) {
		std::string sub_path = normal.substr(0,pos);
		std::string tmp;
		if(!is_in_root(sub_path,root,tmp))
			return false;
		pos++;
	}
	if(!is_in_root(normal,root,real))
		return false;
	return true;
}

namespace {

#ifdef CPPCMS_WIN_NATIVE
typedef struct _stat port_stat;
int get_stat(char const *name,port_stat *st)
{
	std::wstring wname = booster::locale::conv::utf_to_utf<wchar_t>(name,booster::locale::conv::stop);
	return ::_wstat(wname.c_str(),st);
}
#else
typedef struct stat port_stat;
int get_stat(char const *name,port_stat *st)
{
	return ::stat(name,st);
}
#endif

}

int file_server::file_mode(std::string const &file_name)
{
	port_stat st;
	if(get_stat(file_name.c_str(),&st) < 0)
		return 0;
	return st.st_mode;
}

void file_server::list_dir(std::string const &url,std::string const &path)
{
	cppcms::impl::directory d;
	if(!d.open(path)) {
		show404();
		return;
	}
	
#ifdef CPPCMS_WIN_NATIVE
	response().content_type("text/html; charset=UTF-8");
#endif
	std::ostream &out = response().out();
	out << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
	       "     \"http://www.w3.org/TR/html4/loose.dtd\">\n";

	out << "<html><head><title>Directory Listing</title></head>\n"
		"<body><h1>Index of " << util::escape(url) << "</h1>\n";
	out << booster::locale::as::gmt;
	//out <<"<table cellpadding='0' cellspacing='2' border='0' >\n";
	out <<"<table>\n";
	out <<"<thead><tr><td width='60%'>File</td><td width='20%' >Date</td><td width='5%'>&nbsp;</td><td width='15%'>Size</td></tr></thead>\n"
		"<tbody>\n";
	if(url!="/" && !url.empty()) {
		out << "<tr><td><code><a href='../' >..</a></code></td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n";
	}
	out << booster::locale::as::ftime("%Y-%m-%d %H:%M:%S");
	while(d.next()) {
		if(memcmp(d.name(),".",1) == 0)
			continue;
		port_stat st;
		if(get_stat((path + "/" + d.name()).c_str(),&st) < 0)
			continue;
		char const *add="";
		if(st.st_mode & S_IFDIR)
			add="/";
		else if(st.st_mode & S_IFREG)
			;
		else 
			continue;
		out << "<tr>";
		out << "<td><code><a href='" 
			<< util::urlencode(d.name()) << add << "'>" << util::escape(d.name()) << add << "</a></code></td>";
		out << "<td>" << booster::locale::as::strftime << st.st_mtime <<"</td><td>&nbsp;</td>";
		if(st.st_mode & S_IFREG)
			out << "<td>" << booster::locale::as::number << st.st_size <<"</td>";
		else
			out << "<td> <strong>-</strong> </td>";
		out <<"</tr>\n";
	}
	out <<"</tbody>\n</table>\n";
	out <<"<p>CppCMS-Embedded/" CPPCMS_PACKAGE_VERSION "</p>\n";
	out <<"</body>\n";
}

void file_server::main(std::string file_name)
{
	std::string path;


	if(!check_in_document_root(file_name,path)) {
		show404();
		return;
	}
	
	int s=file_mode(path);
	
	if((s & S_IFDIR)) {
		std::string path2;
		int mode_2=0;
	
		bool have_index = check_in_document_root(file_name+"/" + index_file_ ,path2);
		if(have_index) {
			mode_2 = file_mode(path2);
			have_index = (mode_2  & S_IFREG) != 0;
		}

		if(     !file_name.empty() 
			&& file_name[file_name.size()-1]!='/'  // not ending with "/" as should
			&& (have_index || list_directories_)
		) 
		{
			response().set_redirect_header(file_name + "/");
			response().out()<<std::flush;
			return;
		}
		if(have_index) {
			path = path2;
			s=mode_2;
		}
		else {
			if(list_directories_) 
				list_dir(file_name,path);
			else
				show404();
			return;
		}

	}

	if(!(s & S_IFREG)) {
		show404();
		return;
	}
		
	std::string ext;
	size_t pos = path.rfind('.');
	if(pos != std::string::npos)
		ext=path.substr(pos);

	mime_type::const_iterator p=mime_.find(ext);
	if(p!=mime_.end()) 
		response().content_type(p->second);
	else
		response().content_type("application/octet-stream");

	if(!allow_deflate_) {
		response().io_mode(http::response::nogzip);
	}

	booster::nowide::ifstream file(path.c_str(),std::ios_base::binary);
	if(!file) {
		show404();
		return;
	}
	response().out()<<file.rdbuf(); // write stream to stream
}

void file_server::show404()
{
	response().status(http::response::not_found);
	response().set_html_header();
	response().out() <<
		"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
		"	\"http://www.w3.org/TR/html4/loose.dtd\">\n"
		"<html>\n"
		"  <head>\n"
		"    <title>404 Not Found</title>\n"
		"  </head>\n"
		"  <body>\n"
		"    <h1>404 Not Found</h1>\n"
		"  </body>\n"
		"</html>\n"<<std::flush;
}


} // impl
} // cppcms
