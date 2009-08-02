#ifndef CPPCMS_APPLICATION_H
#define CPPCMS_APPLICATION_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"

namespace cppcms {

	class service;
	class cppcms_config;
	class context;
	class url_dispatcher;
	class applications_pool;

	namespace http {
		class request;
		class response;
	}

	class CPPCMS_API application : public util::noncopyable {
	public:
		application(cppcms::service &srv);
		~application();

		cppcms::service &service();
		cppcms_config const &config();
		cppcms::context &context();
		http::request &request();
		http::response &response();
		url_dispatcher &dispatcher();
	private:
		struct data;
		util::hold_ptr<data> d;

		void pool_id(int id);
		int pool_id();
		friend class applications_pool;
	};

} // cppcms

#endif


