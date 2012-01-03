///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_SESSION_POOL_H
#define CPPCMS_SESSION_POOL_H

#include <cppcms/defs.h>
#include <booster/shared_ptr.h>
#include <booster/hold_ptr.h>
#include <cppcms/session_api.h>

#include <memory>

namespace cppcms {
	class service;

	namespace sessions {
		class encryptor_factory;
		class session_storage_factory;
	};

	///
	/// \brief This class provides an access to session management backends an allow customization.
	///
	/// When user implements its own session_api, sessions::encryptor or sessions::session_storage 
	/// interfaces it may set their factories to these classes
	///	
	class CPPCMS_API session_pool: public booster::noncopyable {
	public:
		/// \cond INTERNAL
		session_pool(service &srv);
		~session_pool();

		void init();

		booster::shared_ptr<session_api> get();
		/// \endcond

		///
		/// Assign your own implementation of session_api passing pointer to session_api_factory.
		///
		void backend(std::auto_ptr<session_api_factory> b);
		///
		/// Assign your own implementation of sessions::encryptor that would be used for client side session
		/// management by passing pointer to sessions::encryptor_factory
		///
		void encryptor(std::auto_ptr<sessions::encryptor_factory> e);
		///
		/// Assign your own implementation of sessions::session_storage that would be used for server side session
		/// management by passing pointer to sessions::session_storage_factory
		///
		void storage(std::auto_ptr<sessions::session_storage_factory> s);
	private:

		void after_fork();
		
		struct cookies_factory;
		struct dual_factory;
		struct sid_factory;
		class gc_job; 
		template<typename Encryptor>
		struct enc_factory;
		template<typename Encryptor>
		struct enc_factory_param;
		
		struct _data;

		friend struct cookies_factory;
		friend struct dual_factory;
		friend struct sid_factory;
		friend class gc_job;

		booster::hold_ptr<_data> d;

		std::auto_ptr<session_api_factory> backend_;
		std::auto_ptr<sessions::encryptor_factory> encryptor_;
		std::auto_ptr<sessions::session_storage_factory> storage_; 

		service *service_;
	};
}


#endif
