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
#ifndef CPPCMS_APPLICATION_H
#define CPPCMS_APPLICATION_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>
#include <booster/atomic_counter.h>
#include <booster/intrusive_ptr.h>
#include <booster/shared_ptr.h>
#include <string>

namespace cppcms {
	class application;
}

namespace booster {
	void CPPCMS_API intrusive_ptr_add_ref(cppcms::application *p);
	void CPPCMS_API intrusive_ptr_release(cppcms::application *p);
}


namespace cppcms {

	class service;
	class url_dispatcher;
	class url_mapper;
	class applications_pool;
	class application;
	class base_content;
	class cache_interface;
	class session_interface;

	namespace http {
		class request;
		class response;
		class context;
	}
	namespace json {
		class value;
	}
	namespace filters {
		class streamable;
	}


	///
	/// \brief application class is the base class for all user created applications.
	///
	/// This class is the base for all user actions required for web page generation.
	/// User application classes are created upon web page request and then cached in \a application_pool.
	///
	/// Applications can be bundled to hierarchies. You may add a sub application to hierarchy,
	/// and they will always be connected with topmost application and their lifetime would be binded to them.
	///
	/// application class is reference counted and may be used with \a intrusive_ptr. But reference count
	/// semantics is very different form an ordinary semantics for other classes derived from \a cppcms::refcounted.
	///
	/// 1.  All hierarchy share the counter of the topmost application. Thus, when all bundle is counted
	///     as a single unit allowing passing intrusive_ptr to itself to the central service safely.
	///     When the topmost application is destroyed, it destroys all its children application classes.
	/// 2.  When reference count goes to 0, the application is not destroyed but rather recycled to the 
	///     application pool for future use.
	/// 3.  The above hold only for synchronous applications, asynchronous one are destroyed when all 
	///     reference count goes to 0.
	///
	/// There two ways to add sub-applications to hierarchy:
	///
	/// 1.  Using member function family \a add, usually used with direct members of the parent class.
	///     Such child are not destroyed explicitly.
	/// 2.  Using member function family \a attach. The ownership on the application is moved to the 
	///     parent class and it destroys an attached class with delete.
	///

	class CPPCMS_API application : public booster::noncopyable {
	public:
		///
		/// Create a new application running on service \a srv, with a parent \a parent
		///
		application(cppcms::service &srv);

		///
		/// Destroys an application and all assigned application children.
		///
		virtual ~application();

		///
		/// Get the main service 
		///
		cppcms::service &service();
		
		///
		/// Get global service settings
		///
		json::value const &settings();

		///
		/// Get a context of the single HTTP request/response.
		/// 
		http::context &context();

		///
		/// Get a HTTP request information class, same as context().request();
		///
		http::request &request();

		///
		/// Get a HTTP response information class, same as context().response();
		///
		http::response &response();

		///
		/// Get a dispatched class -- class that responsible on mapping between URLs and a member
		/// functions of application class. 
		///
		/// This member function is application specific and not
		/// Connection specific. 
		///
		url_dispatcher &dispatcher();

		///
		/// Get a url_mapper class -- class that responsible on mapping between real objects and 
		/// urls displayer on the page.
		///
		/// This member function is application specific and not
		/// Connection specific. 
		///
		url_mapper &mapper();

		///
		/// Get a cache_interface instance. Same as context().cache();
		///
		cache_interface &cache();
		
		///
		/// Get current session_interface instance. Same as context().session();
		///
		session_interface &session();

		///
		/// Render a template \a template_name of default skin using content \a content.
		///
		/// Side effect requires: output stream for response class, causes all updated session
		/// data be saved and all headers be written. You can't change headers after calling this function.
		///
		void render(std::string template_name,base_content &content);
		///
		/// Render a template \a template_name of \a skin skin using content \a content.
		///
		/// Side effect requires: output stream for response class, causes all updated session
		/// data be saved and all headers be written. You can't change headers after calling this function.
		///
		void render(std::string skin,std::string template_name,base_content &content);

		///
		/// Render a template \a template_name of default skin using content \a content to an output
		/// stream \a out. Note: You are responsible to imbue suitable locale to the stream.
		///
		/// You should use context().locale() or service().generator() to create such locales.
		///
		void render(std::string template_name,std::ostream &out,base_content &content);

		///
		/// Render a template \a template_name of a skin \a skin using content \a content to an output
		/// stream \a out. Note: You are responsible to imbue suitable locale to the stream.
		///
		/// You should use context().locale() or service().generator() to create such locales.
		///
		void render(std::string skin,std::string template_name,std::ostream &out,base_content &content);

		///
		/// Register an application \a app as child. Ownership of app is not transfered to parent, however
		/// it would shared it's parent reference count.
		///
		void add(application &app);

		///
		/// Register an application \a app as child and mount it into url dispatched calling
		/// dispatcher().mount(regex,app,part);
		///
		///  Ownership of app is not transfered to parent, however
		/// it would shared it's parent reference count.
		///
		///
		void add(application &app,std::string const &regex,int part);

		///
		/// Register an application \a app as child and mount it into:
		///
		/// - url_dispatcher calling dispatcher().mount(regex,app,part);
		/// - url_mapper calling mapper().mount(name,url,app);
		///
		///  Ownership of app is not transfered to parent, however
		///  it would shared it's parent reference count.
		///
		///
		void add(application &app,std::string const &name,std::string const &url,std::string const &regex,int part);
		///
		/// Register an application \a app as child and mount it into
		/// url_mapper calling mapper().mount(name,url,app);
		///
		///  Ownership of app is not transfered to parent, however
		///  it would shared it's parent reference count.
		///
		///
		void add(application &app,std::string const &name,std::string const &url);

		///
		/// Register an application \a app as child. Ownership of app is transfered to parent 
		///
		void attach(application *app);
		///
		/// Register an application \a app as child and mount it into
		/// url_dispatcher calling dispatcher().mount(regex,*app,part);
		///
		///  Ownership of app is transfered to parent.
		///
		void attach(application *app,std::string const &regex,int part);
		///
		/// Register an application \a app as child and mount it into
		/// url_mapper calling mapper().mount(name,url,*app);
		///
		///  Ownership of app is transfered to parent.
		///
		void attach(application *app,std::string const &name,std::string const &url);
		///
		/// Register an application \a app as child and mount it into:
		///
		/// - url_dispatcher calling dispatcher().mount(regex,*app,part);
		/// - url_mapper calling mapper().mount(name,url,*app);
		///
		///  Ownership of app is transfered to parent.
		///
		void attach(application *app,std::string const &name,std::string const &url,std::string const &regex,int part);

		///
		/// Get the parent of the application, if the application is the topmost class in hierarchy,
		/// it would return \a this, So, if you want to check if the application has any parent test
		/// app->parent()!=app;
		///
		application *parent();

		///
		/// Get the root application of the hierarchy. Note, if the application is the topmost one, 
		/// \a this pointer would be returned
		///
		application *root();

		///
		/// Request from an application give-up on ownership of the http::context class and give it
		/// to the user control. Usually it is required for processing asynchronous requests.
		/// 
		/// Note: because application hierarchy shared same context, it affects all classes in it.
		///
		booster::shared_ptr<http::context> release_context();

		///
		/// Get reference counted pointer to the http::context
		///
		booster::shared_ptr<http::context> get_context();

		///
		/// Set context to the application. The application gets shared ownership on the context.
		///
		/// Note: because application hierarchy shared same context, it affects all classes in it.
		///
		void assign_context(booster::shared_ptr<http::context> conn);

		///
		/// Returns true if current application was created as asynchronous application.
		///
		bool is_asynchronous();


		///
		/// This is main function of the application that is called when it is matched
		/// according to the regular expression in the applications_pool class.
		///
		/// By default, main calls dispatcher().dispatch(url). And if the last fails, it
		/// creates 404 Error page. This allows developers to create its own hooks for 
		/// reaction on incoming URL as, initialization and cleanup of general resources, 
		/// Custom 404 and error handlers etc.
		/// 
		virtual void main(std::string url);

		///
		/// This member function called when URL is dispatched to this application, it is 
		/// useful when member functions of applications are called, by default does nothing
		///
		virtual void init();

		///
		/// This function is used for cleanup unused stuff, it should not throw
		///
		virtual void clear();

		///
		/// Translate a message in current locale for given \a message in \a context
		///
		std::string translate(char const *context,char const *message);
		///
		/// Translate a message in current locale for given \a message
		///
		std::string translate(char const *message);
		///
		/// Translate a message in current locale for given \a single and \a plural form for number \a n in \a context.
		///
		std::string translate(char const *context,char const *single,char const *plural,int n);
		///
		/// Translate a message in current locale for given \a single and \a plural form for number \a n
		///
		std::string translate(char const *single,char const *plural,int n);


		///
		/// Map url-key \a key to actual URL, without parameters 
		///
		/// Effectively it calls mapper().map(...)
		///
		std::string url(std::string const &key);

		///
		/// Map url-key \a key to actual URL, with parameter p1
		///
		/// Effectively it calls mapper().map(...)
		///
		std::string url(std::string const &key,
				filters::streamable const &p1);

		///
		/// Map url-key \a key to actual URL, with parameters p1, p2 
		///
		/// Effectively it calls mapper().map(...)
		///
		std::string url(std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2);

		///
		/// Map url-key \a key to actual URL, with parameters p1, p2, p3
		///
		/// Effectively it calls mapper().map(...)
		///
		std::string url(std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3);

		///
		/// Map url-key \a key to actual URL, with parameters p1, p2, p3, p4
		///
		/// Effectively it calls mapper().map(...)
		///
		std::string url(std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4);
		///
		/// Map url-key \a key to actual URL, with parameters p1, p2, p3, p4, p5
		///
		/// Effectively it calls mapper().map(...)
		///
		std::string url(std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5);
		///
		/// Map url-key \a key to actual URL, with parameters p1, p2, p3, p4, p5, p6
		///
		/// Effectively it calls mapper().map(...)
		///
		std::string url(std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5,
				filters::streamable const &p6);

	private:

		void recycle();
		void parent(application *parent);

		void pool_id(int id);
		int pool_id();


		struct _data; // future use
		booster::hold_ptr<_data> d;

		application *parent_;
		application *root_;

		booster::atomic_counter refs_;
		friend class applications_pool;
		friend void booster::intrusive_ptr_add_ref(application *p);
		friend void booster::intrusive_ptr_release(application *p);
	};

} // cppcms

#endif


