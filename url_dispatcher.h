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
#ifndef CPPCMS_URL_DISPATCHER_H
#define CPPCMS_URL_DISPATCHER_H

#include <booster/noncopyable.h>
#include "defs.h"
#include <booster/function.h>
#include <booster/hold_ptr.h>
#include "mem_bind.h"
#include <string>
#include <list>

namespace cppcms {

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
	///	url().assign("^/page/(\\d+)/?$",bind1st(mem_fun(&my_web_project::display_page),this),1);
	///	...
	/// \endcode
	///

	class CPPCMS_API url_dispatcher : public booster::noncopyable {
	public:
		// Handlers
		typedef booster::function<void()> handler;
		typedef booster::function<void(std::string)> handler1;
		typedef booster::function<void(std::string,std::string)> handler2;
		typedef booster::function<void(std::string,std::string,std::string)> handler3;
		typedef booster::function<void(std::string,std::string,std::string,std::string)> handler4;

		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called
		///
		void assign(std::string regex,handler handler);
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
		void assign(std::string regex,handler1 handler,int exp1);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st and 2nd  parameters
		/// the string that was matched at position \a exp1 and \a exp2
		///
		void assign(std::string regex,handler2 handler,int exp1,int exp2);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st, 2nd and 3rd parameters
		/// the string that was matched at position \a exp1, \a exp2 and \a exp2
		///
		void assign(std::string regex,handler3 handler,int exp1,int exp2,int exp3);
		///
		/// Assign \a handler to pattern \a regex thus if URL that matches
		/// this pattern requested, \a handler is called with 1st, 2nd, 3rd and 4th parameters
		/// the string that was matched at position \a exp1, \a exp2, \a exp3 and \a exp4
		///
		void assign(std::string regex,handler4 handler,int exp1,int exp2,int exp3,int exp4);

		///
		/// Try to find match between \a url and registered handlers and applications.
		/// If the match was found, it returns the method, how handler should bd dispatched
		/// synchronous or asynchronous, meaning the handler would be executed in thread pool
		/// or in the main non-blocking loop
		///

		bool dispatch(std::string url);

		url_dispatcher();
		~url_dispatcher();

		///
		/// This template function is a shortcut to assign(regex,callback). It allows
		/// assignment of \a member function of an \a object with signature void handler()
		/// as simple as assign(expr,&bar::foo,this);
		///
		template<typename C>
		void assign(std::string regex,void (C::*member)(),C *object)
		{
			assign(regex,util::mem_bind(member,object));
		}
		///
		/// This template function is a shortcut to assign(regex,callback,int). It allows
		/// assignment of \a member function of an \a object with signature void handler(string)
		///
		template<typename C>
		void assign(std::string regex,void (C::*member)(std::string),C *object,int e1)
		{
			assign(regex,util::mem_bind(member,object),e1);
		}
		///
		/// This template function is a shortcut to assign(regex,callback,int,int). It allows
		/// assignment of \a member function of an \a object with signature void handler(string,string)
		///
		template<typename C>
		void assign(std::string regex,void (C::*member)(std::string,std::string),C *object,int e1,int e2)
		{
			assign(regex,util::mem_bind(member,object),e1,e2);
		}
		template<typename C>
		///
		/// This template function is a shortcut to assign(regex,callback,int,int,int). It allows
		/// assignment of \a member function of an \a object with signature void handler(string,string,string)
		///
		void assign(std::string regex,void (C::*member)(std::string,std::string,std::string),C *object,int e1,int e2,int e3)
		{
			assign(regex,util::mem_bind(member,object),e1,e2,e3);
		}
		///
		/// This template function is a shortcut to assign(regex,callback,int,int,int,int). It allows
		/// assignment of \a member function of an \a object with signature void handler(string,string,string,string)
		///
		template<typename C>
		void assign(std::string regex,void (C::*member)(std::string,std::string,std::string,std::string),C *object,int e1,int e2,int e3,int e4)
		{
			assign(regex,util::mem_bind(member,object),e1,e2,e3,e4);
		}

	private:
		
		friend class application;
		void mount(std::string match,application &app,int part);

		struct data;
		booster::hold_ptr<data> d;
	};

} // cppcms

#endif
