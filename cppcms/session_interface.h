///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_SESSION_INTERFACE_H
#define CPPCMS_SESSION_INTERFACE_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>
#include <booster/shared_ptr.h>
#include <cppcms/cstdint.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/serialization_classes.h>
#include <string>
#include <map>
#include <memory>
#include <sstream>
#include <typeinfo>

namespace cppcms {
namespace http {
	class context;
	class request;
	class response;
}

class session_api;

///
/// \brief This exception is thrown when CSRF attempt is suspected:
///
class CPPCMS_API request_forgery_error : public cppcms_error {
public:
	/// Create an exception object
	request_forgery_error() : 
		cppcms_error("Cross site request forgery detected")
	{
	}
};

///
/// \brief This class provides an access to an application for session management
///
/// Usually it is accessed via application::session member function.
///
/// Note, when the application is asynchronous, the sessions should be loaded manually as
/// fetching information from session may be not so cheap
///
/// Generally, session data is represented as a map of key-value strings that can be read
/// and written. All changes in session should be done before headers are written to the output
/// (before requesting an output stream from http::response object)
///
/// Each of the values in session may be also exposed to client as cookie. For example if
/// you want to disclose "foo" session key to the client side (Java Script) and session
/// cookie is cppcms_session=S231abc23c34ca242352a then a cookie with name
/// cppcms_session_foo will be created caring the value of this key.
///
/// Notes:
/// - Be careful with values you pass, cookies can carry quite a limited range of strings
///  so it is your responsibility to make sure that these values are actually legal.
/// - Of course the client side can alter these cookies but this would not have an effect
/// on the actual values fetched using session object. But still remember you should not
/// relay on cookies values on server side for obvious security reasons.
///
class CPPCMS_API session_interface : private booster::noncopyable {
public:

	/// \cond INTERNAL
	session_interface(http::context &);
	~session_interface();
	/// \endcond

	///
	/// Check if a \a key is set (assigned some value to it) in the session 
	///
	bool is_set(std::string const &key);
	///
	/// Erase specific \a key from the session
	///
	void erase(std::string const &key);
	///
	/// Remove all keys from the session and delete the session at all. (i.e. empty session is automatically deleted)
	///
	void clear();

	///
	/// Returns true if specific \a key is exposed to client via cookies
	///
	bool is_exposed(std::string const &key);
	///
	/// Set exposition of the \a key to client side, if val is true the value will be exposed, otherwise hidden and cookie
	/// will be deleted.
	///
	void expose(std::string const &key,bool val=true);
	///
	/// Disable exposition of a \a key. Same as expose(key,false);
	/// 
	void hide(std::string const &key);

	///
	/// Get the reference to a value for a \a key. Note if \a key is not exist, empty string is created in session object
	/// and reference to it returned (similarly to std::map's operator[])
	///
	std::string &operator[](std::string const &key);
	///
	/// Set value \a v for a session key \a key
	///
	void set(std::string const &key,std::string const &v);

	///
	/// Get a value for a session \a key. If it is not set, throws cppcms_error. It is good idea to call is_set before
	/// you call this function.
	///
	std::string get(std::string const &key);
	
	///
	/// Get a value for a session \a key. If it is not set, returns default_value
	///
	std::string get(std::string const &key,std::string const &default_value);

	///
	/// Get convert the value that is set for a key \a key to type T using std::iostream. For example you can
	/// read a number using int n=session().get<int>("number").
	///
	/// - it throws cppcms_error if a key \a key not set/
	/// - it throws std::bad_cast of the conversion using std::iostream fails
	///
	/// Note: the conversion is locale independent (uses C locale)
	///	
	template<typename T>
	T get(std::string const &key)
	{
		std::istringstream ss(get(key));
		ss.imbue(std::locale::classic());
		T value;
		ss>>value;
		if(ss.fail() || !ss.eof())
			throw booster::bad_cast();
		return value;
	}

	///
	/// Assign a \a value of type \a T to \a key converting it to string using std::iostream.
	/// For example session().set("num",100);
	/// Note: the conversion is locale independent (uses C locale)
	///
	template<typename T>
	void set(std::string const &key,T const &value)
	{
		std::ostringstream ss;
		ss.imbue(std::locale::classic());
		ss<<value;
		set(key,ss.str());
	}

	///
	/// Serialize an \a object and store it under a \a key.
	///
	/// The serialization is done using cppcms::serialization_traits
	///
	template<typename Serializable>
	void store_data(std::string const &key,Serializable const &object)
	{
		std::string buffer;
		serialization_traits<Serializable>::save(object,buffer);
		set(key,buffer);
	}

	///
	/// Fetch an \a object under \a key from the session deserializing it.
	///
	/// The serialization is done using cppcms::serialization_traits
	///
	/// Throws cppcms_error if the key is not set, may throw archive_error if deserialization fails
	/// (assuming that serialization uses cppcms::archive.
	///
	template<typename Serializable>
	void fetch_data(std::string const &key,Serializable &object)
	{
		std::string buffer=get(key);
		serialization_traits<Serializable>::load(buffer,object);
	}
	
	///
	/// This enum defines the way session timeout is managed
	///
	enum {
		fixed, 	///< Once the session is created it will expire in age() second from the moment it created
		renew, 	///< Once the session expires in age() seconds of inactivity, once user sends an HTTP request again
			///< it is renewed
		browser ///< The session is kept as long as browser keeps it (does not get closed). In addition the "renew" expiration
			///< policy is valid. So if user does not close his browser but is not active, it will expire in age() seconds.
	};

	///
	/// Get the maximal age of the session in seconds
	///
	int age();
	///
	/// Set the maximal age of the session in seconds
	///
	void age(int t);
	///
	/// Reset the maximal age of the session to default (session.timeout settings value or 24 hours if not set)
	///
	void default_age();

	///
	/// Get the expiration policy of the session: renew, fixed or browser
	///
	int expiration();
	///
	/// Set the expiration policy of the session: renew, fixed or browser
	///
	void expiration(int h);
	///
	/// Reset the expiration policy to the default (session.expire settings value or browser if not set)
	///
	void default_expiration();

	///
	/// Set store on server side option for session. If srv is true then the session will be always stored on server
	/// side and not in cookies only (required "server" or "both" type storage in session.location setting
	///
	/// Rationale: client side storage using encrypted or signed cookies is very efficient, however it lacks of one
	/// important security feature: there is no other way to control their expiration but using timeout.
	/// So user may just revert the cookie to the old state to get back in time and restore its own session.
	///
	/// So it is recommended to use server side storage in such critical cases, like soling captcha or playing a game
	/// where you can't return to previous status when storing the data in the session object.
	///
	void on_server(bool srv);

	///
	/// Get on_server session property
	///
	bool on_server();


	///
	/// Set the cookie that represents the current session (the value of the cookie)
	///
	/// This function should be used only by user implementations of session storage
	///
	void set_session_cookie(std::string const &data);
	///
	/// Remove the cookie of the current session
	///
	/// This function should be used only by user implementations of session storage
	///
	void clear_session_cookie();

	///
	/// Get the value of the cookie that represents session on the client
	///
	/// This function should be used only by user implementations of session storage
	///
	std::string get_session_cookie();

	///
	/// Load the session, should be called one when dealing with sessions on asynchronous API where sessions
	/// are not loaded by default. This function returns true if any data was loaded.
	///
	bool load();

	///
	/// Save the session data, generally should not be called as it is saved automatically. However when
	/// writing asynchronous application and using custom slow storage devices like SQL it may be useful to control
	/// when and how save() is called.
	///
	void save();

	///
	/// Returns true if the underlying session back-end uses blocking API so it is unsuitable to be called
	/// from asynchronous event loop. This can be used to decide how to load session for specific connection.
	///
	/// If the API is blocking you probably should load and save session from thread pool rather then
	/// from event loop when using asynchronous applications.
	///
	bool is_blocking();

	///
	/// When using session id based session - force generation of new session id to prevent session
	/// fixation attacks
	///
	void reset_session();


	///
	/// Check that the CSRF token is the same as in the session object, it does not do any checks, whether
	/// CSRF enabled or the request method is correct. It should be used for custom request handling (like
	/// custom content types for RESTful services.
	///	
	/// Returns true if the token is valid, otherwise returns false
	/// 
	bool validate_csrf_token(std::string const &str);
	///
	/// Check that there is no Cross Site Request Forgery Attempt.
	///
	/// If CSRF checks enabled it validates that there is a valid CSRF token is submitted via POST request
	/// or via X-CSRF-Token header for AJAX requests.
	///
	/// \note it is checked for POST requests only.
	/// 
	void validate_request_origin();

	///
	/// Set CSRF validation mode.
	///
	/// If \a required \c is true then validate_request_origin() would throw \ref request_forgery_error
	/// if the CSRF token is not valid.
	///
	/// Setting it to false would prevent from validate_request_origin() to do any checks.
	///
	/// \note The default is defined in the configuration property \c security.csrf.automatic. If
	/// its value is not set the default is \c true
	///
	/// It is useful when some parts of the application do not require CSRF validation regardless
	/// the status of sepecifc session owner
	///
	void request_origin_validation_is_required(bool required);

	///
	/// Get CSRF token that is stored in the session that can be used for validation
	/// of the request origin
	///
	std::string get_csrf_token();
	///
	/// Get the cooke name that holds CSRF token. Note it can be used only if security.csrf.exposed
	/// is set to true (which is not by default)
	///
	std::string get_csrf_token_cookie_name();

private:
	friend class http::response;
	friend class http::request;



	struct entry;

	typedef std::map<std::string,entry> data_type;
	data_type data_,data_copy_;
	http::context *context_;

	// Cached defaults
	int timeout_val_def_;
	int how_def_;

	// User Values
	int timeout_val_;
	int how_;

	// Information from session data
	time_t timeout_in_;

	uint32_t new_session_ : 1;
	uint32_t saved_ : 1;
	uint32_t on_server_ : 1;
	uint32_t loaded_ : 1;
	uint32_t reset_ : 1;
	uint32_t csrf_checked_ : 1;
	uint32_t csrf_do_validation_ : 1;
	uint32_t csrf_validation_ : 1;
	uint32_t reserved_ : 24;

	std::string temp_cookie_;

	// storage itself
	
	booster::shared_ptr<session_api> storage_;
	struct _data;
	booster::hold_ptr<_data> d; // for future use

	int cookie_age();
	time_t   session_age();

	void check();
	void update_exposed(bool); 


	void set_session_cookie(int64_t age,std::string const &data,std::string const &key=std::string());

	void save_data(std::map<std::string,entry> const &data,std::string &s);
	void load_data(std::map<std::string,entry> &data,std::string const &s);
	std::string generate_csrf_token();
};

} // cppcms


#endif
