#ifndef CPPCMS_URL_DISPATCHER_H
#define CPPCMS_URL_DISPATCHER_H

#include "noncopyable.h"
#include "defs.h"
#include "callback.h"
#include "hold_ptr.h"
#include <string>
#include <list>

namespace cppcms {
	namespace util {
		class regex;
	}

	class application;

	///
	/// \brief This class is used to glue between member function of application class and urls
	///
	/// This class is used in context of \a cppcms::application  with its \a url member function.
	/// It uses regular expression to bind between url and callbacks that actually process the
	/// request. It also allows mount sub-applications to the root of 
	/// the primary application. For example:
	///
	/// \code
	/// class my_web_project : public cppcms::application {
	///	users_app users_;
	///	void display_page(std::string)
	///      ...
	/// public:
	///    my_web_project() {
	///	static const util::regex rusers("^/users.*$");
	///     url().mount(rusers,users_);
	///	static const util::regex rpage("^/page/(\\d+)/?$");
	///	url().assign(rpage,bind1st(mem_fun(&my_web_project::display_page),this));
	///	...
	/// \endcode
	///

	class CPPCMS_API url_dispatcher : public util::noncopyable {
	public:
		// Handlers 
		typedef util::callback0 handler;
		typedef util::callback1<std::string> handler1;
		typedef util::callback2<std::string,std::string> handler2;
		typedef util::callback3<std::string,std::string,std::string> handler3;
		typedef util::callback4<std::string,std::string,std::string,std::string> handler4;

		///
		/// Mount application thus every request that mathces the regular
		/// expression \a match is redirected to \a app
		///
		void mount(util::regex const &match,application &app);

		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called
		///
		void assign(util::regex const &match,handler);
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
		void assign(util::regex const &match,handler1,int exp1);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st and 2nd  parameters
		/// the string that was matched at position \a exp1 and \a exp2
		///
		void assign(util::regex const &match,handler2,int exp1,int exp2);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st, 2nd and 3rd parameters
		/// the string that was matched at position \a exp1, \a exp2 and \a exp2
		///
		void assign(util::regex const &match,handler3,int exp1,int exp2,int exp3);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st, 2nd, 3rd and 4th parameters
		/// the string that was matched at position \a exp1, \a exp2, \a exp2 and \a exp3
		///
		void assign(util::regex const &match,handler4,int exp1,int exp2,int exp3,int exp4);

		///
		/// Try to find match between \a url and registered handlers and applications.
		/// If the match was found, the handler is called and \a true returned, otherwise
		/// \a false is returned.
		///
		bool dispatch(std::string url);

		url_dispatcher();
		~url_dispatcher();

		template<typename C>
		void assign(util::regex const &match,void (C::*member)(),C *object)
		{
			assign(match,mem_bind(member,object));
		}
		template<typename C>
		void assign(util::regex const &match,void (C::*member)(std::string),C *object,int e1)
		{
			assign(match,mem_bind(member,object),e1);
		}
		template<typename C>
		void assign(util::regex const &match,void (C::*member)(std::string,std::string),C *object,int e1,int e2)
		{
			assign(match,mem_bind(member,object),e1,e2);
		}
		template<typename C>
		void assign(util::regex const &match,void (C::*member)(std::string,std::string,std::string),C *object,int e1,int e2,int e3)
		{
			assign(match,mem_bind(member,object),e1,e2,e3);
		}
		template<typename C>
		void assign(util::regex const &match,void (C::*member)(std::string,std::string,std::string,std::string),C *object,int e1,int e2,int e3,int e4)
		{
			assign(match,mem_bind(member,object),e1,e2,e3,e4);
		}




	private:
		struct option;
		std::list<option> options_;
		struct data;
		util::hold_ptr<data> d;
	};

} // cppcms

#endif
