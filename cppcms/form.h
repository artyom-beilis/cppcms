///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_FORM_H
#define CPPCMS_FORM_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>

#include <string>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <stack>
#include <ostream>
#include <sstream>
#include <cppcms/http_context.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <booster/copy_ptr.h>
#include <booster/perl_regex.h>
#include <booster/shared_ptr.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/util.h>
#include <cppcms/localization.h>

namespace cppcms {

	namespace http {
		class file;
	}

	namespace widgets {
		class base_widget;
	}

	///
	/// \brief This struct holds various flags to control the HTML generation.
	///
	struct form_flags {
		///
		/// This enum represents the HTML/XHTML switch.
		///
		typedef enum {
			as_html = 0,	///< render form/widget as ordinary HTML
			as_xhtml= 1,	///< render form/widget as XHTML
		} html_type;

		///
		/// This enum represents the style for the widgets generation.
		///
		typedef enum {
			as_p	= 0 , ///< Render each widget using paragraphs
			as_table= 1 , ///< Render each widget using table
			as_ul 	= 2 , ///< Render each widget using unordered list
			as_dl	= 3 , ///< Render each widget using definitions list
			as_space= 4   ///< Render each widget using simple blank space separators
		} html_list_type;

		///
		/// This special flag is used to partially generate a widget's HTML.
		///
		typedef enum {
			first_part  = 0, ///< Render part 1: HTML attributes can be inserted after it.
			second_part = 1  ///< Render part 2: complete part 1.
		} widget_part_type;
	};

	///
	/// \brief This class represents the context required to generate the widgets' HTML.
	///
	class CPPCMS_API form_context : public form_flags
	{
	public:
		///
		/// Default constructor.
		///
		form_context();

		///
		/// Copy-constructor.
		///
		form_context(form_context const &other);

		///
		/// Assignment.
		///
		form_context const &operator = (form_context const &other);

		///
		/// Create a rendering context.
		///
		/// \param output the std::ostream output to write HTML to.
		/// \param ht flags represents the type of HTML that should be generated.
		/// \param hlt flag defines the style of widgets generation.
		///
		form_context(	std::ostream &output,
				html_type ht = form_flags::as_html,
				html_list_type hlt=form_flags::as_p);

		///
		/// Destructor.
		///
		~form_context();
	
		///
		/// Set the HTML/XHTML flag.
		///	
		void html(html_type t);

		///
		/// Set the widgets rendering style.
		///
		void html_list(html_list_type t);

		///
		/// Set the flag for the partial rendering of the widget.
		///
		void widget_part(widget_part_type t);

		///
		/// Set the output stream.
		///
		void out(std::ostream &out);
		
		///
		/// Set the HTML/XHTML flag. The default is \a as_html.
		///	
		html_type html() const;

		///
		/// Get the widget rendering style. The default is \a as_p.
		///
		html_list_type html_list() const;

		///
		/// Get the part of the widget that should be generated. See \a widget_part_type. The default is \a first_part.
		///
		widget_part_type widget_part() const;

		///
		/// Get the output stream.
		///
		std::ostream &out() const;

	private:
		uint32_t html_type_;
		uint32_t html_list_type_;
		uint32_t widget_part_type_;
		std::ostream *output_;
		uint32_t reserved_1;
		uint32_t reserved_2;
		struct _data;
		booster::hold_ptr<_data> d;

	};

	
	///
	/// \brief This class is the base class for any form or form widget used in CppCMS.
	///
	/// It provides basic, abstract operations that every widget or form should implement.
	///
	class CPPCMS_API base_form : public form_flags {
	public:
		///
		/// Render the widget to the output set in \a cppcms::form_context::out()
		/// according to the control flags set in \a cppcms::form_flags.
		/// Usually this function is called directly by the template rendering functions.
		///
		virtual void render(form_context &context) = 0;

		///
		/// Load the form information from the provided http::context \a context.
		/// A user can call this function to load all information from the raw POST/GET
		/// data into the internal widget representation.
		///
		virtual void load(http::context &context) = 0;

		///
		/// Validate the form according to defined rules. If all checks are OK,
		/// true is returned. If some widget or form fails, false is returned.
		///
		virtual bool validate() = 0;

		///
		/// Clear the form from all user provided data.
		///
		virtual void clear() = 0;

		///
		/// Set the parent of this form. Used internally. You should not use it.
		///
		virtual void parent(base_form *subform) = 0;

		///
		/// Get the parent of this form. If this is the topmost form, NULL is returned.
		///
		virtual base_form *parent() = 0;

		base_form();
		virtual ~base_form();
	};

	///
	/// \brief The \a form is a container used to collect other widgets and forms into a single unit.
	///
	/// Generally various widgets and forms are combined into a single form in order to simplify their rendering
	/// and validation of the forms that include more than one widget.
	///
	class CPPCMS_API form :	public booster::noncopyable,
				public base_form
	{
	public:
		form();
		virtual ~form();

		///
		/// Render all the widgets and sub-forms to the \a output, using
		/// the flags defined in the \a context.
		///
		virtual void render(form_context &context);

		///
		/// Load all the widget information from http::context \a cont.
		///
		virtual void load(http::context &cont);

		///
		/// Validate all subforms and widgets. If at least one of them fails,
		/// false is returned. Otherwise, true is returned.
		///
		virtual bool validate();

		///
		/// Clear all subforms and widgets from all loaded data.
		///
		virtual void clear();

		///
		/// Add \a subform to form. The ownership is not transferred to
		/// the parent.
		///
		void add(form &subform);

		///
		/// Add \a subform to form. The ownership is transferred to
		/// the parent and the subform will be destroyed together with
		/// the parent.
		///
		void attach(form *subform);

		///
		/// Add \a widget to form. The ownership is not transferred to
		/// to the parent.
		///
		void add(widgets::base_widget &widget);

		///
		/// Add \a widget to form. The ownership is transferred to
		/// the parent and the widget will be destroyed together with
		/// the parent.
		///
		void attach(widgets::base_widget *widget);

		///
		/// \deprecated Use add(form &) instead
		///
		/// Shortcut to \a add.
		///
		CPPCMS_DEPRECATED inline form &operator + (form &f)
		{
			add(f);
			return *this;
		}
		
		///
		/// \deprecated Use add(widgets::base_widget &) instead
		///
		/// Shortcut to \a add.
		///
		CPPCMS_DEPRECATED inline form &operator + (widgets::base_widget &f)
		{
			add(f);
			return *this;
		}

		///
		/// Set the parent of this form. It is used internally; you should not use it. It is called
		/// when the form is added or attached to another form.
		///
		virtual void parent(base_form *subform);

		///
		/// Get the parent of this form. If this is the topmost form, NULL is returned.
		/// It is assumed that the parent is always a form.
		///
		virtual form *parent();
		
		///
		/// \brief Input iterator used to iterate over all the widgets in a form.
		///
		/// This class is mainly used by templates to render widgets. It
		/// recursively iterates over all the widgets and subforms.
		///
		/// \note it iterates over widgets only and never \ref 
		///  form objects.
		///
		/// \code
		/// iterator p=f.begin();
		/// if(p!=f.end())
		///   if p!=f.end() --> *p is derived from widgets::base_widget.
		/// \endcode
		///
		class CPPCMS_API iterator : public std::iterator<std::input_iterator_tag,widgets::base_widget>
		{
		public:
			///
			/// End iterator.
			///
			iterator();

			///
			/// Create a widget iterator.
			///
			iterator(form &);

			///
			/// Destructor.
			///
			~iterator();

			///
			/// Copy the iterator. This operation is not cheap.
			///
			iterator(iterator const &other);

			///
			/// Assign the iterator. This operation is not cheap.
			///
			iterator const &operator = (iterator const &other);

			///
			/// Return the underlying widget. Condition: *this!=iterator().
			///
			widgets::base_widget *operator->() const
			{
				return get();
			}

			///
			/// Return the underlying widget. Condition: *this!=iterator().
			///
			widgets::base_widget &operator*() const
			{
				return *get();
			}

			///
			/// Check if two iterators point to the same element.
			///
			bool operator==(iterator const &other) const
			{
				return equal(other);
			}

			///
			/// Check if two iterators point to different elements.
			///
			bool operator!=(iterator const &other) const
			{
				return !equal(other);
			}

			///
			/// Post increment operator. It forwards the iterator to the next widget.
			/// Note it does not point to the higher level form container.
			///
			/// Note: it is preferable to use ++i rather than i++ as copying iterators is not cheap.
			///
			iterator operator++(int /*unused*/)
			{
				iterator tmp(*this);
				next();
				return tmp;
			}

			///
			/// Increment operator. It forwards the iterator to the next widget.
			/// Note it does not point to the higher level form container.
			///
			iterator &operator++()
			{
				next();
				return *this;
			}

		private:

			friend class form;

			bool equal(iterator const &other) const;
			void zero();
			void next();
			widgets::base_widget *get() const;

			std::stack<unsigned> return_positions_;
			form *current_;
			unsigned offset_;
			struct _data;
			booster::copy_ptr<_data> d;

		};

		///
		/// Returns the iterator to the first widget.
		///
		iterator begin();

		///
		/// Returns the end of the range iterator.
		///
		iterator end();


	private:
		friend class iterator;

		struct _data;
		// Widget and ownership - true means I own it.
		typedef std::pair<base_form *,bool> widget_type;
		std::vector<widget_type> elements_;
		form *parent_;
		booster::hold_ptr<_data> d;
	};



	///
	/// \brief This namespace includes all the widgets (i.e. parts of HTML forms) supported by cppcms.
	///
	namespace widgets {

		///
		/// \brief this class is the base class of all renderable widgets which
		/// can be used with CppCMS form system.
		///
		/// All cppcms widgets are derived from this class. Users who want to create
		/// their own custom widgets must derive them from this class.
		///

		class CPPCMS_API base_widget : 	
			public base_form,
			public booster::noncopyable
		{
		public:
			///
			/// Default constructor.
			///
			base_widget();

			virtual ~base_widget();
			
			/// 
			/// Check if a value has been assigned to the widget. It is usually true
			/// when the user has assigned a value to the widget or when the widget is loaded.
			///
			/// If there is a reasonable default value for the widget 
			/// then set() should be true. For widgets like file or numeric
			/// where explicit parsing is required, the set() value would indicate
			/// that user provided some value (i.e. uploaded a file or entered a number).
			///
			bool set();

			///
			/// After having executed the validation process, each widget can be tested for validity.
			///
			bool valid();

			///
			/// Get the HTML \a id attribute.
			///
			std::string id();

			///
			/// Get the HTML \a name attribute.
			/// 
			std::string name();

			///
			/// Get the short message that would be displayed near the widget.
			/// 
			locale::message message();

			///
			/// Check if a message is set.
			///
			bool has_message();

			///
			/// Get the error message that would be displayed near the widget
			/// if the widget validation failed.
			/// 
			locale::message error_message();

			///
			/// Check if an error message is set.
			///
			bool has_error_message();

			///
			/// Get the eventual long description of the wigget.
			///
			locale::message help();

			///
			/// Check if a help message is set.
			///
			bool has_help();

			///
			/// Get the HTML \c disabled attribute.
			///
			bool disabled();

			///
			/// Set/Unset the HTML \c disabled attribute.
			///
			void disabled(bool);

			///
			/// Get the general user defined attribute string that can be added to the widget.
			/// 
			std::string attributes_string();

			///
			/// Set the existence of content for the widget. By default the widget is not set.
			/// By convention, trying to fetch a value from a widget that is "unset" will throw an exception.
			/// Call set(true) to change the state to "set" and call set(false) to change it to "unset".
			///
			void set(bool);

			///
			/// Set th validity state of the widget. By default the widget is valid. If it fails to pass
			/// the validation, its validity state is changed by calling this function.
			///
			/// Note: a widget may be "unset" and still be valid. Conversely, it may be set but be not-valid.
			///
			void valid(bool);

			///
			/// Set the HTML \c id attribute of the widget.
			///
			void id(std::string);

			///
			/// Set the HTML \c name attribute of the widget. Note: if this attribute
			/// is not set, the widget will not be able to be loaded from the POST/GET
			/// data.
			///
			void name(std::string);

			///
			/// Set a short description for the widget. Generally, it is a good idea to
			/// define this value.
			///
			/// The short message can also be set using the base_widget constructor.
			///
			void message(std::string);

			///
			/// Set a short translatable description for the widget. Generally, it is a good idea to
			/// define this value.
			///
			/// The short message can also be set using the base_widget constructor.
			///
			void message(locale::message const &);

			///
			/// Set the error message that is displayed for invalid widgets.
			///
			/// If it is not set, a simple "*" is shown instead.
			///
			void error_message(std::string);

			///
			/// Set the translatable error message that is displayed for invalid widgets.
			///
			/// If it is not set, a simple "*" is shown instead.
			///
			void error_message(locale::message const &);

			///
			/// Set a longer help message that describes this widget.
			///
			void help(std::string);
			
			///
			/// Set a translatable help message that describes this widget.
			///
			void help(locale::message const &msg);

			///
			/// Set general HTML attributes that are not directly supported
			/// For example:
			///
			/// \code
			///  my_widget.attributes_string("style='direction:rtl' onclick='return foo()'");
			/// \endcode
			///
			/// This string is inserted as-is just after render_input_start.
			/// 
			void attributes_string(std::string v);


			///
			/// Render the full widget together with error messages and decorations as paragraphs
			/// or table elements to the output set in \a cppcms::form_context::out().
			///
			virtual void render(form_context &context);

			///
			/// This is a virtual member function that should be implemented by each widget.
			/// It executes the actual rendering of the HTML form. 
			///
			virtual void render_input(form_context &context) = 0;

			///
			/// Clear the form. It also calls set(false).
			///
			virtual void clear();

			///
			/// Validate the form. If not overridden, it sets the widget to \a valid.
			///
			virtual bool validate();

			///
			/// Render standard common attributes like \a id, \a name, \a disabled, etc.
			///
			virtual void render_attributes(form_context &context);
			
			///
			/// Set the parent of this widget. It is used internally; you should not use it. It is called
			/// when the form is added or attached to another form.
			///
			virtual void parent(base_form *subform);

			///
			/// Get the parent of this form. If this is the topmost form, NULL is returned.
			/// It is assumed that the parent is always a form.
			///
			virtual form *parent();

			///
			/// This function should be called before actual loading
			/// of widgets, it performs cross widgets validation
			/// and causes automatic generation of undefined names
			///
			void pre_load(http::context &);

		protected:
			///
			/// This function should be called by overloadeding the load/render methods
			/// before the loading/rendering starts.
			///
			void auto_generate(form_context *context = 0);

		private:
			void generate(int position,form_context *context = 0);

			std::string id_;
			std::string name_;
			locale::message message_;
			locale::message error_message_;
			locale::message help_;
			std::string attr_;
			form *parent_;

			uint32_t is_valid_  : 1;
			uint32_t is_set_ : 1;
			uint32_t is_disabled_ : 1;
			uint32_t is_generation_done_ : 1;
			uint32_t has_message_ : 1;
			uint32_t has_error_ : 1;
			uint32_t has_help_ : 1;
			uint32_t reserverd_ : 25;

			struct _data;
			booster::hold_ptr<_data> d;
		};

		///
		/// \brief This widget is used as base for text input fields.
		///
		/// This widget is used as the base class for other widgets that are used for
		/// text input like: \ref text, \ref textarea, etc.
		///
		/// This widget does much more than reading simple text data from the POST
		/// or GET form. It also performs charset validation.
		///
		class CPPCMS_API base_text : virtual public base_widget {
		public:
			
			base_text();
			virtual ~base_text();

			///
			/// Get the string that contains the input value of the widget.
			///
			std::string value();
			
			///
			/// Set the widget content to the value \a v before rendering.
			///
			void value(std::string v);
			
			///
			/// Inform the validator that this text widget should contain some text.
			/// It is similar to limits(1,-1).
			///
			void non_empty();
			
			///
			/// Set the minimum and maximum limits of the text size. Note: max == -1 indicates that there
			/// is no maximum limit; min==0 indicates that there is no minimum limit.
			///
			/// Note: these numbers represent the length in Unicode code points (even if the encoding
			/// is not Unicode). If the character set validation is disabled, then these numbers represent
			/// the number of octets in the string.
			///
			void limits(int min,int max);

			///
			/// Get the minimum and maximum size limits, 
			///
			std::pair<int,int> limits();

			///
			/// Inform the validator whether it should check the validity of the charset.
			/// The default is enabled (true).
			///
			/// Generally you should not use this option to disable the charset validation
			/// unless you want to load some raw data as
			/// form input, or the character set is different from the one defined in the locale.
			///
			void validate_charset(bool );
			
			///
			/// Return true if the charset validation is enabled.
			///
			bool validate_charset();

			///
			/// Validate the widget content according to the rules and to the charset encoding.
			///
			/// Notes:
			/// 
			/// -  The charset validation is very efficient for variable length UTF-8 encoding as well as 
			///    for most popular fixed length encodings like ISO-8859-*, windows-125* and koi8*.
			///    For other encodings, character set conversion is used for the actual validation.
			/// -  Special characters (that are not allowed in HTML) are assumed to be forbidden, even if they are
			///    valid code points (like NUL = 0 or DEL=127).
			/// 
			virtual bool validate();

			///
			/// Load the widget for http::context. It uses the locale given in the context to
			/// validate the text.
			///
			virtual void load(http::context &);

		private:
			std::string value_;
			int low_;
			int high_;
			bool validate_charset_;
			size_t code_points_;
			struct _data;
			booster::hold_ptr<_data> d;
		};

		///
		/// \brief This class represents a basic widget that generates HTML form elements
		/// the widgets that use the <input \/> HTML tag.
		///
		/// It allows you to create your own widgets more easily. It does most of job required to
		/// generate the HTML. The user is only required to generate the actual value like
		/// value="10.34" as with a numeric widget.
		///

		class CPPCMS_API base_html_input : virtual public base_widget {
		public:
			///
			/// Create a new instance. \a type is the HTML type tag of the input element, for example "text" or
			/// "password".
			///
			base_html_input(std::string const &type);

			///
			/// Virtual destructor.
			///
			virtual ~base_html_input();

			///
			/// This function generates the actual HTML. It calls render_value where needed.
			///
			virtual void render_input(form_context &context);

		protected:
			///
			/// Write the actual value of the HTML tag. Derived classes must implement this.
			/// 
			virtual void render_value(form_context &context) = 0;

		private:
			struct _data;
			booster::hold_ptr<_data> d;
			std::string type_;
		};

		///
		/// \brief This class represents an HTML form input element of type text.
		/// 
		class CPPCMS_API text : public base_html_input, public base_text
		{
		public:
			///
			/// Create a text field widget.
			///
			text();
			
			///
			/// This constructor is provided for use by derived classes where it is required
			/// to change the type of the widget, like text, password, etc. 
			///
			text(std::string const &type);

			~text();

			///
			/// Set the HTML size attribute of the widget.
			///
			void size(int n);

			///
			/// Get the HTML size attribute size of the widget. It returns -1 if it is undefined.
			///
			int size();


			virtual void render_attributes(form_context &context);
			virtual void render_value(form_context &context);

		private:
			int size_;
			struct _data;
			booster::hold_ptr<_data> d;
		};

		///
		/// \brief This widget represents a hidden input form element. It is used to provide
		/// information invisible to the user.
		///
		/// I has the same properties as a text widget has but it does not render any HTML
		/// for message(), help() and other informational types.
		///
		/// When you render the form in templates, it is a good idea to render it separately
		/// to make sure that no invalid HTML is created.
		///
		class CPPCMS_API hidden : public text 
		{
		public:
			hidden();
			~hidden();
			///
			/// Render the HTML of the widget. It overrides
			/// the default HTML rendering as hidden widget
			/// is never displayed.
			///
			virtual void render(form_context &context);
		private:
			struct _data;
			booster::hold_ptr<_data> d;
		};


		///
		/// \brief This text widget behaves similarly to the text widget but uses
		/// the \c textarea HTML tag rather than the \c input HTML tag.
		///
		class CPPCMS_API textarea : public base_text 
		{
		public:
			textarea();
			~textarea();

			///
			/// Get the number of rows in the textarea. The default is -1 -- undefined.
			///
			int rows();

			///
			/// Get the number of columns in the textarea. The default is -1 -- undefined.
			///
			int cols();

			///
			/// Set the number of rows in the textarea.
			///
			void rows(int n);

			///
			/// Set the number of columns in the textarea.
			///
			void cols(int n);

			virtual void render_input(form_context &context);

		private:
			int rows_,cols_;

			struct _data;
			booster::hold_ptr<_data> d;
		};

		///
		/// \brief Widget for number input. It is a template class that assumes that T is a number.
		///
		/// This class parses the input and checks if it is input text is a valid
		/// numeric value. If it is valid, the set() would return true
		/// be true.
		///
		/// If the value was not defined, access to value() will throw an exception.
		///
		template<typename T>
		class numeric: public base_html_input {
		public:
			numeric() :
				base_html_input("text"),
				check_low_(false),
				check_high_(false),
				non_empty_(false)
			{
			}

			///
			/// Inform the validator that this widget should contain some value.
			///
			void non_empty()
			{
				non_empty_=true;
			}


			///
			/// Get numeric value that was loaded from the POST or
			/// GET data. 
			///
			/// \note if the value was not set (empty field for example)
			/// then this function will throw. So it is good idea to
			/// check if \ref set() returns true before using this
			/// function.
			///
			T value()
			{
				if(!set())
					throw cppcms_error("Value not loaded");
				return value_;
			}

			///
			/// Set the value of the widget.
			///
			void value(T v)
			{
				set(true);
				value_=v;
			}

			/// 
			/// Set the minimum valid value.
			///
			void low(T a)
			{
				min_=a;
				check_low_=true;
				non_empty();
			}

			/// 
			/// Set the maximum valid value.
			///
			void high(T b)
			{
				max_=b;
				check_high_=true;
				non_empty();
			}

			///
			/// Same as low(a); high(b);
			///			
			void range(T a,T b)
			{
				low(a);
				high(b);
			}

			///
			/// Render the first part of the widget.
			///
			virtual void render_value(form_context &context)
			{
				if(set())
					context.out()<<"value=\""<<value_<<"\" ";
				else
					context.out()<<"value=\""<<util::escape(loaded_string_)<<"\" ";
			}

			virtual void clear()
			{
				base_html_input::clear();
				loaded_string_.clear();
			}

			///
			/// Load the widget data.
			///
			virtual void load(http::context &context)
			{
				pre_load(context);

				loaded_string_.clear();

				set(false);
				valid(true);

				http::request::form_type::const_iterator p;
				http::request::form_type const &request=context.request().post_or_get();
				p=request.find(name());
				if(p==request.end()) {
					return;
				}
				else {
					loaded_string_=p->second;
					if(loaded_string_.empty())
						return;

					std::istringstream ss(loaded_string_);
					ss.imbue(context.locale());
					ss>>value_;
					if(ss.fail() || !ss.eof())
						valid(false);
					else
						set(true);
				}
			}
			
			///
			/// Validate the widget.
			///
			virtual bool validate()
			{
				if(!valid())
					return false;
				if(!set()) {
					if(non_empty_) {
						valid(false);
						return false;
					}
					return true;
				}
				if(check_low_ && value_ <min_) {
					valid(false);
					return false;
				}
				if(check_high_ && value_ > max_) {
					valid(false);
					return false;
				}
				return true;
			}

		private:

			T min_,max_,value_;

			bool check_low_;
			bool check_high_;
			bool non_empty_;
			std::string loaded_string_;
		};

		/// 
		/// \brief The password widget is a simple text widget with few, obvious differences.
		//  
		class CPPCMS_API password: public text {
		public:
			password();

			~password();

			///
			/// Set the equality constraint to another password widget. This password should be
			/// equal to the one in \a p2. It is usefull when creating new passwords: if the passwords
			/// are not equal, the validation will fail.
			///
			void check_equal(password &p2);
			virtual bool validate();

		private:
			struct _data;
			booster::hold_ptr<_data> d;
			password *password_to_check_;
		};

		
		///
		/// \brief This class is extending a simple text widget by using additional regular expression validation.
		///
		class CPPCMS_API regex_field : public text {
		public:
			regex_field();
			///
			/// Create a widget using the regular expression \a e.
			///
			regex_field(booster::regex const &e);
			
			///
			/// Create a widget using the regular expression \a e.
			///
			regex_field(std::string const &e);


			///
			/// Set the regular expression.
			///
			void regex(booster::regex const &e);

			~regex_field();
			
			virtual bool validate();

		private:
			booster::regex expression_;
			struct _data;
			booster::hold_ptr<_data> d;
		};

		///
		/// \brief This widget checks that the input is a valid email address.
		///
		class CPPCMS_API email : public regex_field {
		public:

			email();
			~email();

		private:
			struct _data;
			booster::hold_ptr<_data> d;
		};

		///
		/// \brief This class represent an HTML checkbox input element.
		///
		class CPPCMS_API checkbox: public base_html_input {
		public:
			///
			/// The constructor that allows you to specify \c type HTML
			/// attribute. It is passed to the constructor of the 
			/// \ref base_html_input class.
			///
			checkbox(std::string const &type);

			///
			/// Default constructor.
			///
			checkbox();
			virtual ~checkbox();
			
			///
			/// Return true if the checkbox was checked (selected).
			///
			bool value();

			///
			/// Set the state as \a checked.
			///
			void value(bool is_set);

			///
			/// Get the unique identification string of the checkbox.
			///
			std::string identification();

			///
			/// Set the unique identification string of the checkbox. It is useful when you want to
			/// have many options with the same name.
			///
			void identification(std::string const &);

			virtual void render_value(form_context &context);
			virtual void load(http::context &context);

		private:
			struct _data;
			booster::hold_ptr<_data> d;
			std::string identification_;
			bool value_;
		};
	
		///
		/// \brief This widget represents an HTML multiple select form element.
		///	
		class CPPCMS_API select_multiple : public base_widget {
		public:
			select_multiple();
			~select_multiple();

			///
			/// Add to the multiple select a new option with the display name \a msg, and specify if it is initially
			/// selected. The default is \a false.
			///
			void add(std::string const &msg,bool selected=false);

			///
			/// Add to the multiple select a new option with the display name \a msg, and specify if it is initially
			/// selected (the default is \a false), providing a unique identification string to use as the element's \a id.
			///
			void add(std::string const &msg,std::string const &id,bool selected=false);

			///
			/// Add to the multiple select a new option with the localized display name \a msg, and specify if it is initially
			/// selected. The default is \a false.
			///
			void add(locale::message const &msg,bool selected=false);

			///
			/// Add to the multiple select a new option with the localized display name \a msg, and specify if it is initially
			/// selected (the default is \a false), providing a unique identification string to use as the element's \a id.
			///
			void add(locale::message const &msg,std::string const &id,bool selected=false);

			///
			/// Get the mapping of all the selected items according to the order they where added to the multiple select list.
			///
			std::vector<bool> selected_map();

			///
			/// Get all the selected items ids according to the order they where added to the list. If no
			/// specific id was given, a string like "0", "1"... will be used.
			///
			std::set<std::string> selected_ids();

			///
			/// Get the minimum amount of options that can be chosen. The default is \a 0.
			///
			unsigned at_least();

			///
			/// Set the minimum amount of options that can be chosen. The default is \a 0.
			///
			void at_least(unsigned v);

			///
			/// Get the maximum amount of options that can be chosen. The default is unlimited.
			///
			unsigned at_most();

			///
			/// Set the maximum amount of options that can be chosen. The default is unlimited.
			///
			void at_most(unsigned v);

			///
			/// Same as at_least(1).
			///
			void non_empty();

			///
			/// Get the number of rows used to display the widget. The default is \a 0 -- undefined.
			///
			unsigned rows();

			///
			/// Set the number of rows used to display the widget. The default is \a 0 -- undefined.
			///
			void rows(unsigned n);
			
			virtual void render_input(form_context &context);
			virtual bool validate();
			virtual void load(http::context &context);
			virtual void clear();

		private:
			struct _data;
			booster::hold_ptr<_data> d;

			struct element {
				element();
				element(std::string const &v,locale::message const &msg,bool sel);
				element(std::string const &v,std::string const &msg,bool sel);
				uint32_t selected : 1;
				uint32_t need_translation : 1;
				uint32_t original_select : 1;
				uint32_t reserved : 29;
				std::string id;
				std::string str_option;
				locale::message tr_option;
				friend std::ostream &operator<<(std::ostream &out,element const &el);
			};

			std::vector<element> elements_;
			
			unsigned low_;
			unsigned high_;
			unsigned rows_;
			
		};

		///
		/// \brief This is the base class for "select" like widgets which include dropdown lists
		/// and radio button sets.
		///
		class CPPCMS_API select_base : public base_widget {
		public:
			select_base();
			virtual ~select_base();

			///
			/// Add a new entry to the selection list.
			///
			void add(std::string const &string);

			///
			/// Add to the selection list a new entry with the unique identification string \a id.
			///
			void add(std::string const &string,std::string const &id);

			///
			/// Add a new localized entry to the selection list.
			///
			void add(locale::message const &msg);

			///
			/// Add to the selection list a new localized entry with the unique identification string \a id.
			///
			void add(locale::message const &msg,std::string const &id);

			///
			/// Return the number of the selected entry in the list. Entries are numbered starting from 0. -1 indicates that nothing
			/// was selected.
			/// 
			int selected();

			///
			/// Return the identification string of the selected entry in the list. An empty string indicates that 
			/// nothing was selected.
			///
			std::string selected_id();

			///
			/// Select the entry numbered \a no.
			///
			void selected(int no);

			///
			/// Select the entry with the identification string \a id.
			///
			void selected_id(std::string id);

			///
			/// Require that one item in the list should be selected (for the validation).
			///
			void non_empty();
			
			virtual void render_input(form_context &context) = 0;
			virtual bool validate();
			virtual void load(http::context &context);
			virtual void clear();

		protected:
			struct CPPCMS_API element {
				
				element();
				element(std::string const &v,locale::message const &msg);
				element(std::string const &v,std::string const &msg);
				element(element const &);
				element const &operator=(element const &);
				~element();

				uint32_t need_translation : 1;
				uint32_t reserved : 31;
				std::string id;
				std::string str_option;
				locale::message tr_option;

			private:
				struct _data;
				booster::copy_ptr<_data> d;

			};

			std::vector<element> elements_;

		private:
			struct _data;
			booster::hold_ptr<_data> d;

			int selected_;
			int default_selected_;

			uint32_t non_empty_ : 1;
			uint32_t reserverd  : 32;
		};

		///
		/// \brief The widget that uses a drop-down list for selection.
		///
		class CPPCMS_API select : public select_base {
		public:
			select();
			virtual ~select();
			virtual void render_input(form_context &context);

		private:
			struct _data;
			booster::hold_ptr<_data> d;
		};

		///
		/// \brief The widget that uses a set of radio buttons..
		///
		class CPPCMS_API radio : public select_base {
		public:
			radio();
			virtual ~radio();
			virtual void render_input(form_context &context);
			
			///
			/// Return the rendering order
			///
			bool vertical();

			///
			/// Set rendering order of the list one behind other (default)
			/// or in same line.
			///
			/// Baiscally it defines whether the radio buttons should
			/// appear in row (false) or in column (true).
			///
			void vertical(bool);

		private:
			uint32_t vertical_ : 1;
			uint32_t reserved_ : 31;

			struct _data;
			booster::hold_ptr<_data> d;
		};

		///
		/// \brief This class represents a file upload form entry.
		///
		/// If the file was not uploaded, set() will be false and 
		/// an attempt to access the member function value() will throw.
		///
		class CPPCMS_API file : public base_html_input {
		public:
			///
			/// Ensure that a file is uploaded (for the validation).
			///
			void non_empty();

			///
			/// Set the minimum and maximum limits for the file size. Note: max == -1 indicates that there
			/// is no maximum limit; min==0 indicates that there is no minimum limit.
			///
			///
			void limits(int min,int max);

			///
			/// Get the minimum and maximum size limits.
			///
			std::pair<int,int> limits();

			///
			/// Set the filename validation pattern. For example ".*\\.(jpg|jpeg|png)".
			///
			/// Please, note that it is a good idea to check magic numbers as well
			/// and not rely on file name only.
			///
			/// See add_valid_magic() function
			///
			void filename(booster::regex const &fn);

			///
			/// Get the regular expression for the filename validation.
			///
			booster::regex filename();
			
			///
			/// Validate or not the filename's charset. The default is \a true.
			///
			void validate_filename_charset(bool);

			///
			/// Get the validation option for the filename's charset.
			///
			bool validate_filename_charset();

			///
			/// Get the uploaded file. This throws cppcms_error if set() == false, i.e. if no file
			/// was uploaded.
			///
			booster::shared_ptr<http::file> value();

			///
			/// Set the required file mime type.
			///
			void mime(std::string const &);

			///
			/// Set the regular expression to validate the mime type.
			///
			void mime(booster::regex const &expr);

			///
			/// Add a string that represents a valid magic number that shoud exist on begging of file.
			///
			/// By default no tests are performed.
			///
			void add_valid_magic(std::string const &);

			virtual void load(http::context &context);
			virtual void render_value(form_context &context);
			virtual bool validate();
			
			file();
			~file();

		private:

			int size_min_;
			int size_max_;
			
			std::vector<std::string> magics_;
			
			std::string mime_string_;
			booster::regex mime_regex_;
			booster::regex filename_regex_;

			uint32_t check_charset_ : 1;
			uint32_t check_non_empty_ : 1;
			uint32_t reserved_ : 30;

			booster::shared_ptr<http::file> file_;

			struct _data;
			booster::hold_ptr<_data> d;
		};


		///
		/// \brief Submit button widget.
		///
		class CPPCMS_API submit : public base_html_input {
		public:
			submit();
			~submit();
			
			///
			/// Return true if this specific button was pressed.
			///
			bool value();

			///
			/// Set the text on the button.
			///
			void value(std::string val);

			///
			/// Set the text on the button.
			///
			void value(locale::message const &msg);

			virtual void render_value(form_context &context);
			virtual void load(http::context &context);

		private:
			struct _data;
			booster::hold_ptr<_data> d;
			bool pressed_;
			locale::message value_;
		};


		

	} // widgets


} //cppcms

#endif // CPPCMS_FORM_H
