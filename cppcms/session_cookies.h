///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_SESSION_COOKIES_H
#define CPPCMS_SESSION_COOKIES_H
#include <cppcms/session_api.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>
#include <memory>
#include <string>
namespace cppcms {
class session_interface;

///
/// \brief this namespace keeps various session storage backends
///
namespace sessions {

	///
	/// \brief This is an interface to generic session cookies encryption or signing API.
	///
	/// Note for users implementing their own ecryptor classes:
	/// 
	/// - Be super paranoid and extremely careful in what you are doing.
	/// - Take in mind that this object are created and destroyed much frequently then they are actually
	///   accessed, so lazy initialization is your best friend.
	///
	/// Note this class does not have to be thread safe to use from multiple threads.
	///
	class encryptor : public booster::noncopyable {
	public:
		///
		/// Encrypt or sign the plain text \a plain together with \a timeout and return the encrypted value for a cookie.
		/// Don't forget to use base64 encoding in order to create a string that is valid for cookie
		///
		virtual std::string encrypt(std::string const &plain) = 0;
		///
		/// Decrypt the \a cipher text or check the signature and return the \a plain text and the session expiration value: \a timeout.
		///
		/// If signature checks or decryption failed return false.
		///	
		virtual bool decrypt(std::string const &cipher,std::string &plain) = 0;
		///
		/// Destructor
		///
		virtual ~encryptor() {}
	};

	///
	/// \brief This is an interface for an object that creates new encryptors 
	///
	/// This class must be thread safe.
	///
	class encryptor_factory {
	public:
		///
		/// Return a pointer to a newly created encryptor.
		///
		virtual std::auto_ptr<encryptor> get() = 0;

		///
		/// Destructor - cleanup everything
		///
		virtual ~encryptor_factory() {}
	};

	///
	/// \brief The implementation of session_api using encrypted or signed cookies
	///
	class CPPCMS_API session_cookies : public session_api {
	public:
		///
		/// Create a new object passing it a pointer ecryptor as parameter
		///
		session_cookies(std::auto_ptr<encryptor> encryptor);
		///
		/// Destroy it and destroy an encryptor it was created with
		///
		~session_cookies();
		
		/// 
		/// Save session to cookies, see session_api::save
		///
		virtual void save(session_interface &,std::string const &data,time_t timeout,bool newone ,bool on_server);
		
		/// 
		/// Load session from cookies, see session_api::load
		///
		virtual bool load(session_interface &,std::string &data,time_t &timeout);
		
		///
		/// See session_api::is_blocking
		///
		virtual bool is_blocking();
		
		/// 
		/// Delete session, see session_api::clear
		///
		virtual void clear(session_interface &);
	private:
		struct _data;
		booster::hold_ptr<_data> d;
		std::auto_ptr<encryptor> encryptor_;
	};
} // sessions
} // cppcms

#endif
