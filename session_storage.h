#ifndef CPPCMS_SESSION_STORAGE_H
#define CPPCMS_SESSION_STORAGE_H

#include "defs.h"
#include "refcounted.h"
#include "noncopyable.h"

namespace cppcms {
namespace sessions {

	///
	/// \a session_server_storage is an abstract class that allows user to implements
	/// custom session storage device like, database storage device
	///
	/// Note: if the member functions save/load/remove are thread safe -- can be called
	/// from different threads, than you may create a single session and return \a intrusive_ptr
	/// to a single intstance, otherwise you have to create multiple instances of object
	///
	
	class session_server_storage : 
		public util::noncopyable,
		public refcounted
	{
	public:
		///
		/// Save session with end of life time at \a timeout using session id \a sid and content \a in
		///
	
		virtual void save(std::string const &sid,time_t timeout,std::string const &in) = 0;

		///
		/// Load session with \a sid, put its end of life time to \a timeout and return its
		/// value to \a out
		///
		virtual bool load(std::string const &sid,time_t &timeout,std::string &out) = 0;
		
		///
		/// Remove a session with id \a sid  from the storage
		///
		
		virtual void remove(std::string const &sid) = 0;
		
		virtual ~session_server_storage()
		{
		}
	};


} // sessions
} // cppcms


#endif
