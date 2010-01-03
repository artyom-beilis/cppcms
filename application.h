#ifndef CPPCMS_APPLICATION_H
#define CPPCMS_APPLICATION_H

#include "defs.h"
#include "noncopyable.h"
#include "atomic_counter.h"
#include "hold_ptr.h"
#include "intrusive_ptr.h"


namespace cppcms {

	class service;
	class url_dispatcher;
	class applications_pool;
	class application;
	class base_content;
	class cache_interface;
	class session_interface;

	namespace locale {
		class environment;
	}
	namespace http {
		class request;
		class response;
		class context;
	}
	namespace json {
		class value;
	}

	///
	/// Increase reference count of the p->root().
	///
	void CPPCMS_API intrusive_ptr_add_ref(application *p);
	///
	/// Decrease reference count of the p->root(). If reference count is 0, and application is asynchronous:
	/// destroy it, otherwise recycle it to the application pool for future use.
	///
	void CPPCMS_API intrusive_ptr_release(application *p);

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

	class CPPCMS_API application : public util::noncopyable {
	public:
		///
		/// Create a new application running on service \a srv, with a parent \a parent
		///
		application(cppcms::service &srv,application *parent = 0);

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
		/// functions of application class. This member function is application specific and not
		/// Connection specific. 
		///
		url_dispatcher &dispatcher();

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
		/// Register an application \a app as child. Ownership of app is not transfered to parent, however
		/// it would shared it's parent reference count.
		///
		/// All URL that match regular expression \a regex would be passed to the child for match. Matched part
		/// \a part would be used by child for matching.
		///
		/// For example:
		///
		/// \code 
		/// add(users,"^/users(.*)$",1")
		/// \endcode
		///
		/// For URL /users/moshe would pass only "/moshe" to URL dispatched of \a users object
		///
		void add(application &app,std::string regex,int part);

		///
		/// Register an application \a app as child. Ownership of app is transfered to parent 
		///
		void attach(application *app);
		///
		/// Register an application \a app as child. Ownership of app is transfered to parent 
		///
		/// All URL that match regular expression \a regex would be passed to the child for match. Matched part
		/// \a part would be used by child for matching.
		///
		/// For example:
		///
		/// \code 
		/// add(users,"^/users(.*)$",1")
		/// \endcode
		///
		/// For URL /users/moshe would pass only "/moshe" to URL dispatched of \a users object
		///
		void attach(application *app,std::string regex,int part);

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
		intrusive_ptr<http::context> release_context();

		///
		/// Get reference counted pointer to the http::context
		///
		intrusive_ptr<http::context> get_context();

		///
		/// Set context to the application. The application gets shared ownership on the context.
		///
		/// Note: because application hierarchy shared same context, it affects all classes in it.
		///
		void assign_context(intrusive_ptr<http::context> conn);

		///
		/// Returns true if current application was created as asynchronous application.
		///
		bool is_asynchronous();
		long refs();

	private:

		void recycle();
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


