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
	
	class CPPCMS_API session_pool: public booster::noncopyable {
	public:
		session_pool(service &srv);
		~session_pool();

		void init();

		booster::shared_ptr<session_api> get();

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

		booster::hold_ptr<data> d;

		std::auto_ptr<session_api_factory> backend_;
		std::auto_ptr<sessions::encryptor_factory> encryptor_;
		std::auto_ptr<sessions::session_storage_factory> storage_; 

		service *service_;
	};
}


#endif
