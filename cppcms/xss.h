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
#include <booster/function.h>
#include <cppcms/defs.h>

#include <string.h>
#include <string>
#include <algorithm>

namespace cppcms {
	namespace json {
		class value;
	}
	///
	/// \brief Namespace that holds Anti-Cross Site Scripting Filter support
	///
	/// The classes in this namespace created to provide a filtering for a save
	/// handing of HTML and preventing XSS attacks
	///
	namespace xss {
		
		/// \cond INTERNAL
		namespace details {
			class c_string;
		}
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

			/// Create rules from JSON object \a r
			/// 
			/// The json object the defines the XSS prevention rules.
			/// This object has following properties:
			/// 
			/// - "xhtml" - boolean; default true - use XHTML (true) or HTML input
			/// - "comments" - boolean; setting it to true allows comments, default false
			/// - "numeric_entities" - boolean; setting it to true allows numeric_entities, default false
			/// - "entities" - array of strings: list of allowed HTML entities besides lt, gt and amp
			/// - "encoding" - string; the encoding of the text to validate, by default not checked and the
			/// 	input is assumed to be ASCII compatible. Always specifiy it for multibyte encodings
			/// 	like Shift-JIS or GBK as they are not ASCII compatible.
			/// - "tags" - object with 3 properties of type array of string: 
			/// 	- "opening_and_closing" - the tags that should come in pair like "<b></b>"
			/// 	- "stand_alone" - the tags that should appear stand alone like "<br/>"
			/// 	- "any_tag" - the tags that can be both like "<input>"
			/// - "attributes" - array of objects that define HTML attributes. Each object consists
			/// 	of following properties:
			/// 	- "type" - string - the type of the attribute one of: "boolean", "uri", "relative_uri",
			///		"absolute_uri", "integer", "regex".
			/// 	- "scheme" - string the allowed URI scheme - regular expression like "(http|ftp)". Used with
			/// 		"uri" and "absolute_uri" type
			/// 	- "expression"  - string the regular expression that defines the value that the attribute
			/// 		should match.
			/// 	- "tags" - array of strings - list of tags that this attribute is allowed for.
			/// 	- "attributes" - array of strings - lisf of names of the attribute
			///     - "pairs" - array of objects that consists of two properities "tag" and "attr" of
			///		type string that define tag and attributed that such type of property
			///		should be allowed for.
			///
			/// The extra properties that are not defined by this scheme are ingored
			///
			/// For example:
			/// \code
			/// {
			/// 	"xhtml" : true,
			/// 	"encoding" : "UTF-8",
			/// 	"entities" : [ "nbsp" , "copy" ],
			/// 	"comments" : false,
			/// 	"numeric_entities" : false,
			/// 	"tags" : {
			/// 		"opening_and_closing" : [
			/// 			"p", "b", "i", "tt",
			/// 			"a",
			/// 			"strong", "em",
			/// 			"sub", "sup",
			/// 			"ol", "ul", "li",
			/// 			"dd", "dt", "dl",
			/// 			"blockquote","code", "pre",
			///			"span", "div"
			/// 		],
			/// 		"stand_alone" : [ "br", "hr", "img" ]
			/// 	],
			/// 	"attributes": [
			/// 		{
			/// 			"tags" : [ "p", "li", "ul" ]
			/// 			"attr" : [ "style" ],
			/// 			"type" : "regex",
			/// 			"expression" : "\\s*text-algin:\\s*(center|left|right|justify);?\\s*"
			/// 		},
			/// 		{
			/// 			"tags" : [ "span", "div" ]
			/// 			"attr" : [ "class", "id" ],
			/// 			"type" : "regex",
			/// 			"expression" : "[a-zA-Z_0-9]+"
			/// 		},
			/// 		{
			///			"pairs" : [ 
			///				{ "tag" : "a",   "attr" : "href" },
			///				{ "tag" : "img", "attr" : "src"  }
			///			],
			/// 			"type" : "absolute_uri",
			/// 			"scheme" : "(http|https|ftp)"
			/// 		},
			///		{
			///			"tags" : [ "img" ],
			///			"attr" : [ "alt" ],
			///			"type" : "regex",
			///			"expression" : ".*"
			///		}
			/// 	]
			/// }
			/// \endcode
			///
			rules(json::value const &r);

			///
			/// Create rules from the JSON object stored in the file \a file_name
			///
			/// \see rules(json::value const&)
			///
			rules(std::string const &file_name);

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
			/// Functor that allows to provide custom validations for different properties
			///
			typedef booster::function<bool(char const *begin,char const *end)> validator_type;

			///
			/// Add the property that should be allowed to appear for specific tag as boolean property like
			/// checked="checked", when the type
			/// is HTML it is case insensitive.
			///
			/// The \a property should be ASCII only
			///
			void add_boolean_property(std::string const &tag_name,std::string const &property);
			///
			/// Add the property that should be checked using custom functor
			///
			void add_property(std::string const &tag_name,std::string const &property,validator_type const &val);
			///
			/// Add the property that should be checked using regular expression.
			///
			void add_property(std::string const &tag_name,std::string const &property,booster::regex const &r);
			///
			/// Add numeric property, same as add_property(tag_name,property,booster::regex("-?[0-9]+") but
			/// little bit more efficient
			///
			void add_integer_property(std::string const &tag_name,std::string const &property);

			///
			/// Add URI property.
			/// It should be used for properties like like "href" or "src".
			/// It is very good idea to use it in order to prevent urls like javascript:alert('XSS')
			/// 
			/// It's behavior is same as add_property(tag_name,property,rules::uri_validator());
			///
			void add_uri_property(std::string const &tag_name,std::string const &property);
			///
			/// Add URI property, using regular expression that matches allowed schemas.
			/// It should be used for properties like like "href" or "src".
			/// It is very good idea to use it in order to prevent urls like javascript:alert('XSS')
			/// 
			/// It's behavior is same as add_property(tag_name,property,rules::uri_validator(schema));
			///
			void add_uri_property(std::string const &tag_name,std::string const &property,std::string const &schema);

			///
			/// \deprecated use uri_validator 
			///
			/// Create a regular expression that checks URI for safe inclusion in the property. 
			/// By default it allows only: http, https, ftp, mailto, news, nntp.
			///
			/// If you need finer control over allowed schemas, use uri_matcher(std::string const&).
			///
			CPPCMS_DEPRECATED static booster::regex uri_matcher();
			///
			/// \deprecated use uri_validator 
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
			CPPCMS_DEPRECATED static booster::regex uri_matcher(std::string const &schema);

			////
			/// Create a validator that checks URI for safe inclusion in the property. 
			/// By default it allows only: http, https, ftp, mailto, news, nntp.
			///
			/// If you need finer control over allowed schemas, use uri_validator(std::string const&).
			///
			static validator_type uri_validator();
			////
			/// Create a validator that checks URI for safe inclusion in the property.
			/// - schema is a regular expression that matches specific protocols that can be used.
			/// - absolute_only - set to true to prevent accepting relative URIs like "/files/img.png" or "test.html"
			///
			/// \note You don't need to add "^" or "$" tags to \a scheme
			///
			/// For example:
			/// \code
			/// uri_validator("(http|https)");
			/// \endcode
			///
			///
			/// If you need finer control over allowed schemas, use uri_validator(std::string const&).
			///
			static validator_type uri_validator(std::string const &scheme,bool absolute_only = false);

			///
			/// Create a validator that checks that this URI is relative and it is safe for inclusion
			/// in URI property like href or src
			///
			static validator_type relative_uri_validator();

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
