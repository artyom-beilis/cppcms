///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/url_dispatcher.h>
#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_context.h>
#include <cppcms/encoding.h>

#include <booster/regex.h>
#include <booster/shared_ptr.h>

namespace cppcms {

	namespace /* anon */ {
		struct option : public booster::noncopyable {
			option(booster::regex const &expr) :
				expr_(expr),
				match_method_(0)
			{
			}
			option(booster::regex const &expr,std::string const &method) :
				expr_(expr),
				match_method_(1),
				mexpr_(method),
				method_(method)
			{
				for(size_t i=0;i<method.size();i++) {
					// common methods
					char c=method[i];
					if(!('A'<= c && c<='Z')) {
						match_method_ =2;
						break;
					}
				}
			}
			virtual ~option()
			{
			}

			bool matches(std::string const &path,char const *method)
			{
				if(match_method_==1) {
					if(!method || method_ != method)
						return false;
				}
				else if(match_method_ == 2) {
					if(!method || !booster::regex_match(method,mexpr_))
						return false;
				}
				
				return booster::regex_match(path.c_str(),match_,expr_);
			}

			virtual bool dispatch(std::string const &url,char const *method,application *app) = 0;
		protected:
			booster::regex expr_;
			booster::cmatch match_;
			int match_method_;
			booster::regex mexpr_;
			std::string method_;
		};

		struct mounted : public option {
			mounted(std::string expr,int select,application *app) :
				option(expr),
				app_(app),
				select_(select)
			{
			}

			virtual bool dispatch(std::string const &url,char const *method,application *)
			{
				if(matches(url,method)) {
					app_->main(match_[select_]);
					return true;
				}
				return false;
			}
		private:
			application *app_;
			int select_;
		};

		template<typename H>
		struct base_handler : public option {
			base_handler(std::string expr,H handle,int a=0,int b=0,int c=0,int d=0,int e=0,int f=0)
				: option(expr),handle_(handle)
			{
				select_[0]=a;
				select_[1]=b;
				select_[2]=c;
				select_[3]=d;
				select_[4]=e;
				select_[5]=f;
			}
			virtual bool dispatch(std::string const &url,char const *method,application *)
			{
				if(matches(url,method)) {
					execute_handler(handle_);
					return true;
				}
				return false;
			}
		private:
			void execute_handler(url_dispatcher::handler const &h)
			{
				h();
			}

			void execute_handler(url_dispatcher::rhandler const &h)
			{
				h(match_);
			}

			void execute_handler(url_dispatcher::handler1 const &h)
			{
				h(match_[select_[0]]);
			}

			void execute_handler(url_dispatcher::handler2 const &h)
			{
				h(match_[select_[0]],match_[select_[1]]);
			}
			void execute_handler(url_dispatcher::handler3 const &h)
			{
				h(match_[select_[0]],match_[select_[1]],match_[select_[2]]);
			}
			void execute_handler(url_dispatcher::handler4 const &h)
			{
				h(match_[select_[0]],match_[select_[1]],match_[select_[2]],match_[select_[3]]);
			}
			void execute_handler(url_dispatcher::handler5 const &h)
			{
				h(match_[select_[0]],match_[select_[1]],match_[select_[2]],match_[select_[3]],match_[select_[4]]);
			}
			void execute_handler(url_dispatcher::handler6 const &h)
			{
				h(match_[select_[0]],match_[select_[1]],match_[select_[2]],match_[select_[3]],match_[select_[4]],match_[select_[5]]);
			}

			int select_[6];
			H handle_;
		};

		struct generic_option : public option {
			generic_option(booster::regex const &r,url_dispatcher::generic_handler const &h) : 
				option(r),
				handle_(h)
			{
			}
			generic_option(std::string method,booster::regex const &r,url_dispatcher::generic_handler const &h) :
				option(r,method),
				handle_(h)
			{
			}
			virtual bool dispatch(std::string const &url,char const *method,application *app)
			{
				if(!app)
					return false;
				if(matches(url,method)) {
					return handle_(*app,match_);
				}
				return false;
			}
			url_dispatcher::generic_handler handle_;
		};

		template<typename H>
		booster::shared_ptr<option> make_handler(std::string expr,H const &handler,int a=0,int b=0,int c=0,int d=0,int e=0,int f=0)
		{
			return booster::shared_ptr<option>(new base_handler<H>(expr,handler,a,b,c,d,e,f));
		}

	} // anonynoys


	struct url_dispatcher::_data {
		_data(application *a) : app(a) {}
		application *app;
		std::vector<booster::shared_ptr<option> > options;
	};
	
	url_dispatcher::url_dispatcher() :
			d(new url_dispatcher::_data(0))
	{
	}
	url_dispatcher::url_dispatcher(application *app) :
			d(new url_dispatcher::_data(app))
	{
	}
	url_dispatcher::~url_dispatcher()
	{
	}

	bool url_dispatcher::dispatch(std::string url)
	{
		unsigned i;
		std::string method;
		char const *cmethod = 0;
		application *app = d->app;
		if(app && app->has_context()) {
			method = app->request().request_method();
			cmethod = method.c_str();
		}
		else {
			app = 0;
		}
		for(i=0;i<d->options.size();i++) {
			if(d->options[i]->dispatch(url,cmethod,app))
				return true;
		}
		return false;
	}

	void url_dispatcher::mount(std::string const &match,application &app,int select)
	{
		d->options.push_back(booster::shared_ptr<option>(new mounted(match,select,&app)));
	}



	void url_dispatcher::assign(std::string const &expr,handler h)
	{
		d->options.push_back(make_handler(expr,h));
	}
	
	void url_dispatcher::assign_generic(std::string const &expr,rhandler h)
	{
		d->options.push_back(make_handler(expr,h));
	}
	
	void url_dispatcher::map_generic(std::string const &method,booster::regex const &re,generic_handler const &h)
	{
		booster::shared_ptr<option> opt(new generic_option(method,re,h));
		d->options.push_back(opt);
	}
	void url_dispatcher::map_generic(booster::regex const &re,generic_handler const &h)
	{
		booster::shared_ptr<option> opt(new generic_option(re,h));
		d->options.push_back(opt);
	}

	void url_dispatcher::assign(std::string const &expr,handler1 h,int p1)
	{
		d->options.push_back(make_handler(expr,h,p1));
	}

	void url_dispatcher::assign(std::string const &expr,handler2 h,int p1,int p2)
	{
		d->options.push_back(make_handler(expr,h,p1,p2));
	}

	void url_dispatcher::assign(std::string const &expr,handler3 h,int p1,int p2,int p3)
	{
		d->options.push_back(make_handler(expr,h,p1,p2,p3));
	}

	void url_dispatcher::assign(std::string const &expr,handler4 h,int p1,int p2,int p3,int p4)
	{
		d->options.push_back(make_handler(expr,h,p1,p2,p3,p4));
	}

	void url_dispatcher::assign(std::string const &expr,handler5 h,int p1,int p2,int p3,int p4,int p5)
	{
		d->options.push_back(make_handler(expr,h,p1,p2,p3,p4,p5));
	}

	void url_dispatcher::assign(std::string const &expr,handler6 h,int p1,int p2,int p3,int p4,int p5,int p6)
	{
		d->options.push_back(make_handler(expr,h,p1,p2,p3,p4,p5,p6));
	}
	bool url_dispatcher::validate_encoding(application &app,char const *begin,char const *end)
	{
		size_t unused;
		return encoding::valid(app.context().locale(),begin,end,unused);
	}
	void url_dispatcher::setup_stream(application &app,std::istream &s)
	{
		s.imbue(app.context().locale());
	}

} // namespace cppcms
