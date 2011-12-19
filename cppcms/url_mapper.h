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
#ifndef CPPCMS_URL_MAPPER_H
#define CPPCMS_URL_MAPPER_H

#include <cppcms/defs.h>
#include <cppcms/string_key.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>
#include <cppcms/filters.h>
#include <string>
#include <vector>


namespace cppcms {
	class application;

	///
	/// \brief class for mapping URLs - the opposite of dispatch
	///
	/// This class is useful for mapping between different page identifications
	/// that represent different classes/view and the URL.
	///
	///
	/// The URL mapping is done in hierarchy of applications where each application
	/// has its own name and they bundler into single hierarchy. Each application in hierarchy
	/// can be referred by its key and location. It may have different "urls" for mapping
	/// different values.
	///
	/// For example, we develop a content management system with following tools:
	///
	/// - site
	///     - news 
	///         - article 
	///         - category 
	///     - forums
	///         - topics
	///         - threads
	///     - users
	///         - management
	///         - registration
	///         - profile
	///
	/// Each node above is represented my cppcms::application and its children mounted
	/// to it. In order to access each URL we use "file system" like convention:
	///
	///  - "/" - the default site page
	///  - "/news" - the default news page
	///  - "/news/article" - the default article page
	///  - "/news/category/by_date" - the categories pages sorted by data
	///  - "/news/category/by_interest" - the categories pages sorted by interest
	///  - "/news/category" - the default category pages
	///
	/// And so on.
	///
	/// Each application can be referred by its full path from root or by its relative path,
	/// so if we for example in article sub-application and we want to refer to "category"
	/// URL we would use something like map(out(),"../category/by_interest",category_id);
	///
	/// In order to all this process work, each application should mount its children by some name,
	/// like:
	///
	/// \code
	///     site::site(cppcms::service &s) : 
	///           cppcms::application(s),
	///           news_(s),
	///           forums_(s),
	///           users_(s)
	///     {
	///         add(news_);
	///         mapper().mount("news","/news{1}",news_);
	///         add(forums_);
	///         mapper().mount("forums","/forums{1}",forums_);
	///         ...
	/// \endcode
	///
	/// You can also use cppcms::application::add and cppcms::application::attach that allows
	/// to provide mapping for url_dispatcher and url_mapper in a single command like:
	///
	/// \code
	///    add(news_,
	///        "news","/news{1}",
	///        "/news((/.*)?)",1);
	/// \endcode
	///
	/// Which effectively the same as:
	///
	/// \code
	///    add(news_);
	///    mapper().mount("news","/news{1}",news_);
	///    dispatcher().mount("/news((/.*)?)",news_,1);
	/// \endcode
	///
	/// Such system allows using "url" tag in templates system easily as"
	/// 
	/// \code
	///   <a href="<% url "/news/article" using article id %>" >...</a>
	/// \endcode
	///
	/// Each mounted application may a default URL (something like index.html)
	/// which is mapped when mounted application is referred. So for example there
	/// are may be following URLs:
	///
	/// - "/news/article" or "/news/article/" - the default URL
	/// - "/news/article/preview" - preview unpublished article URL.
	///
	/// They can be defined in article class as following:
	///
	/// \code
	///  article::article(cppcms::service &s) : cppcms::application(s)
	///  {
	///     mapper().assign("/{1}"); // the default URL
	///     dispatcher().assign("/(\\d+)",&article::display,this,1);
	///     mapper().assign("preview","/{1}/preview"); // the preview URL
	///     dispatcher().assign("/(\\d+)/preview",&article::preview,this,1);
	///  }
	/// \endcode
	///
	/// Additional supported feature is "named" parameters which are usually set to
	/// some default value using set_value, they are defined by special keywords between
	/// instead of numbers. 
	///
	/// For example assign("article", "/article/{lang}/{1}") were "lang" is special value for language
	/// defined by set_value("lang","en"), so mapping map(out(),"article",10) would create
	/// a URL "/article/en/10"
	///
	/// Sometimes it is useful to change such values there are two ways to do it:
	///
	/// -    Overloading keyword with different parameters number assign("article","/article/{1}/{2}")
	///      and then calling map(out(),"article","ru",10) for URL like "/article/ru/10".
	/// -    Using naming of parameters at mapping level by prepending comma separated keywords at the
	///      end of the path line after ";" map(out(),"article;lang","ru",10) - which would effectively work
	///      like temporary calling set_value("lang","ru") and then calling map(out(),"article",10).
	///      Unlike the previous case it also allows to do such changes globally.
	///      <br>
	///      For example if "news" application mounts article using "/{lang}/{1}" then
	///      using such mapping would affect top level URL that does not belong to specific application.
	///
	///
	///
	/// 
	///
	///
	class CPPCMS_API url_mapper : public booster::noncopyable {
	public:
		/// \cond INTERNAL
		url_mapper(application *app);
		~url_mapper();
		/// \endcond
		
		///
		/// Get the root of the application - the string that
		/// is added to the any URL patter like "/forum" or
		/// "http://my.site.com"
		///
		std::string root();
		///
		/// Set the root of the application - the string that
		/// is added to the any URL patter like "/forum" or
		/// "http://my.site.com"
		///
		void root(std::string const &r);

		///
		/// Provide a mapping between special \a key and a \a url pattern.
		///
		/// URL patter is a string that includes mapped patters between "{" and "}"
		/// brackets. For example "/page/{1}" where "{1}" is being substituted
		/// by the first parameter in map functions.
		///
		/// The ids can be numbers - 1 to 6 and special keys that can be changed
		/// in the run time using set_value functions. For example:
		///
		/// "/wiki/{lang}/page/{1}"
		///
		/// Where "lang" can be defined by "set_value". For example.
		///
		/// For the url above with "lang" set to "en" and first parameter "cppcms"
		/// the string would be "/wiki/en/page/cppcms"
		///
		/// Note the keys may be overloaded by number of parameters as for example:
		///
		/// - <tt>assign("page","/wiki/{1}/page/{2}");</tt>
		/// - <tt>assign("page","/wiki/{lang}/page/{1}");</tt>
		/// - <tt>assign("page","/wiki/{lang}/page/main");</tt>
		///
		/// Then map(output,"page") - would create "/wiki/en/page/main", 
		/// map(output,"page",134) would create "/wiki/en/page/132" and 
		/// map(output,"page","ru","cppcms") would create "/wiki/ru/page/cppcms"
		///
		/// Note: They keys containing "/", "," or ";" and keys with values "..", ".", ""  are prohibited  
		/// as they have special meanings
		///
		void assign(std::string const &key,std::string const &url);
		///
		/// Map the default key for the application, \a url has same rules as for assign(key,url) but
		/// they rather refer to default application's URL when it is used in hierarchy.
		///
		void assign(std::string const &url);

		///
		/// Set special value for a key that would be used
		/// in URL mapping, for example set_value("lang","en")
		///
		/// Note: this value is defined globally for all applications hierarchy and not only
		/// for this specific application
		///
		void set_value(std::string const &key,std::string const &value);
		///
		/// Clear the special value - reset to empty
		///
		/// Note: this value is cleared globally for all applications hierarchy and not only
		/// for this specific application
		///
		void clear_value(std::string const &key);
	
		///
		/// Write the URL to output stream \a out for the URL \a path with 0 parameters
		///	
		void map(	std::ostream &out,
				char const *path);

		///
		/// Write the URL to output stream \a out for the URL \a path with 1 parameters
		///	
		void map(	std::ostream &out,
				char const *path,
				filters::streamable const &p1);

		///
		/// Write the URL to output stream \a out for the URL \a path with 2 parameters
		///	
		void map(	std::ostream &out,
				char const *path,
				filters::streamable const &p1,
				filters::streamable const &p2);

		///
		/// Write the URL to output stream \a out for the URL \a path with 3 parameters
		///	
		void map(	std::ostream &out,
				char const *path,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3);

		///
		/// Write the URL to output stream \a out for the URL \a path with 4 parameters
		///	
		void map(	std::ostream &out,
				char const *path,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4);
		///
		/// Write the URL to output stream \a out for the URL \a path with 5 parameters
		///	
		void map(	std::ostream &out,
				char const *path,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5);
		///
		/// Write the URL to output stream \a out for the URL \a path with 6 parameters
		///	
		void map(	std::ostream &out,
				char const *path,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5,
				filters::streamable const &p6);
		///
		/// Write the URL to output stream \a out for the URL \a path with 0 parameters
		///	
		void map(	std::ostream &out,
				std::string const &path);

		///
		/// Write the URL to output stream \a out for the URL \a path with 1 parameters
		///	
		void map(	std::ostream &out,
				std::string const &path,
				filters::streamable const &p1);

		///
		/// Write the URL to output stream \a out for the URL \a path with 2 parameters
		///	
		void map(	std::ostream &out,
				std::string const &path,
				filters::streamable const &p1,
				filters::streamable const &p2);

		///
		/// Write the URL to output stream \a out for the URL \a path with 3 parameters
		///	
		void map(	std::ostream &out,
				std::string const &path,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3);

		///
		/// Write the URL to output stream \a out for the URL \a path with 4 parameters
		///	
		void map(	std::ostream &out,
				std::string const &path,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4);
		///
		/// Write the URL to output stream \a out for the URL \a path with 5 parameters
		///	
		void map(	std::ostream &out,
				std::string const &path,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5);
		///
		/// Write the URL to output stream \a out for the URL \a path with 6 parameters
		///	
		void map(	std::ostream &out,
				std::string const &path,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5,
				filters::streamable const &p6);

		///
		/// Mount sub application \a app using name \a name to \a url.
		///
		/// The URL format as in assign but it requires a single parameter {1}
		/// which would be substituted with the mapping of the URL of sub-application
		/// instead of using "root" patch
		///
		void mount(std::string const &name,std::string const &url,application &app);
		///
		/// Get a mapper of mounted application by its name
		///
		url_mapper &child(std::string const &name);

		///
		/// Get a parent mapper, if not exists throws cppcms_error
		///
		url_mapper &parent();
		///
		/// Get a topmost mapper, if have no parents returns reference to \c this.
		///
		url_mapper &topmost();

	private:
		void real_assign(std::string const &key,std::string const &url,application *child = 0);
		url_mapper &get_mapper_for_key(string_key const &key,string_key &real_key,std::vector<string_key> &direct);
		url_mapper *root_mapper();
		void real_map(	char const *key,
				filters::streamable const *const *params,
				size_t params_no,
				std::ostream &output);

		struct data;
		booster::hold_ptr<data> d;
	};

};

#endif
