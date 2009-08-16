#define CPPCMS_SOURCE
#include "application.h"
#include "url_dispatcher.h"
#include "service.h"
#include "http_response.h"
#include "internal_file_server.h"
#include "global_config.h"
#include <sstream>
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/system/error_code.hpp>


namespace fs = boost::filesystem;

namespace cppcms {
namespace impl {

file_server::file_server(cppcms::service &srv) : application(srv)
{
	document_root_ = settings().str("http.document_root","./");

	dispatcher().assign("^(.*)$",&file_server::serve_file,this,1);

	std::string mime_file=settings().str("http.mime_types","");


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
	std::ifstream inp(file_name.c_str());
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

void file_server::serve_file(std::string file_name)
{
	if(file_name.find("..") || file_name.empty() || file_name[0]!='/') {
		show404();
		return;
	}
	
	fs::path full = fs::path(document_root_,fs::native) / fs::path(file_name);
	
	boost::system::error_code e;
	fs::file_status status = fs::status(full,e);

	if(e) {
		show404();
		return;
	}

	if(fs::is_directory(status)) {
		full = full / fs::path("index.html");
		status = fs::status(full,e);
		if(e) {
			show404();
			return;
		}
	}

	if(!fs::is_regular_file(status)) {
		show404();
		return;
	}
	std::string ext=full.extension();
	mime_type::const_iterator p=mime_.find(ext);
	if(p!=mime_.end()) 
		response().content_type(p->second);
	else
		response().content_type("application/octet-stream");
	if(!settings().integer("http.allow_deflate",0)) {
		response().io_mode(http::response::nogzip);
	}
	std::ifstream file(full.file_string().c_str(),std::ifstream::binary | std::ifstream::in);
	if(!file) {
		show404();
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
