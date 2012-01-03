///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_BASE_CONTENT_H
#define CPPCMS_BASE_CONTENT_H

#include <cppcms/defs.h>
#include <booster/copy_ptr.h>

namespace cppcms {

	class application;

	///
	/// \brief This is a simple polymorphic class that every content for templates rendering should be derided from it.
	/// It does not carry much information with exception of RTTI that allows type-safe casting of user provided
	/// content instances to target content class that is used by specific template.
	///
	class CPPCMS_API base_content {
	public:

		base_content();
		base_content(base_content const &);
		base_content const &operator=(base_content const &);
		virtual ~base_content();

		///
		/// Get the application that renders current
		/// content, throw cppcms_error if the application was not set
		///
		application &app();
		///
		/// Set the application that renders current
		///
		/// Called automatically by application::render
		///
		void app(application &app);

		///
		/// Resets the application 
		///
		void reset_app();
		///
		/// Returns true of the application is assigned
		///
		bool has_app();

		///
		/// \brief Special guard class that allows setting and resetting
		/// content's rendeding according to the specific scope
		///
		class app_guard {
			app_guard(app_guard const &);
			void operator=(app_guard const &);
		public:
			///
			/// Initialize set the applicaton \a to a content \a c if have
			/// not one ready 
			///
			app_guard(base_content &c,application &a) : p_(0)
			{
				if(!c.has_app()) {
					p_=&c;
					c.app(a);
				}
			}
			///
			/// Assign the application to \a c from the \a parent's application.
			///
			/// It is assigned if it is not already defined
			///
			app_guard(base_content &c,base_content &parent) : p_(0)
			{
				if(!c.has_app() && parent.has_app()) {
					p_ = &c;
					c.app(parent.app());
				}
			}
			///
			/// Reset the application if the content if it was assigned in constructor
			///
			~app_guard()
			{
				if(p_) {
					p_->reset_app();
					p_ = 0;
				}
			}
		private:
			base_content *p_;
		};

	private:
		struct _data;
		booster::copy_ptr<_data> d;
		application *app_;
	};

}


#endif
