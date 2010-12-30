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

#include <booster/copy_ptr.h>
#include <booster/regex.h>
#include <cppcms/defs.h>

#include <string.h>
#include <string>
#include <algorithm>

namespace cppcms {
	///
	/// \brief Namespace that holds Anti-Cross Site Scripting Filter support
	///
	/// The classes in this namespace created to provide a filtering for a save
	/// handing of HTML and preventing XSS attacks
	///
	namespace xss {
		
		/// \cond INTERNAL
		namespace details {
	  
			class c_string {
			public:
				
				typedef char const *const_iterator;
				
				char const *begin() const
				{
					return begin_;
				}
				
				char const *end() const
				{
					return end_;
				}
				
				c_string(char const *s) 
				{
					begin_=s;
					end_=s+strlen(s);
				}

				c_string(char const *b,char const *e) : begin_(b), end_(e) {}

				c_string() : begin_(0),end_(0) {}

				bool compare(c_string const &other) const
				{
					return std::lexicographical_compare(begin_,end_,other.begin_,other.end_,std::char_traits<char>::lt);
				}

				bool icompare(c_string const &other) const
				{
					return std::lexicographical_compare(begin_,end_,other.begin_,other.end_,ilt);
				}

				explicit c_string(std::string const &other)
				{
					container_ = other;
					begin_ = container_.c_str();
					end_ = begin_ + container_.size();
				}
				c_string(c_string const &other)
				{
					if(other.begin_ == other.end_) {
						begin_ = end_ = 0;
					}
					else if(other.container_.empty()) {
						begin_ = other.begin_;
						end_ = other.end_;
					}
					else {
						container_ = other.container_;
						begin_ = container_.c_str();
						end_ = begin_ + container_.size();
					}
				}
				c_string const &operator=(c_string const &other)
				{
					if(other.begin_ == other.end_) {
						begin_ = end_ = 0;
					}
					else if(other.container_.empty()) {
						begin_ = other.begin_;
						end_ = other.end_;
					}
					else {
						container_ = other.container_;
						begin_ = container_.c_str();
						end_ = begin_ + container_.size();
					}
					return *this;
				}

			private:
				static bool ilt(char left,char right)
				{
					unsigned char l = tolower(left);
					unsigned char r = tolower(right);
					return l < r;
				}
				static char tolower(char c)
				{
					if('A' <= c && c<='Z')
						return c-'A' + 'a';
					return c;
				}
				char const *begin_;
				char const *end_;
				std::string container_;
			};
			
		} // details
		
		struct basic_rules_holder;
		
		/// \endcond

		///
		/// \brief The class that holds XSS filter rules
		///
		/// This is the major class the defines the white list rules to handle the 
		/// Correct HTML input.
		///
		/// When using these rules you should be very strict about what you need and what you
		/// allow.
		/// 
		/// Basically you need to specify:
		///
		/// -#  The XHTML or HTML parsing rules - should be done first
		/// -#  The encoding of the text. If you do not specify the encoding
		///     it would be assumed that it is ASCII compatible. 
		///     You may not specify encoding only if you know that it was validated
		///     for example by using widgets::text, otherwise \b always specify 
		///     encoding
		/// -#  Provide the list of tags that should be used. Specify only thous you need.
		///     .
		///     Never allow tags like style, object, embed or of course script as they can be easily
		///     used for XSS attacks 
		/// -#  Provide essential HTML attributes - properties for tags you need. 
		///     Use add_uri_property for links like src for img or href for a.
		///     It would check correctness of URI syntax and ensure that only white-listed 
		///     schemas are allowed (i.e. no javascript would be allowed).
		///     .
		///     Never allow style tags unless you specify very strict white list of really used
		///     styles. Styles can be easily exploited for both XSS and click-jacking. For example
		///     \code
		///     <p style="width: expression(alert('XSS'));"></p>
		///     \endcode
		///     .
		///     If you want to use styles specify very strict list of things you need like:
		///     \code
		///     add_property("p","style",booster::regex("text-align:(left|right|center)"));
		///     \endcode
		/// -#  Do not allow comments unless you need them. Note not all comments are allowed. Comments
		///     containing "<", ">" or "&" would be considered invalid as some exploits use them.
		///
		/// Remember more strict you are it is harder to make attack. Read about XSS, see existing
		/// attacks to understand how they work and then decide what you allow.
		///
		/// rules class can be treated as value for thread safe access, i.e. you can safely use
		/// const reference and const member functions as long as you don't change the rules 
		/// under the hood.
		///
		/// The simplest way: define at application startup some global rules object configure
		/// it and use it for filtering and validation - and make your attackers cry :-).
		///
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
				invalid_tag		= 0, ///< This tag is invalid (returned by validate)
				opening_and_closing 	= 1, ///< This tag should be opened and closed like em	, or strong
				stand_alone 		= 2, ///< This tag should stand alone (like hr or br)
				any_tag  		= 3, ///< This tag can be used in both roles (like input)
			} tag_type;

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
			/// Add the tag that should be allowed to appear in the text, for HTML the name is case
			/// insensitive, i.e.  "br", "Br", "bR" and "BR" are valid tags for name "br".
			///
			/// The \a name should be ASCII only
			///
			void add_tag(std::string const &name,tag_type = any_tag);

			///
			/// Add allowed HTML entity, by default only "lt", "gt", "quot" and "amp" are allowed
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
			/// Add the property that should be allowed to appear for specific tag as boolean property like
			/// checked="checked", when the type
			/// is HTML it is case insensitive.
			///
			/// The \a property should be ASCII only
			///
			void add_boolean_property(std::string const &tag_name,std::string const &property);
			///
			/// Add the property that should be checked using regular expression.
			///
			void add_property(std::string const &tag_name,std::string const &property,booster::regex const &r);
			///
			/// Add numeric property, same as add_property(tag_name,property,booster::regex("-?[0-9]+")
			///
			void add_integer_property(std::string const &tag_name,std::string const &property);

			///
			/// Add URI property.
			/// It should be used for properties like like "href" or "src".
			/// It is very good idea to use it in order to prevent urls like javascript:alert('XSS')
			/// 
			/// It's behavior is same as add_property(tag_name,property,rules::uri_matcher());
			///
			void add_uri_property(std::string const &tag_name,std::string const &property);
			///
			/// Add URI property, using regular expression that matches allowed schemas.
			/// It should be used for properties like like "href" or "src".
			/// It is very good idea to use it in order to prevent urls like javascript:alert('XSS')
			/// 
			/// It's behavior is same as add_property(tag_name,property,rules::uri_matcher(schema));
			///
			void add_uri_property(std::string const &tag_name,std::string const &property,std::string const &schema);

			///
			/// Create a regular expression that checks URI for safe inclusion in the property. 
			/// By default it allows only: http, https, ftp, mailto, news, nntp.
			///
			/// If you need finer control over allowed schemas, use uri_matcher(std::string const&).
			///
			static booster::regex uri_matcher();
			///
			/// Create a regular expression that checks URI for safe inclusion in the text, where
			/// schema is a regular expression that matches specific protocols that can be used.
			///
			/// \note Don't add "^" or "$" tags as this expression would be used in construction of regular
			/// other expression.
			///
			/// For example:
			/// \code
			/// booster::regex uri = uri_matcher("(http|https)");
			/// \endcode
			///
			static booster::regex uri_matcher(std::string const &schema);

			///
			/// Check if the comments are allowed in the text
			///
			bool comments_allowed() const;
			///
			/// Set to true if the comments are allowed in the text
			///
			void comments_allowed(bool comments);

			///
			/// Set the character encoding of the source, otherwise encoding is not checked and
			/// assumed valid all invalid characters are removed from the text or replaced with default character
			///
			/// It is very important to specify this option. You may skip it if you are sure that the
			/// the input encoding was already validated using cppcms::form::text widget that handles
			/// character encoding validation by default.
			///
			/// In any case it is generally better to always specify this option.
			///
			/// 
			/// \note the replace functionality is not supported for all encoding, only UTF-8, ISO-8859-* and single byte windows-12XX
			/// encodings support such replacement with default character, for all other encodings like Shift-JIS, the invalid
			/// characters or characters that are invalid for use in HTML are removed.
			///
			void encoding(std::string const &enc);


			/// \cond INTERNAL

			///
			/// Test if the tag is valid.
			/// \a tag should be lower case for HTML or unchanged for XHTML
			///
			tag_type valid_tag(details::c_string const &tag) const;
		
			///
			/// Test if the property is valid (without value) or unchanged for XHTML 
			/// \a tag and \a property should be lower case for HTML or unchanged for XHTML
			///	
			bool valid_boolean_property(details::c_string const &tag,details::c_string const &property) const;
			///
			/// Test if the property and its \a value are valid;
			///
			/// \a tag and \a property should be lower case for HTML or unchanged for XHTML
			///	
			bool valid_property(details::c_string const &tag,details::c_string const &property,details::c_string const &value) const;

			///
			/// Test if specific html entity is valid
			///
			bool valid_entity(details::c_string const &val) const;

			///
			/// Get the encoding, returns empty string if not encoding testing
			/// is required
			///
			std::string encoding() const;

			/// \endcond


		private:
			basic_rules_holder &impl();
			basic_rules_holder const &impl() const;

			struct data;
			booster::copy_ptr<data> d;

		};
		
		///
		/// \brief The enumerator that defines filtering invalid HTML method
		///
		typedef enum {
			remove_invalid, ///< Remove all invalid HTML form the input
			escape_invalid  ///< Escape (convert to text) all invalid HTML in the input
		} filtering_method_type;

		///
		/// \brief Check the input in range [\a begin, \a end) according to the rules \a r.
		///
		/// It does not filters the input it only checks its validity, it would be faster then validate_and_filter_if_invalid
		/// or filter functions but it does not correct errors.
		///
		CPPCMS_API bool validate(char const *begin,char const *end,rules const &r);
		///
		/// \brief Validate the input in range [\a begin, \a end) according to the rules \a r and if it is not valid filter it
		/// and save filtered text into \a filtered string using a filtering method \a method.
		///
		/// If the data was valid, \a filtered remains unchanged and the function returns true, otherwise it returns false
		/// and the filtered data is saved.
		///
		CPPCMS_API bool validate_and_filter_if_invalid(	char const *begin,
								char const *end,
								rules const &r,
								std::string &filtered,
								filtering_method_type method=remove_invalid,
								char replacement_char = 0);

		///
		/// \brief Filter the input in range [\a begin, \a end) according to the rules \a r using filtering 
		/// method \a method
		///
		CPPCMS_API std::string filter(char const *begin,
					      char const *end,
					      rules const &r,
					      filtering_method_type method=remove_invalid,
					      char replacement_char = 0);
		///
		/// \brief Filter the input text \a input according to the rules \a r using filtering method \a method
		///
		CPPCMS_API std::string filter(std::string const &input,
					      rules const &r,
					      filtering_method_type method=remove_invalid,
					      char replacement_char = 0);

	} // xss
}
#endif
