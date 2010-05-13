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
namespace cppcms {
	class http_client_impl {
	public:
		http_client_impl() :
			method("GET"),
			timeout(10),
			status(0)

		std::string url;
		std::string method;
		std::string post_data;
		std::string content_type;
		std::list<std::string> headers;
		int timeout;
		
		int status;
		std::string reason;
		std::string response;
		std::vector<std::pair<std::string,std::string> > response_headers;
		
		void async_transfer(booster::function<void(completion_status_type) const &handler,boost::asio::io_service &srv)
		{
			static const boost::regex r("^[hH][Tt][Tt][Pp]://([^:/]+)(:(\\d+))?(.*)$");
			boost::cmatch match;
			if(!boost::regex_match(url.c_str(),match,r)) {
				srv.post(boost::bind(handler,http_client::invalid_request));
				return;
			}
			std::string host=match[1];
			std::string sport=match[3];
			std::string get=match[4];
			int port = sport.empty() ? 80 : atoi(sport.c_str());
			if(get.empty())
				get="/";
			headers.push_back("Host: " + host);
			headers.push_back("Accept: */*");
			headers.push_back("Connection: close");	
			if(method=="POST") {
				headers.push_back("Content-Type: " + content_type);
				std::ostringstream ss;
				ss.imbue(std::locale::classic());
				ss<<post_data.size();
				headers.push_back("Content-Lenght: " + ss.str());
			}
			std::string full_request;
			full_request.reserve(post_data.size()+150);
			full_request+=method;
			full_request+=' ';
			full_request+=get;
			full_request+=" HTTP/1.0\r\n";
			for(std::list<std::string>::iterator p=headers.begin();p!=headers.end();++p) {
				full_request+=*p;
				full_request+="\r\n";
			}
			full_request+="\r\n";
			if(method=="POST")
				full_request+=post_data;
		}
	};

} 

