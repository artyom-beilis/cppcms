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
		application &rendering_application();
		///
		/// Set the application that renders current
		///
		/// Called automatically by application::render
		///
		void rendering_application(application &app);

		///
		/// Resets the application 
		///
		void reset_rendering_application();

	private:
		struct _data;
		booster::copy_ptr<_data> d;
		application *app_;
	};

}


#endif
