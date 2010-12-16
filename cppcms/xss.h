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
#ifndef CPPCMS_XSS_H
#define CPPCMS_XSS_H

#include <booster/shared_ptr.h>
#include <cppcms/defs.h>

#include <string>

namespace cppcms {
	///
	/// \brief Namespace that holds Anti-Cross Site Scripting Filter support
	///
	/// The classes in this namespace created to provide a filtering for a save
	/// handing of HTML and preventing XSS attacks
	///
	namespace xss {
		///
		/// \brief The class that holds XSS filter rules
		///
		class CPPCMS_API rules {
		public:
			rules();
			rules(rules const &);
			rules const &operator=(rules const &);
			~rules();

			///
			/// How to treat in input
			///
			typedef enum {
				xhtml_input, ///< Assume that the input is XHTML
				html_input   ///< Assume that the input is HTML
			} html_type;
			
			///
			/// The type of tag
			///
			typedef enum {
				opening_and_closing 	= 1, ///< This tag should be opened and closed (a, or strong
				stand_alone 		= 2, ///< This tag should stand alone (like hr or br)
				any_tag  		= 3, ///< This tag can be used in both roles (like input)
			} tag_type;

			///
			/// How to filter invalid input data
			///
			typedef enum {
				escape_invalid, ///< Escape the invalid data - i.e. "<script>" goes to "&lt;script&gt;"
				remove_invalid  ///< Remove the invalid data - i.e. "<script>" got removed.
			} filtering_type;

			///
			/// Property type - the allowed type of the value
			///
			typedef enum {
				boolean_propery,	///< Boolean like disabled in HTML or disabled="disabled" in XHTML
				integer_propery, 	///< Integer property - value like "-?[0-9]+"
				textual_propery		///< General text property like alt="anything you want"
			} property_type;

			///
			/// Get how to treat input - HTML or XHTML
			///
			html_type html() const;
			///
			/// Set how to treat input - HTML or XHTML, it should be called first before you add any other
			/// rules
			///
			void html(html_type t);

			///
			/// Get filter method
			///
			filtering_type filtering() const;
			///
			/// Set filter method
			///
			void filtering(filtering_type f);

			///
			/// Add the tag that should be allowed to appear in the text, for HTML the name is case
			/// insensitive, i.e.  "br", "Br", "bR" and "BR" are valid tags for name "br".
			///
			/// The \a name should be ASCII only
			///
			void add_tag(std::string const &name,tag_type = any_tag);

			///
			/// Add allowed HTML entity, by default only "&lt;", "&gt;", "&quot;" and "&amp;" are allowed
			///
			void add_entity(std::string const &name);


			///
			/// Get if numeric entities are allowed, default is false
			///
			bool numeric_entities_allowed() const;

			///
			/// Set if numeric entities are allowed
			///
			void numeric_entities_allowed(bool v);

			///
			/// Add the property that should be allowed to appear for specific tag, when the type
			/// is HTML it is case insensitive.
			///
			/// The \a property should be ASCII only
			///
			void add_propery(std::string const &tag_name,std::string const &property,property_type type);
			///
			/// Add the property that should be checked using regular expression.
			/// is HTML it is case insensitive.
			///
			void add_propery(std::string const &tag_name,std::string const &property,booster::regex const &r);

			///
			/// Check if the comments are allowed in the text
			///
			bool comments_allowed() const;
			///
			/// Set to true if the comments are allowed in the text
			///
			void comments_allowed(bool comments);

			///
			/// Test if the tag is valid.
			/// \a tag should be lower case for HTML or unchanged for XHTML
			///
			bool valid_tag(std::string const &tag,tag_type type) const;
		
			///
			/// Test if the property is valid (without value) or unchanged for XHTML 
			/// \a tag and \a property should be lower case for HTML or unchanged for XHTML
			///	
			bool valid_property(std::string const &tag,std::string const &property) const;
			///
			/// Test if the property is valid (with value withing range [begin,end));
			///
			/// \a tag and \a property should be lower case for HTML or unchanged for XHTML
			///	
			bool valid_property(std::string const &tag,std::string const &property,char const *begin,char const *end) const;


		private:

			struct data;

			typedef booster::shared_ptr<data> pimpl;

			pimpl impl_;

			booster::shared_ptr<data> impl();
			booster::shared_ptr<data> impl() const;

		};

		///
		/// \brief The class that filters or validates the text based on given rules.
		///
		class CPPCMS_API filter {
		public:
			///
			/// \brief Create a filter that given rules \a set.
			///
			filter(rules const &set = rules());
			filter(filter const &);
			filter const &operator=(rules const &);
			~filter();

			///
			/// Perform validation of the text only, don't fix anything.
			///
			/// Validates the input text in range [begin,end) and returns true if it
			/// is valid
			///
			bool validate(char const *begin,char const *end) const;
			///
			/// Perform validation of the text in range [begin,end), and if it is not valid creates a valid version
			/// in \a output  only fixing it.
			///
			/// If the text is valid, validate_or_filter returns true and output remains unchanged, otherwise
			/// it 
			///
			///
			bool validate_or_filter(char const *begin,char const *end,std::string &output) const;

			///
			/// Filter the input according to the rules, such that output is always remains the valid
			/// HTML
			///
			std::string operator()(std::string const &input) const
			{
				char const *begin = input.c_str();
				char const *end = begin+input.size();
				std::string filtered_result;
				if(validate_or_filter(begin,end,filtered_result))
					return input;
				else
					return filtered_result;
				
			}
		};
	} // xss
}
#endif
