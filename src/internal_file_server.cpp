///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
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
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef CPPCMS_WIN_NATIVE
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif


namespace cppcms {
namespace impl {

file_server::file_server(cppcms::service &srv) : application(srv)
{
	if(!canonical(settings().get("file_server.document_root","."),document_root_))
		throw cppcms_error("Invalid document root");

	std::string mime_file=settings().get("file_server.mime_types","");

	allow_deflate_ = settings().get("file_server.allow_deflate",false);


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
		real=canon;
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
		int size=4096;
		std::vector<char> buffer(size,0);
		for(;;) {
			DWORD res = ::GetFullPathName(normal.c_str(),buffer.size(),&buffer.front(),0);
			if(res == 0)
				return false;
			if(res >= buffer.size()) {
				buffer.resize(buffer.size()*2,0);
			}
			else {
				real=&buffer.front();
				break;
			}
		}
#endif
	return true;
}

bool file_server::check_in_document_root(std::string normal,std::string &real) 
{
	normal=document_root_ + "/" + normal;
	if(!canonical(normal,real))
		return false;
	if(real.size() < document_root_.size() || memcmp(real.c_str(),document_root_.c_str(),document_root_.size()) !=0)
		return false;
	return true;
}

int file_server::file_mode(std::string const &file_name)
{
	struct stat st;
	if(::stat(file_name.c_str(),&st) < 0)
		return 0;
	return st.st_mode;
}

void file_server::main(std::string file_name)
{
	file_name = util::urldecode(file_name);
	if(file_name.empty() || file_name[file_name.size()-1]=='/')
		file_name+="/index.html";

	std::string path;

	if(!check_in_document_root(file_name,path)) {
		show404();
		return;
	}
	
	int s=file_mode(path);
	
	if((s & S_IFDIR) && (file_mode(path+"/index.html") & S_IFREG)) {
		response().set_redirect_header(file_name + "/");
		response().out()<<std::flush;
		return;
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
	std::vector<char> buffer(4096);
	while(!file.eof()) {
		file.read(&buffer.front(),buffer.size());
		response().out().write(&buffer.front(),file.gcount());
	}
	response().out()<<std::flush;
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
