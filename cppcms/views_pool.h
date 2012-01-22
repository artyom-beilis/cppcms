///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_VIEWS_POOL_H
#define CPPCMS_VIEWS_POOL_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <cppcms/base_view.h>
#include <cppcms/cppcms_error.h>

#include <memory>
#include <map>
#include <vector>
#include <ostream>

namespace cppcms {

	namespace json { class value; }

	///
	/// \brief this namespace holds all classes used for rendering CppCMS views.
	///
	namespace views {

		///
		/// \brief The class that represents a single skin and generates its views.
		///
		/// Usually used by templates compiler
		///
		class CPPCMS_API generator : public booster::noncopyable {
		public:
			/// The callback that creates a single view
			typedef std::auto_ptr<base_view> view_factory_type(std::ostream &,base_content *c);

			generator();
			~generator();

			///
			/// Add a single view of type \a View that uses content of type \a Content
			/// Using name \a view_name.
			///
			/// If \a safe is true that dynamic cast is used to ensure that content has proper type
			/// otherwise static cast.
			///
			/// Usually used by templates generator
			///
			template<typename View,typename Content>
			void add_view(std::string const &view_name,bool safe = true)
			{
				view_factory_type *factory = 0;
				if(safe)
					factory = view_builder<View,Content>;
				else
					factory = unsafe_view_builder<View,Content>;
				add_factory(view_name,factory);
			}
			
			///
			/// Add a view that uses a callback
			///	
			void add_factory(std::string const &name,view_factory_type *factory);
			///	
			/// Get skin name
			///
			std::string name() const;
			///
			/// Set skin name
			///
			void name(std::string const &n);
			///
			/// Create a view by its name that writes that data to \a outout using
			/// a content \a content.
			///
			std::auto_ptr<base_view> create(std::string const &view_name,
							std::ostream &output,
							base_content *content) const;
		private:
			
			template<typename View,typename Content>
			static std::auto_ptr<base_view> view_builder(std::ostream &stream,base_content *c) 
			{
				std::auto_ptr<base_view> p;
				
				try {
					p.reset(new View(stream,dynamic_cast<Content &>(*c)));
				}
				catch(std::bad_cast const &) {
					throw cppcms_error("cppcms::views::generator: an attempt to use content if invalid type");
				}
				return p;
			}
			
			template<typename View,typename Content>
			static std::auto_ptr<base_view> unsafe_view_builder(std::ostream &stream,base_content *c) 
			{
				std::auto_ptr<base_view> p(new View(stream,static_cast<Content &>(*c)));
				return p;
			}
			

			struct data;
			typedef std::map<std::string,view_factory_type *> views_type;
			views_type views_;
			std::string name_;
			booster::hold_ptr<data> d;
		};

		///
		/// \brief This is a singleton object that holds all views in the process. Any view
		/// is registered and unregistered via this object.
		///
		/// It is usually not used directly
		///
		class CPPCMS_API pool : public booster::noncopyable {
		public:
			///
			/// Add new skin to pool
			///
			/// This function is thread safe
			///
			void add(generator const &generator);
			///
			/// Remove the skin from pool
			///
			/// This function is thread safe
			///
			void remove(generator const &generator);
		
			///
			/// Render Skin
			///	
			/// This member function is used to render templates. Generally you should not use
			/// it directly, unless you have very good reasons.
			///
			/// \param skin - the name of the skin that should be used
			/// \param template_name - the name of template (class) that should be rendered.
			/// \param out - the output stream into which the view should be rendered
			/// \param content - the content that should be rendered using this view.
			///
			/// This function is thread safe
			///
			void render(std::string const &skin,std::string const &template_name,std::ostream &out,base_content &content);

			///
			/// Get all loaded views
			///
			/// This function is thread safe
			///
			std::vector<std::string> enumerate();
			
			///
			/// Get the singleton instance of the views pool
			///
			static pool &instance();
		
		private:
			pool();
			~pool();

			struct data;
			booster::hold_ptr<data> d;
		};

		///
		/// \brief This class controls the views used my application it knows to load them dynamically
		/// and reload if needed
		///
		class CPPCMS_API manager : public booster::noncopyable {
		public:
			///
			/// Create new views manager
			///
			/// Usually created by cppcms::service()
			///
			manager(json::value const &settings);
			~manager();

			///
			/// Render a template in a skin. Checks if any of shared objects/dlls should be reloaded
			///
			void render(std::string const &skin,std::string const &template_name,std::ostream &out,base_content &content);
			///
			/// Get default skin
			///
			std::string default_skin();
		private:
			struct data;
			booster::hold_ptr<data> d;
		};
	} // views

}


#endif
