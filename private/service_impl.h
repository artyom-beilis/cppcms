///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_SERVICE_IMPL_H
#define CPPCMS_SERVICE_IMPL_H

#include <cppcms/json.h>
#include <cppcms/localization.h>
#include <booster/aio/io_service.h>
#include <booster/aio/stream_socket.h>
#include <booster/shared_ptr.h>
#include <booster/auto_ptr_inc.h>


namespace cppcms {
class service;
class applications_pool;
class application;
class thread_pool;
class session_pool;
namespace plugin { class scope; }
namespace impl {
	struct cached_settings;
	class prefork_acceptor;
	namespace cgi {
		class acceptor;
	}

	class service : public booster::noncopyable {
	public:
		service();
		~service();
		booster::aio::io_service &get_io_service()
		{
			return *io_service_;
		}


	private:
		friend class cppcms::service;
		std::unique_ptr<booster::aio::io_service> io_service_;

		std::vector<booster::shared_ptr<cgi::acceptor> > acceptors_;
		#ifndef CPPCMS_WIN32
		std::unique_ptr<prefork_acceptor> prefork_acceptor_;
		#endif
		std::unique_ptr<json::value> settings_;
		std::unique_ptr<applications_pool> applications_pool_;
		std::unique_ptr<thread_pool> thread_pool_;
		std::unique_ptr<locale::generator> locale_generator_;
		std::unique_ptr<views::manager> views_pool_;
		std::unique_ptr<cache_pool> cache_pool_;
		std::unique_ptr<session_pool> session_pool_;
		std::unique_ptr<cppcms::forwarder> forwarder_;
		std::unique_ptr<impl::cached_settings> cached_settings_;
		std::locale default_locale_;

		std::vector<booster::function<void()> > on_fork_;

		int id_;

		booster::aio::native_type notification_socket_;
		std::unique_ptr<booster::aio::stream_socket> sig_,breaker_;
		
		std::vector<std::string> args_;

		booster::hold_ptr<plugin::scope> plugins_;
	};


}
} // cppcms



#endif
