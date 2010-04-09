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
#ifndef SESSION_SQLITE_STORAGE_H
#define SESSION_SQLITE_STORAGE_H

#include <string>
#include <vector>
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/noncopyable.hpp>
#   include <boost/shared_ptr.hpp>
#else // Internal Boost
#   include <cppcms_boost/noncopyable.hpp>
#   include <cppcms_boost/shared_ptr.hpp>
    namespace boost = cppcms_boost;
#endif
#include "session_storage.h"
#include "session_backend_factory.h"

namespace cppcms {
class cppcms_config;

namespace storage {

struct sqlite;

class sqlite_N {
	vector<boost::shared_ptr<sqlite> > dbs;
	unsigned size;
	sqlite &db(std::string const &sid);
public:
	sqlite_N(string db,int n,bool sync,int def_commits,int def_timeout);
	void save(string const &sid,time_t timeout,string const &data);
	bool load(std::string const &sid,time_t *timeout,std::string &out);
	void remove(string const &sid);
};

} // storage

class session_sqlite_storage : public session_server_storage {
	boost::shared_ptr<storage::sqlite_N> db;
public:
	static session_backend_factory factory(cppcms_config const &);
	session_sqlite_storage(boost::shared_ptr<storage::sqlite_N> );
	virtual void save(std::string const &sid,time_t timeout,std::string const &in);
	virtual bool load(std::string const &sid,time_t *timeout,std::string &out);
	virtual void remove(std::string const &sid) ;
};

} // cppcms


#endif
