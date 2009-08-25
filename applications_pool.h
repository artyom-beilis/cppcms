#ifndef CPPCMS_APPLICATIONS_POOL_H
#define CPPCMS_APPLICATIONS_POOL_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"
#include "intrusive_ptr.h"

#include <memory>
#include <string>

namespace cppcms {

	class application;
	class service;

	class CPPCMS_API applications_pool {
	public:

		struct factory : public util::noncopyable {
			virtual std::auto_ptr<application> operator()(service &) const = 0;
			virtual ~factory(){}
		};

		void mount(std::auto_ptr<factory> aps);
		void mount(std::auto_ptr<factory> aps,std::string path_info,int select);
		void mount(std::auto_ptr<factory> aps,std::string script_name);
		void mount(std::auto_ptr<factory> aps,std::string script_name,std::string path_info, int select);

		void mount(intrusive_ptr<application> app);
		void mount(intrusive_ptr<application> app,std::string path_info,int select);
		void mount(intrusive_ptr<application> app,std::string script_name);
		void mount(intrusive_ptr<application> app,std::string script_name,std::string path_info, int select);

		intrusive_ptr<application> get(std::string script_name,std::string path_info,std::string &match);
		void put(application *app);

		applications_pool(service &srv,int pool_size_limit);
		~applications_pool();

	private:
		struct basic_app_data;
		struct app_data;
		struct long_running_app_data;
		struct data;
		std::string script_name();
		bool matched(basic_app_data &data,std::string script_name,std::string path_info,std::string &matched);
		service *srv_;
		util::hold_ptr<data> d;
	};

	namespace details {
		template<typename T>
		struct simple_factory0 : public applications_pool::factory
		{
			std::auto_ptr<application> operator()(service &s) const
			{
				std::auto_ptr<application> app(new T(s));
				return app;
			}
		};
		template<typename T,typename P1>
		struct simple_factory1 : public applications_pool::factory
		{
			simple_factory1(P1 p1) : p1_(p1) {}
			P1 p1_;
			std::auto_ptr<application> operator()(service &s) const
			{
				std::auto_ptr<application> app(new T(s,p1_));
				return app;
			}
		};
		template<typename T,typename P1,typename P2>
		struct simple_factory2 : public applications_pool::factory 
		{
			simple_factory2(P1 p1,P2 p2) : p1_(p1),p2_(p2) {}
			P1 p1_;
			P2 p2_;
			std::auto_ptr<application> operator()(service &s) const
			{
				std::auto_ptr<application> app(new T(s,p1_,p2_));
				return app;
			}
		};
	} // details

	template<typename T>
	std::auto_ptr<applications_pool::factory> applications_factory()
	{
		std::auto_ptr<applications_pool::factory> f(new details::simple_factory0<T>);
		return f;
	}
	
	template<typename T,typename P1>
	std::auto_ptr<applications_pool::factory> applications_factory(P1 p1)
	{
		std::auto_ptr<applications_pool::factory> f(new details::simple_factory1<T,P1>(p1));
		return f;
	}
	
	template<typename T,typename P1,typename P2>
	std::auto_ptr<applications_pool::factory> applications_factory(P1 p1,P2 p2)
	{
		std::auto_ptr<applications_pool::factory> f(new details::simple_factory2<T,P1,P2>(p1,p2));
		return f;
	}


} // cppcms



#endif
