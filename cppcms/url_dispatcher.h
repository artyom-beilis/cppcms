///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_URL_DISPATCHER_H
#define CPPCMS_URL_DISPATCHER_H

#include <booster/noncopyable.h>
#include <cppcms/defs.h>
#include <booster/function.h>
#include <booster/hold_ptr.h>
#include <booster/traits/enable_if.h>
#include <booster/traits/is_base_of.h>
#include <booster/traits/type_traits.h>
#include <booster/regex.h>
#include <cppcms/application.h>
#include <cppcms/steal_buf.h>
#include <string>
#include <list>

namespace cppcms {

	///
	/// Sets the content of parameter to output
	///
	/// \ver{v1_2}
	inline bool parse_url_parameter(util::const_char_istream &parameter,std::string &output)
	{
		output.assign(parameter.begin(),parameter.end());
		return true;
	}
	///
	/// Parses general value
	///
	/// \ver{v1_2}
	template<typename ParamType>
	bool parse_url_parameter(util::const_char_istream &parameter,ParamType &value)
	{
		parameter >> value;
		if(!parameter || !parameter.eof())
			return false;
		return true;
	}


	///
	/// \brief This class is used to glue between member function of application class and urls
	///
	/// This class is used in context of \a cppcms::application  with its \a url member function.
	/// It uses regular expression to bind between url and callbacks that actually process the
	/// request. It also allows mount sub-applications to the root of
	/// the primary application.
	///
	/// There are two families of functions, assign and map.
	
	/// Older `assign` interface allows matching only URL against regular expression.
	/// and passes std::string as parameter. All the validation must be
	/// performed by regular expression or the code that was called.
	///
	/// Newer `map` interface allows both matching an URL and an HTTP request method.
	/// Parameters are parsed using `bool parse_url_parameter(util::const_char_istream &parameter,Type &param)`
	/// that by default uses std::istream to perform casting.
	///
	/// Additionally every matched parameter is checked to contain valid text encoding
	///
	/// Newer API uses map member functions family that was introduced in CppCMS 1.1.
	///
	/// For example:
	///
	/// \code
	/// class my_web_project : public cppcms::application {
	///	users_app users_;
	///     /* Older Handlers */
	///	void display_page(std::string)
	///      ...
	///     /* Newer CppCMS 1.1 handlers */
	///     void get_resource(int id);
	///     void update_resource(int id);
	///     void new_resource();
	///     void id_by_name(std::string const &name); /* name checked for valid encoding */
	///	void display_page_by_id(int id)
	/// public:
	///    my_web_project() {
	///	/* Older API */
	///	dispatcher().assign("/page/(\\d+)/?",&my_web_project::display_page,this,1);
	///
	///     /* New API - CppCMS 1.1 and above */
	///
	///     dispatcher().map("GET","/resource/(\\d+)",&my_web_project::get_resource,this,1);
	///     dispatcher().map("PUT","/resource/(\\d+)",&my_web_project::update_resource,this,1);
	///     dispatcher().map("POST","/resources",&my_web_project::new_resource,this);
	///     dispatcher().map("GET"."/id_by_name/(.*)",&my_web_project::id_by_name,this,1);
	///     dispatcher().map("/page/(\\d+)",&my_web_project::display_page_by_id,this,1); /* any method */
	///	...
	/// \endcode
	///
	///
	class CPPCMS_API url_dispatcher : public booster::noncopyable {
	public:
		///
		/// \brief RESTful API Handler that validates parameters and executes a method.
		///
		/// If validation fails it should return false and thus the matching would continue to next handler
		///
		/// \ver{v1_2}
		typedef booster::function<bool(cppcms::application &,booster::cmatch const &)> generic_handler;
		// Handlers
		typedef booster::function<void()> handler;
		typedef booster::function<void(booster::cmatch const &)> rhandler;
		typedef booster::function<void(std::string)> handler1;
		typedef booster::function<void(std::string,std::string)> handler2;
		typedef booster::function<void(std::string,std::string,std::string)> handler3;
		typedef booster::function<void(std::string,std::string,std::string,std::string)> handler4;
		typedef booster::function<void(std::string,std::string,std::string,std::string,std::string)> handler5;
		typedef booster::function<void(std::string,std::string,std::string,std::string,std::string,std::string)> handler6;

		
		///
		/// Map a callback \a h to a URL matching regular expression \a re and an HTTP \a method 
		///
		/// \param method - HTTP method to match like GET, POST, note regular expression can be used as well, for example "(POST|PUT)"
		/// \param re - matched URL
		/// \param h - handler to execute
		///
		/// \ver{v1_2}
		void map_generic(std::string const &method,std::string const &re,generic_handler const &h);
		///
		/// Map a callback \a h to a URL matching regular expression \a re
		///
		/// \param re - matched URL
		/// \param h - handler to execute
		///
		/// \ver{v1_2}
		void map_generic(std::string const &re,generic_handler const &h);


		///
		/// \brief Map \a member of \a app as a URL handler that matches regualr expression \a re and HTTP method \a method
		///
		/// \param method - HTTP method to match like GET, POST, note regular expression can be used as well, for example "(POST|PUT)"
		/// \param re - matched URL
		/// \param member - member function of application \a app
		/// \param app - application that its \a member is called
		///
		/// In addition to calling \a member function it calls app->init() before call
		/// and app->clean() after the call of the C is derived from cppcms::application
		///
		/// \ver{v1_2}
		template<typename C>
		void map(std::string const &method,std::string const &re,void (C::*member)(),C *app)
		{
			map_generic(method,re,url_binder0<C>(member,app));
		}
		///
		/// \brief Map \a member of \a app as a URL handler that matches regualr expression \a re
		///
		/// \param re - matched URL
		/// \param member - member function of application \a app
		/// \param app - application that its \a member is called
		///
		/// In addition to calling \a member function it calls app->init() before call
		/// and app->clean() after the call of the C is derived from cppcms::application
		///
		/// \ver{v1_2}
		template<typename C>
		void map(std::string const &re,void (C::*member)(),C *app)
		{
			map_generic(re,url_binder0<C>(member,app));
		}
		///
		/// \brief Map \a member of \a app as a URL handler that matches regualr expression \a re and HTTP method \a method
		///
		/// \param method - HTTP method to match like GET, POST, note regular expression can be used as well, for example "(POST|PUT)"
		/// \param re - matched URL
		/// \param member - member function of application \a app
		/// \param app - application that its \a member is called
		/// \param g1 - a matched group passed as first parameter of \a member after validation and conversion using parse_url_parameter function
		///
		/// In addition to calling \a member function it calls app->init() before call
		/// and app->clean() after the call of the C is derived from cppcms::application
		///
		///		
		/// \ver{v1_2}
		template<typename C,typename P1>
		void map(std::string const &method,std::string const &re,void (C::*member)(P1),C *app,int g1)
		{
			map_generic(method,re,url_binder1<C,P1>(member,app,g1));
		}
		///
		/// \brief Map \a member of \a app as a URL handler that matches regualr expression \a re and HTTP method \a method
		///
		/// \param re - matched URL
		/// \param member - member function of application \a app
		/// \param app - application that its \a member is called
		/// \param g1 - a matched group passed as first parameter of \a member after validation and conversion using parse_url_parameter function
		///
		/// In addition to calling \a member function it calls app->init() before call
		/// and app->clean() after the call of the C is derived from cppcms::application
		///
		///		
		/// \ver{v1_2}
		template<typename C,typename P1>
		void map(std::string const &re,void (C::*member)(P1),C *app,int g1)
		{
			map_generic(re,url_binder1<C,P1>(member,app,g1));
		}

		


		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with matched results
		///
		void assign_generic(std::string const &regex,rhandler handler);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called
		///
		void assign(std::string const &regex,handler handler);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with first parameters
		/// the string that was matched at position \a exp1.
		///
		/// For example: if
		/// regular expression is "^/page/(\\d+)/(\\w+)$" , exp1=2, and the url is
		/// "/page/13/to_be_or_not", then handler would be called with "to_be_or_not"
		/// as its first parameter
		///
		void assign(std::string const &regex,handler1 handler,int exp1);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st and 2nd  parameters
		/// the string that was matched at position \a exp1 and \a exp2
		///
		void assign(std::string const &regex,handler2 handler,int exp1,int exp2);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st, 2nd and 3rd parameters
		/// the string that was matched at position \a exp1, \a exp2 and \a exp2
		///
		void assign(std::string const &regex,handler3 handler,int exp1,int exp2,int exp3);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st, 2nd, 3rd and 4th parameters
		/// the string that was matched at position \a exp1, \a exp2, \a exp3 and \a exp4
		///
		void assign(std::string const &regex,handler4 handler,int exp1,int exp2,int exp3,int exp4);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st, 2nd, 3rd, 4th and 5th parameters
		/// the string that was matched at position \a exp1, \a exp2, \a exp3, \a exp4 and \a exp5
		///
		/// \ver{v1_2}
		void assign(std::string const &regex,handler5 handler,int exp1,int exp2,int exp3,int exp4,int exp5);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st, 2nd, 3rd, 4th, 5th and 6th parameters
		/// the string that was matched at position \a exp1, \a exp2, \a exp3, \a exp4, \a exp 5 and \a exp6
		///
		/// \ver{v1_2}
		void assign(std::string const &regex,handler6 handler,int exp1,int exp2,int exp3,int exp4,int exp5,int exp6);

		///
		/// Try to find match between \a url and registered handlers and applications.
		/// If the match was found, it returns the method, how handler should bd dispatched
		/// synchronous or asynchronous, meaning the handler would be executed in thread pool
		/// or in the main non-blocking loop
		///

		bool dispatch(std::string url);

		/// \cond INTERNAL
		url_dispatcher(application *app);
		url_dispatcher();
		~url_dispatcher();
		/// \endcond

		///
		/// This template function is a shortcut to assign(regex,callback). It allows
		/// assignment of \a member function of an \a object with signature void handler()
		/// as simple as assign(expr,&bar::foo,this);
		///
		/// In addition to calling \a member function it calls object->init() before call
		/// and object->clean() after the call of the C is derived from cppcms::application
		///
		template<typename C>
		void assign(std::string const &regex,void (C::*member)(),C *object)
		{
			assign(regex,binder0<C>(member,object));
		}
		///
		/// This template function is a shortcut to assign_generic(regex,rhandler). It allows
		/// assignment of \a member function of an \a object with signature void handler(booster::cmatch const &)
		///
		/// In addition to calling \a member function it calls object->init() before call
		/// and object->clean() after the call of the C is derived from cppcms::application
		///
		/// \ver{v1_2}
		template<typename C>
		void assign_generic(std::string const &regex,void (C::*member)(booster::cmatch const &),C *object)
		{
			assign_generic(regex,rbinder<C>(member,object));
		}
		///
		/// This template function is a shortcut to assign(regex,callback,int). It allows
		/// assignment of \a member function of an \a object with signature void handler(string)
		///
		/// In addition to calling \a member function it calls object->init() before call
		/// and object->clean() after the call of the C is derived from cppcms::application
		///
		template<typename C>
		void assign(std::string const &regex,void (C::*member)(std::string),C *object,int e1)
		{
			assign(regex,binder1<C>(member,object),e1);
		}
		///
		/// This template function is a shortcut to assign(regex,callback,int,int). It allows
		/// assignment of \a member function of an \a object with signature void handler(string,string)
		///
		/// In addition to calling \a member function it calls object->init() before call
		/// and object->clean() after the call of the C is derived from cppcms::application
		///
		template<typename C>
		void assign(std::string const &regex,void (C::*member)(std::string,std::string),C *object,int e1,int e2)
		{
			assign(regex,binder2<C>(member,object),e1,e2);
		}
		template<typename C>
		///
		/// This template function is a shortcut to assign(regex,callback,int,int,int). It allows
		/// assignment of \a member function of an \a object with signature void handler(string,string,string)
		///
		/// In addition to calling \a member function it calls object->init() before call
		/// and object->clean() after the call of the C is derived from cppcms::application
		///
		void assign(std::string const &regex,void (C::*member)(std::string,std::string,std::string),C *object,int e1,int e2,int e3)
		{
			assign(regex,binder3<C>(member,object),e1,e2,e3);
		}
		///
		/// This template function is a shortcut to assign(regex,callback,int,int,int,int). It allows
		/// assignment of \a member function of an \a object with signature void handler(string,string,string,string)
		///
		/// In addition to calling \a member function it calls object->init() before call
		/// and object->clean() after the call of the C is derived from cppcms::application
		///
		template<typename C>
		void assign(std::string const &regex,void (C::*member)(std::string,std::string,std::string,std::string),C *object,int e1,int e2,int e3,int e4)
		{
			assign(regex,binder4<C>(member,object),e1,e2,e3,e4);
		}
		///
		/// This template function is a shortcut to assign(regex,callback,int,int,int,int,int). It allows
		/// assignment of \a member function of an \a object with signature void handler(string,string,string,string,string)
		///
		/// In addition to calling \a member function it calls object->init() before call
		/// and object->clean() after the call of the C is derived from cppcms::application
		///
		/// \ver{v1_2}
		template<typename C>
		void assign(std::string const &regex,void (C::*member)(std::string,std::string,std::string,std::string,std::string),C *object,int e1,int e2,int e3,int e4,int e5)
		{
			assign(regex,binder5<C>(member,object),e1,e2,e3,e4,e5);
		}
		///
		/// This template function is a shortcut to assign(regex,callback,int,int,int,int,int,int). It allows
		/// assignment of \a member function of an \a object with signature void handler(string,string,string,string,string,string)
		///
		/// In addition to calling \a member function it calls object->init() before call
		/// and object->clean() after the call of the C is derived from cppcms::application
		///
		/// \ver{v1_2}
		template<typename C>
		void assign(std::string const &regex,void (C::*member)(std::string,std::string,std::string,std::string,std::string,std::string),C *object,int e1,int e2,int e3,int e4,int e5,int e6)
		{
			assign(regex,binder6<C>(member,object),e1,e2,e3,e4,e5,e6);
		}

		/// 
		/// Mount a sub application \a app to the URL dispatcher, using regular expression match.
		///
		/// When mounted the URL is checked against match expression and then calls app.main(substr) where
		/// substr is the matched subexpression part. For example:
		///
		/// \code
		///  dispatcher().mount("/formums((/.*)?)",forums,1);
		/// \endcode
		///
		/// For example: for url /forums/page/3 it would call \c forums::main with value "/page/3"
		///
		void mount(std::string const &match,application &app,int part);

	private:

		template<typename C,typename Enable = void>
		class page_guard {
		public:
			page_guard(C * /*o*/) {}
			page_guard(C * /*o*/,std::istream &) {}
			application *app() { return 0; }
		};

		template<typename C>
		class page_guard<C,typename booster::enable_if<booster::is_base_of< cppcms::application,C> >::type > {
		public:
			page_guard(C *o) : 
				object_(o)
			{
				object_->init();
			}
			application *app() { return object_; }
			~page_guard()
			{
				object_->clear();
			}
		private:
			application *object_;
		};

		template<typename C>						
		struct binder0{
			typedef void (C::*member_type)();
			member_type member;
			C *object;
			
			binder0(member_type m,C *o) :
				member(m),
				object(o)
			{
			}
			void operator()() const
			{
				page_guard<C> guard(object);
				(object->*member)();
			}
		};
		
		template<typename C>						
		struct rbinder{
			typedef void (C::*member_type)(booster::cmatch const &);
			member_type member;
			C *object;
			
			rbinder(member_type m,C *o) :
				member(m),
				object(o)
			{
			}
			void operator()(booster::cmatch const &p1) const
			{
				page_guard<C> guard(object);
				(object->*member)(p1);
			}
		};

		template<typename C>						
		struct binder1{
			typedef void (C::*member_type)(std::string);
			member_type member;
			C *object;
			
			binder1(member_type m,C *o) :
				member(m),
				object(o)
			{
			}
			void operator()(std::string p1) const
			{
				page_guard<C> guard(object);
				(object->*member)(p1);
			}
		};

		template<typename C>						
		struct binder2{
			typedef void (C::*member_type)(std::string,std::string);
			member_type member;
			C *object;
			
			binder2(member_type m,C *o) :
				member(m),
				object(o)
			{
			}
			void operator()(std::string p1,std::string p2) const
			{
				page_guard<C> guard(object);
				(object->*member)(p1,p2);
			}
		};
		template<typename C>						
		struct binder3{
			typedef void (C::*member_type)(std::string,std::string,std::string);
			member_type member;
			C *object;
			
			binder3(member_type m,C *o) :
				member(m),
				object(o)
			{
			}
			void operator()(std::string p1,std::string p2,std::string p3) const
			{
				page_guard<C> guard(object);
				(object->*member)(p1,p2,p3);
			}
		};
		template<typename C>						
		struct binder4{
			typedef void (C::*member_type)(std::string,std::string,std::string,std::string);
			member_type member;
			C *object;
			
			binder4(member_type m,C *o) :
				member(m),
				object(o)
			{
			}
			void operator()(std::string p1,std::string p2,std::string p3,std::string p4) const
			{
				page_guard<C> guard(object);
				(object->*member)(p1,p2,p3,p4);
			}
		};

		template<typename C>						
		struct binder5{
			typedef void (C::*member_type)(std::string,std::string,std::string,std::string,std::string);
			member_type member;
			C *object;
			
			binder5(member_type m,C *o) :
				member(m),
				object(o)
			{
			}
			void operator()(std::string p1,std::string p2,std::string p3,std::string p4,std::string p5) const
			{
				page_guard<C> guard(object);
				(object->*member)(p1,p2,p3,p4,p5);
			}
		};

		static bool validate_encoding(application &app,char const *begin,char const *end);
		static void setup_stream(application &app,std::istream &s);

		template<typename C>						
		struct binder6{
			typedef void (C::*member_type)(std::string,std::string,std::string,std::string,std::string,std::string);
			member_type member;
			C *object;
			
			binder6(member_type m,C *o) :
				member(m),
				object(o)
			{
			}
			void operator()(std::string p1,std::string p2,std::string p3,std::string p4,std::string p5,std::string p6) const
			{
				page_guard<C> guard(object);
				(object->*member)(p1,p2,p3,p4,p5,p6);
			}
		};



		template<typename T>
		static bool parse(application &app,util::const_char_istream &p,booster::cmatch const &m,int group,T &v)
		{
			if(!validate_encoding(app,m[group].first,m[group].second))
				return false;
			p.range(m[group].first,m[group].second);
			return parse_url_parameter(p,v);
		}


		template<typename C>
		struct url_binder0  {
			typedef void (C::*member_type)();
			member_type member;
			C *self;
			url_binder0(member_type m,C *s) : member(m),self(s) {}
			bool operator()(application &,booster::cmatch const  &) 
			{
				page_guard<C> guard(self);
				(self->*member)();
				return true;
			}
		};
		
		template<typename C,typename P1>
		struct url_binder1  {
			typedef void (C::*member_type)(P1);
			member_type member;
			C *self;
			int g1;
			url_binder1(member_type m,C *s,int p1) : member(m),self(s),g1(p1) {}
			bool operator()(application &app,booster::cmatch const  &m) 
			{
				util::const_char_istream s;
				setup_stream(app,s);

				typename booster::remove_const_reference<P1>::type p1;
				bool res = parse(app,s,m,g1,p1);

				if(!res) return false;
				page_guard<C> guard(self);
				(self->*member)(p1);
				return true;
			}
		};
		
		template<typename C,typename P1,typename P2>
		struct url_binder2  {
			typedef void (C::*member_type)(P1,P2);
			member_type member;
			C *self;
			int g1,g2;
			url_binder2(member_type m,C *s,int p1,int p2) : member(m),self(s),g1(p1),g2(p2) {}
			bool operator()(application &app,booster::cmatch const  &m) 
			{
				util::const_char_istream s;
				setup_stream(app,s);

				typename booster::remove_const_reference<P1>::type p1;
				typename booster::remove_const_reference<P2>::type p2;
				bool res = parse(app,s,m,g1,p1) && parse(app,s,m,g2,p2);
				
				if(!res) return false;
				page_guard<C> guard(self);
				(self->*member)(p1,p2);
				return true;
			}
		};


		struct _data;
		booster::hold_ptr<_data> d;
	};

} // cppcms

#endif
