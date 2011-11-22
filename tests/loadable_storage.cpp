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
#include <cppcms/defs.h>
#include <cppcms/session_storage.h>
#include <cppcms/json.h>
#include <time.h>
#include <map>

namespace {

class test_storage  : public cppcms::sessions::session_storage {
public:
	///
	/// Save session with end of life time at \a timeout using session id \a sid and content \a in
	///

	virtual void save(std::string const &sid,time_t timeout,std::string const &in)
	{
		data_[sid]=std::pair<time_t,std::string>(timeout,in);
	}

	///
	/// Load session with \a sid, put its end of life time to \a timeout and return its
	/// value to \a out
	///
	virtual bool load(std::string const &sid,time_t &timeout,std::string &out)
	{
		data_type::iterator p=data_.find(sid);
		if(p==data_.end())
			return false;
		if(p->second.first < time(0)) {
			data_.erase(p);
			return false;
		}
		out=p->second.second;
		timeout = p->second.first;
		return true;
	}

	///
	/// Remove a session with id \a sid  from the storage
	///

	virtual void remove(std::string const &sid)
	{
		data_.erase(sid);
	}

	///
	/// Return true of the save or load operations can be blocking
	///
	virtual bool is_blocking() 
	{
		return false;
	}
private:
	typedef std::map<std::string,std::pair<time_t,std::string> > data_type;
	data_type data_;
};

class test_fact : public cppcms::sessions::session_storage_factory {
public:
	///
	/// Get a pointer to session_storage. Note if the returned pointer is same for different calls
	/// session_storage implementation should be thread safe.
	///
	virtual booster::shared_ptr<cppcms::sessions::session_storage> get() 
	{
		return storage_;
	}

	///
	/// Return true if session_storage requires garbage collection - removal of expired session time-to-time
	///
	virtual bool requires_gc() { return false; };
	///
	/// Delete the object, cleanup
	///
	virtual ~test_fact() {}
	test_fact() : 
		storage_(new test_storage())
	{
	}
private:
	booster::shared_ptr<cppcms::sessions::session_storage> storage_;
};

} // anon

#if defined(CPPCMS_WIN32)
# define STORAGE_API declspec(__dllexport)
#else
# define STORAGE_API
#endif

extern "C" {
	STORAGE_API cppcms::sessions::session_storage_factory *my_sessions_generator(cppcms::json::value const &v)
	{
		v.get<bool>("must_be_set");
		return new test_fact();
	}
}

