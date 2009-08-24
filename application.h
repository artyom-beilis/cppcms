#ifndef CPPCMS_APPLICATION_H
#define CPPCMS_APPLICATION_H

#include "defs.h"
#include "noncopyable.h"
#include "atomic_counter.h"
#include "hold_ptr.h"
#include "intrusive_ptr.h"


namespace cppcms {

	class service;
	class cppcms_config;
	class url_dispatcher;
	class applications_pool;
	class application;

	namespace locale {
		class environment;
	}
	namespace http {
		class request;
		class response;
		class context;
	}

	void CPPCMS_API intrusive_ptr_add_ref(application *p);
	void CPPCMS_API intrusive_ptr_release(application *p);

	class CPPCMS_API application : public util::noncopyable {
	public:
		application(cppcms::service &srv,application *parent = 0);
		virtual ~application();

		cppcms::service &service();
		cppcms_config const &settings();
		http::context &context();
		http::request &request();
		http::response &response();
		url_dispatcher &dispatcher();
		locale::environment &locale();

		char const *gt(char const *s);
		char const *ngt(char const *s,char const *p,int n);

		void assign_context(intrusive_ptr<http::context> conn);

		void add(application &app);
		void add(application &app,std::string regex,int part);

		void assign(application *app);
		void assign(application *app,std::string regex,int part);

		application *parent();
		application *root();

	private:

		void release_all_contexts();
		void parent(application *parent);

		void pool_id(int id);
		int pool_id();


		struct data; // future use
		util::hold_ptr<data> d;

		application *parent_;
		application *root_;

		atomic_counter refs_;
		friend class applications_pool;
		friend void intrusive_ptr_add_ref(application *p);
		friend void intrusive_ptr_release(application *p);
	};

} // cppcms

#endif


