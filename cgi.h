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
#ifndef CPPCMS_CGI_H
#define CPPCMS_CGI_H

namespace cppcms {

class cgi_cgi_session : public cgi_session {
	cgicc_connection_cgi cgi;
public:
	virtual cgicc_connection &get_connection() { return cgi; };
	virtual bool prepare() { return true; } ;
	virtual ~cgi_cgi_session() {};
};

class cgi_cgi_api : public cgi_api {
public:
	virtual int get_socket() { return 0; };
	virtual cgi_session *accept_session() { return new cgi_cgi_session(); };
	virtual ~cgi_cgi_api() {};
};

};

#endif
