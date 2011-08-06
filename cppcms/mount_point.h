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
#ifndef CPPCMS_MOUNT_POINT_H
#define CPPCMS_MOUNT_POINT_H

#include <cppcms/defs.h>
#include <string>
#include <booster/perl_regex.h>
#include <booster/copy_ptr.h>

namespace cppcms {
	///
	/// This class represents application's mount point or the rule on which specific application
	/// is selected to process the query. It is used by applications_pool class for mounting applications,
	/// and by forwarding managers to match forwarding requests
	///
	class CPPCMS_API mount_point {
	public:
		///
		/// Type that describes what parameter should be passed to application::main(std::string) function
		///
		/// When the application selected specific string is passed for matching with application's member function
		/// or application's children. This can be CGI variable PATH_INFO or SCRIPT_NAME or their substring taken
		/// with regular expression.
		///
		/// If selection is \a match_path_info then PATH_INFO passed for matching, otherwide SCRIPT_NAME is used
		/// for matching.
		///
		/// For example if your service works with SCRIPT_NAME /app and the URL is pointing to /app/page/20
		/// such as SCRIPT_NAME is "/app" and PATH_INFO is "/page/20" then the last one will be used for dispatching
		/// by cppcms::application::main function.
		///
		/// But you may also work with "*.cgi" style URL if your application process all queries comping from
		/// "*.cgi" and you have "dummy scripts" at "/cgi-bin/users.cgi" and "/cgi-bin/app.cgi" and you want to match
		/// against SCRIPT_NAME rather then PATH_INFO you can use \a match_script_name option and the script name
		/// will be used for matching. 
		///
		typedef enum {
			match_path_info,    ///< Pass PATH_INFO to applications
			match_script_name   ///< Pass SCRIPT_NAME to applications
		} selection_type;

		///
		/// Get regular expression for HTTP_HOST CGI variable matching, if empty, no restrictions given
		///
		booster::regex host() const;
		///
		/// Get regular expression for SCRIPT_NAME CGI variable matching, if empty, no restrictions given
		///
		booster::regex script_name() const;
		///
		/// Get regular expression for PATH_INFO CGI variable matching, if empty, no restrictions given
		///
		booster::regex path_info() const;
		///
		/// Get regular expression subgroup that is passes to application for URL dispatching
		///
		int group() const;

		///
		/// Get SCRIPT_NAME/PATH_INFO selection
		///
		selection_type selection() const;

		///
		/// Set regular expression for HTTP_HOST CGI variable matching, if empty, no restrictions given
		///
		void host(booster::regex const &);
		///
		/// Set regular expression for SCRIPT_NAME CGI variable matching, if empty, no restrictions given
		///
		void script_name(booster::regex const &);
		///
		/// Set regular expression for PATH_INFO CGI variable matching, if empty, no restrictions given
		///
		void path_info(booster::regex const &);
		///
		/// Set regular expression subgroup that is passes to application for URL dispatching
		///
		void group(int);
		///
		/// Get SCRIPT_NAME/PATH_INFO selection
		///
		void selection(selection_type);

		///
		/// Match \a h - HTTP_HOST, \a s - SCRIPT_NAME, \a p - PATH_INFO against mount point and return
		/// true and selected URL path for application
		/// Otherwise return false and empty string
		/// 
		std::pair<bool,std::string> match(std::string const &h,std::string const &s,std::string const &p) const;
		///
		/// Match \a h - HTTP_HOST, \a s - SCRIPT_NAME, \a p - PATH_INFO against mount point and return
		/// true and selected URL path for application
		/// Otherwise return false and empty string
		/// 
		std::pair<bool,std::string> match(char const *h,char const *s,char const *p) const;

		///
		/// Create default mount point, it uses PATH_INFO for url-dispatching and gives no restriction on URL
		///
		mount_point();
		///
		/// Destructor
		~mount_point();
		///
		/// Copy constructor
		///
		mount_point(mount_point const &);
		///
		/// Assignment variable
		///
		mount_point const &operator=(mount_point const &);

		///
		/// Create a mount point that checks PATH_INFO only and passes matched \a group for dispatching
		///
		mount_point(std::string const &path,int group);
		///
		/// Create a mount point that checks SCRIPT_NAME, and passes PATH_INFO for dispatching
		///
		mount_point(std::string const &script);
		///
		/// Create a mount point that checks SCRIPT_NAME, PATH_INFO only and passes matched 
		/// PATH_INFO's \a group for dispatching
		///
		mount_point(std::string const &script,std::string const &path,int group);

		///
		/// Create a mount point with selection rule \a sel.
		///
		/// \param sel selection rule use SCRIPT_INFO or PATH_NAME for URL based dispatching 
		/// \param selected_part is a regular expression for matching against PATH_INFO or SCRIPT_NAME according \a sel
		/// \param group regular expression subgroup of \a selected_part for URL dispatching
		///
		mount_point(	selection_type sel,
				std::string const &selected_part,
				int group);

		///
		/// Create a mount point with selection rule \a sel.
		///
		/// \param sel -- selection rule use SCRIPT_INFO or PATH_NAME for URL based dispatching 
		/// \param non_selected_part is a regular expression for matching against PATH_INFO or SCRIPT_NAME according to
		///           opposite of \a sel, if sel is match_path_info then non_selected_part checked against SCRIPT_NAME
		///           otherwise it is checked against PATH_INFO
		///
		mount_point(	selection_type sel,
				std::string const &non_selected_part);

		///
		/// Create a mount point with selection rule \a sel.
		///
		/// \param sel -- selection rule use SCRIPT_INFO or PATH_NAME for URL based dispatching 
		/// \param non_selected_part is a regular expression for matching against PATH_INFO or SCRIPT_NAME according to
		///           opposite of \a sel, if sel is match_path_info then non_selected_part checked against SCRIPT_NAME
		///           otherwise it is checked against PATH_INFO
		/// \param selected_part is a regular expression for matching against PATH_INFO or SCRIPT_NAME according \a sel
		/// \param group regular expression subgroup of \a selected_part for URL dispatching 
		///
		mount_point(	selection_type sel,
				std::string const &non_selected_part,
				std::string const &selected_part,
				int group);

		///
		/// Create fully defined mount rule for matching against, \a http_host - HTTP_HOST, \a script - SCRIPT_NAME,
		/// \a path - PATH_INFO, and use subgroup \a group of regular expression selected with \a sel definition.
		///
		/// Note: if regular expression is empty, no checks are performed.
		///

		mount_point(	selection_type sel,
				booster::regex const &http_host,
				booster::regex const &script,
				booster::regex const &path,
				int group);

	private:
		booster::regex host_;
		booster::regex script_name_;
		booster::regex path_info_;
		int group_;
		selection_type selection_;
		struct _data;
		booster::copy_ptr<_data> d;
	};

} // cppcms

#endif


