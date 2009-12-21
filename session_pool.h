#ifndef CPPCMS_SESSION_POOL_H
#define CPPCMS_SESSION_POOL_H

#include "defs.h"
#include "intrusive_ptr.h"
#include "hold_ptr.h"
#include "session_api.h"

#include <memory>

namespace cppcms {
	class service;

	namespace sessions {
		class encryptor_factory;
		class session_storage_factory;
	};
	
	class CPPCMS_API session_pool: public util::noncopyable {
	public:
		session_pool(service &srv);
		~session_pool();

		void init();

		intrusive_ptr<session_api> get();

		void backend(std::auto_ptr<session_api_factory> b);
		void encryptor(std::auto_ptr<sessions::encryptor_factory> e);
		void storage(std::auto_ptr<sessions::session_storage_factory> s);
	private:

		void after_fork();
		
		struct cookies_factory;
		struct dual_factory;
		struct sid_factory;
		class gc_job; 
		template<typename Encryptor>
		struct enc_factory;
		
		struct data;

		friend struct cookies_factory;
		friend struct dual_factory;
		friend struct sid_factory;
		friend class gc_job;

		util::hold_ptr<data> d;

		std::auto_ptr<session_api_factory> backend_;
		std::auto_ptr<sessions::encryptor_factory> encryptor_;
		std::auto_ptr<sessions::session_storage_factory> storage_; 

		service *service_;
	};
}


#endif
