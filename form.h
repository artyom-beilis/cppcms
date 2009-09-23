#ifndef CPPCMS_FORM_H
#define CPPCMS_FORM_H

#include "defs.h"
#include "noncopyable.h"

#include <string>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <ostream>
#include <sstream>
#include "http_context.h"
#include "http_request.h"
#include "http_response.h"
#include "copy_ptr.h"
#include "cppcms_error.h"
#include "util.h"
#include "regex.h"

namespace cppcms {

	namespace widgets {
		class base_widget;
	}

	///
	/// \brief This class is base class of any form or form-widget used in CppCMS.
	///
	/// It provides abstract basic operations that every widget or form should implement
	///

	class CPPCMS_API base_form {
	public:
		enum {
			as_html = 0,	///< render form/widget as ordinary HTML
			as_xhtml= 1,	///< render form/widget as XHTML

			error_with    = 0 << 1, ///< display error message for widgets that failed validation
			error_without = 1 << 1, ///< do not display error message for invalid widgets

			as_p	= 0 << 2, ///< Render each widget using paragraphs
			as_table= 1 << 2, ///< Render each widget using table
			as_ul 	= 2 << 2, ///< Render each widget using unordered list
			as_dl	= 3 << 2, ///< Render each widget using definitions list
			as_space= 4 << 2  ///< Render each widget using simple blank space separators

			// to be extended
		};


		///
		/// Render the widget to std::ostream \a output with control flags \a flags.
		/// Usually this function is called directly by template rendering functions
		///

		virtual void render(std::ostream &output,unsigned int flags) = 0;


		///
		/// Load the information of form from the provided http::context \a context.
		/// User calls this function for loading all information from the raw POST/GET
		/// form to internal widget representation.
		///

		virtual void load(http::context &context) = 0;

		///
		/// Validate the form according to defined rules. If all checks are OK
		/// true is returned. If some widget or form fails, false is returned.
		///

		virtual bool validate() = 0;

		///
		/// Clear the form from all user provided data.
		///
		virtual void clear() = 0;


		base_form();
		virtual ~base_form();
	};

	///
	/// \brief The \a form is a container that used to collect other widgets and forms to single unit
	///
	/// Generally various widgets and forms are combined into single form in order to simplify rendering
	/// and validaion of forms that include more then one widget
	///

	class CPPCMS_API form :	public util::noncopyable,
				public base_form
	{
	public:


		form();
		virtual ~form();


		///
		/// Render all widgets and sub-forms to \a output, using
		/// base_form \a flags
		///

		virtual void render(std::ostream &output,unsigned int flags);


		///
		/// Load all information from widgets from http::context \a cont
		///
		virtual void load(http::context &cont);

		///
		/// validate all subforms and widgets. If at least one of them fails,
		/// false is returned. Otherwise, true is returned.
		///

		virtual bool validate();

		///
		/// Clear all subforms and widgets for all loaded data.
		///
		virtual void clear();

		///
		/// adds \a subform to form, the ownership is not transferred to
		/// to the parent
		///

		void add(form &subform);

		///
		/// add \a subform to form, the owenrshit is transferred to
		/// the parent and subform will be destroyed together with
		/// the parent
		///

		void attach(form *subform);

		///
		/// adds \a widget to form, the ownership is not transferred to
		/// to the parent
		///

		void add(widgets::base_widget &widget);

		///
		/// add \a widget to form, the owenrshit is transferred to
		/// the parent the widget will be destroyed together with
		/// the parent form
		///

		void attach(widgets::base_widget *widget);

		///
		/// Shortcut to \a add
		///
		inline form &operator + (form &f)
		{
			add(f);
			return *this;
		}
		
		///
		/// Shortcut to \a add
		///
		inline form &operator + (widgets::base_widget &f)
		{
			add(f);
			return *this;
		}
		///
		/// \brief Input iterator that is used to iterate over all widgets of the form
		///
		/// This class is mainly used by templates framework for widgets rendering. It
		/// walks on all widgets and subforms recursively.
		///
		/// Note: it walks over widgets only:
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
			iterator();
			~iterator();
			iterator(iterator const &other);
			iterator const &operator = (iterator const &other);

			widgets::base_widget *operator->() const
			{
				return get();
			}
			widgets::base_widget &operator*() const
			{
				return *get();
			}

			bool operator==(iterator const &other) const
			{
				return equal(other);
			}
			bool operator!=(iterator const &other) const
			{
				return !equal(other);
			}

			iterator operator++(int unused)
			{
				iterator tmp(*this);
				next();
				return tmp;
			}
			iterator &operator++()
			{
				next();
				return *this;
			}


		private:

			friend class form;

			bool equal(iterator const &other) const;
			void next();
			widgets::base_widget *get() const;

			struct data;
			util::copy_ptr<data> d;

		};

		///
		/// Returns an iterator to the first widget.
		///
		iterator begin();

		///
		/// Returns the end-iterator for walking over all widgets.
		///
		iterator end();


	private:
		friend class iterator;

		struct data;
		// Widget and ownership true mine
		typedef std::pair<base_form *,bool> widget_type;
		std::vector<widget_type> elements_;
		util::hold_ptr<data> d;
	};



	namespace widgets {

		///
		/// \brief this class is the base class of all renderable widgets that can be
		/// used together with forms
		///
		/// All cppcms widgets are derived from this class. User, who want to create 
		/// its own custom widgets must derive them from this class
		///

		class CPPCMS_API base_widget : 	
			public base_form,
			public util::noncopyable
		{
		public:

			///
			/// Default constructor
			///
			base_widget();

			///
			/// Construct and set \a name attribute of input html form.
			/// Same as construct object and call name(v);
			/// 
			base_widget(std::string name);

			///
			/// Construct widget and set \a name attribute and short description
			/// text that would be shown near widget \a msg.
			/// 
			
			base_widget(std::string name,std::string msg);
			
			virtual ~base_widget();
			
			/// 
			/// Check if a value was assigned to widget. Usually becomes true
			/// when user assignees value to widget or the widget is loaded.
			/// 
			bool set();

			///
			/// After executing validation, each widget can be tested for validity
			///
			bool valid();

			///
			/// Get html id attribute
			///
			std::string id();

			///
			/// Get html name attribute
			/// 
			std::string name();

			///
			/// Set short message that would be displayed near the widget
			/// 
			std::string message();

			///
			/// Get associated error message that would be displayed near the widget
			/// if widget validation failed.
			/// 
			std::string error_message();

			///
			/// Get long description for specific widget
			///

			std::string help();

			///
			/// Get disabled html attribute
			///

			bool disabled();

			///
			/// Set/Unset disabled html attribute
			///

			void disabled(bool);

			///
			/// Get the general user defined attributes string that can be added to widget
			/// 
			std::string attributes_string();

			///
			/// Set the existence of content for widget. By default the widget is not set.
			/// Any value fetch from "unset" widget by convention should throw an exception
			/// Calling set with true -- changes state to "set" and with false to "unset"
			///
			
			void set(bool);

			///
			/// Set validity state of widget. By default the widget is valid. When it
			/// passes validation its validity state is changed by calling this function
			///
			/// Note: widget maybe not-set but still valid and it may be set but not-valid
			///

			void valid(bool);

			///
			/// Set html id attribute of the widget
			///
			void id(std::string);

			///
			/// Set html name attribute of the widget. Note: if this attribute
			/// is not set, the widget would not be able to be loaded from POST/GET
			/// data.
			///
			
			void name(std::string);

			///
			/// Set short description for the widget. Generally it is good idea to
			/// define this value.
			///
			/// Short message can be also set using base_widget constructor
			///
			void message(std::string);

			///
			/// Set error message that is displayed for invalid widgets.
			///
			/// If it is not set, simple "*" is shown
			///
			void error_message(std::string);

			///
			/// Set longer help message that describes this widget
			///
			void help(std::string);

			///
			/// Set general html attributes that are not supported
			/// directly. For example:
			///
			/// \code
			///  my_widget.attributes_string("style='direction:rtl' onclick='return foo()'");
			/// \endcode
			///
			/// This string is inserted as-is just behind render_input_start
			/// 
			void attributes_string(std::string v);


			///
			/// Render full widget with error messages and decorations as paragraphs
			/// or table elements to \a output
			///

			virtual void render(std::ostream &output,unsigned int flags);

			///
			/// This is a virtual member function that should be implemented by each widget
			/// 
			/// It executes actual rendering of the input HTML form up to the position where
			/// user can insert its own data in HTML templates
			///
			/// For example, render_input_start would render:
			/// \code
			///  <input type="text" value="yossi"
			/// \endcode
			///
			/// and the render_input_end would render:
			/// \code
			///   />
			/// \endcode
			/// 
			/// The html template designer can write:
			/// \code
			///   <% formstart wiget %> style="text:align-right" <% formend widget %>
			/// \endcode
			/// 
			/// And the additional html code would be inserted withing the widget.
			///
			virtual void render_input_start(std::ostream &output,unsigned flags) = 0;

			///
			/// See \a render_input_start
			/// 
			virtual void render_input_end(std::ostream &output,unsigned flags) = 0;

			///
			/// Clean the form. Calls set(false) as well
			///
			virtual void clear();

			///
			/// Validate form. If not overridden it sets widget to valid
			///
			virtual bool validate();

		private:
			std::string id_;
			std::string name_;
			std::string message_;
			std::string error_message_;
			std::string help_;
			std::string attr_;

			uint32_t is_valid_  : 1;
			uint32_t is_set_ : 1;
			uint32_t is_disabled_ : 1;
			uint32_t reserverd_ : 29;

			struct data;
			util::hold_ptr<data> d;
		};

		///
		/// \brief this is the widget that is used as base for text input field representation
		///
		/// This widget is used as base class for other widgets that are used for
		/// text input like: text, textarea, etc.
		///
		/// This widget does much more then reading simple filed data from the POST
		/// or GET form, it performs charset validation and if required conversion
		/// to and from Unicode charset to locale charset.
		///


		class CPPCMS_API base_text : public base_widget {
		public:
			
			///
			/// Create an empty widget
			/// 
			base_text();

			///
			/// Create a widget with http attribute name - \a name
			///
			base_text(std::string name);

			///
			/// Create a widget with http attribute name - \a name
			/// and short description \a msg.
			///
			base_text(std::string name,std::string msg);

			~base_text();

			///
			/// Get the string that contains input value of the widget
			/// the string is returned in locale specific representation.
			/// i.e. if the locale is ru_RU.ISO8859-5 (8 bit encoding)
			/// then the string is encoded in ISO-8859-5 encoding.
			/// If the locale is ru_RU.UTF-8 that the string is encoded
			/// in UTF-8.
			///
			
			
			std::string value();
			
			///
			/// Get the string that contains input value of the widget
			/// converting the string from the locale \a loc to
			/// UTF-8 Unicode encoding. If the locale uses UTF-8
			/// natively it is just copied without conversion.
			///

			std::string value(std::locale const &loc);

			
			///
			/// Set the widget content before rendering, the value \a v 
			/// is assumed to be encoding according to current locale.
			/// It should be encoded appropriately.
			///
			/// For example. If the locale is ru_RU.ISO8859-5, then \a v
			/// should be encoded as ISO-8859-5 if it is ru_RU.UTF-8
			/// then it should be encoded as UTF-8 string.
			///

			void value(std::string v);
			
			///
			/// Set the widget content before rendering, the value \a v 
			/// is assumed to be encoding according to current locale.
			/// It should be encoded appropriately.
			///
			/// For example. If the locale is ru_RU.ISO8859-5, then \a v
			/// should be encoded as ISO-8859-5 if it is ru_RU.UTF-8
			/// then it should be encoded as UTF-8 string.
			///

			void value(char const *str)
			{
				value(std::string(str));
			}

			///
			/// Set the widget content before rendering, to value \a v,
			/// where string \a v is encoded in UTF-8 Unicode encoding,
			/// and it is converted to locale specific encoding defined
			/// by locale \a loc.
			///
			/// If the locale uses UTF-8 encoding natively then the string
			/// is just copied.
			///
			void value(std::string v,std::locale const &loc);

			
			///
			/// Acknowledge the validator that this text widget should contain some text.
			/// similar to limits(1,-1)
			///
			void non_empty();
			///
			/// Set minimum and maximum limits for text size. Note max == -1 indicates that there
			/// is no maximal limit, min==0 indicates that there is no minimal limit.
			///
			/// Note: these numbers represent the length in Unicode code points (even if the encoding
			/// is not Unicode). If character set validation is disabled, then these number represent
			/// the number of octets in the string.
			///
			void limits(int min,int max);

			///
			/// Get minimal and maximal size limits, 
			///
			
			std::pair<int,int> limits();

			///
			/// Acknowledge the validator if it should not check the validity of the charset.
			/// Default -- enabled
			///
			/// Generally you should not use this option unless you want to load some raw data as
			/// form input, or the character set is different from the defined in locale.
			///
			void validate_charset(bool );
			
			///
			/// Returns true if charset validation is enabled.
			///

			bool validate_charset();

			virtual void render_input_start(std::ostream &output,unsigned flags) = 0;
			virtual void render_input_end(std::ostream &output,unsigned flags) = 0;
			
			///
			/// Validate the widget content according to rules and charset encoding.
			///
			/// Notes:
			/// 
			/// -  The charset validation is very efficient for variable length UTF-8 encoding, 
			///    and most popular fixed length ISO-8859-*, windows-125* and koi8* encodings, for other
			///    encodings iconv conversion is used for actual validation.
			/// -  Special characters (that not allowed in HTML) are assumed as forbidden, even they are
			///    valid code points (like NUL = 0 or DEL=127).
			/// 
			virtual bool validate();

			///
			/// Load the widget for http::context. It used the locale given in the context for
			/// validation of text.
			///
			virtual void load(http::context &);
		private:
			std::string value_;
			int low_;
			int high_;
			bool validate_charset_;
			size_t code_points_;
			struct data;
			util::hold_ptr<data> d;
		};

		///
		/// \brief This class represents html input of type text
		/// 
		
		class CPPCMS_API text : public base_text 
		{
		public:
			///
			/// Create an empty text widget
			/// 
			text();

			///
			/// Create a text widget with http attribute name - \a name
			///
			text(std::string name);

			///
			/// Create a text widget with http attribute name - \a name
			/// and short description \a msg.
			///
			text(std::string name,std::string msg);

			///
			/// Set html attribute size of the widget
			///

			void size(int n);

			///
			/// Get html attribute size of the widget, -1 undefined
			///

			int size();

			~text();


			virtual void render_input_start(std::ostream &output,unsigned flags);
			virtual void render_input_end(std::ostream &output,unsigned flags);
		protected:
			void type(std::string str);
		private:
			int size_;
			std::string type_;
			struct data;
			util::hold_ptr<data> d;
		};

		class CPPCMS_API textarea : public base_text 
		{
		public:
			///
			/// Create an empty text widget
			/// 
			textarea();

			///
			/// Create a text widget with http attribute name - \a name
			///
			textarea(std::string name);

			///
			/// Create a text widget with http attribute name - \a name
			/// and short description \a msg.
			///
			textarea(std::string name,std::string msg);

			~textarea();

			///
			/// Get number of rows in textarea -- default -1 -- undefined
			///
			int rows();
			///
			/// Get number of columns in textarea -- default -1 -- undefined
			///
			int cols();

			///
			/// Set number of rows in textarea
			///
			void rows(int n);
			///
			/// Set number of columns in textarea
			///
			void cols(int n);

			virtual void render_input_start(std::ostream &output,unsigned flags);
			virtual void render_input_end(std::ostream &output,unsigned flags);
		private:
			int rows_,cols_;

			struct data;
			util::hold_ptr<data> d;
		};


		///
		/// \brief Widget for number input. It is template class that assumes that T is number
		///

		template<typename T>
		class number: public base_widget {
		public:

			number() 
			{
				init();
			}

			///
			/// Construct widget with html attribute name \a name
			///
			number(std::string name) : base_widget(name)
			{
				init();
			}
			///
			/// Construct widget with html attribute name \a name and description \a msg
			///
			number(std::string name,std::string msg) : base_widget(name,msg)
			{
				init();
			}

			///
			/// Defines that this widget should have some value
			///
			void non_empty()
			{
				non_empty_=true;
			}


			///
			/// Get loaded widget value
			///
			T value()
			{
				if(!set())
					throw cppcms_error("Value not loaded");
				return value_;
			}

			///
			/// Set widget value
			///
			void value(T v)
			{
				set(true);
				value_=v;
			}

			/// 
			/// Set minimal input number value
			///
			void low(T a)
			{
				min_=a;
				check_low_=true;
				non_empty();
			}

			/// 
			/// Set maximal input number value
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
			/// Render first part of widget
			///
			
			virtual void render_input_start(std::ostream &output,unsigned flags)
			{
				output<<"<input ";
			}

			///
			/// Render second part of widget
			///
			
			virtual void render_input_end(std::ostream &output,unsigned flags)
			{
				std::string v=id();
				if(!v.empty())
					output<<"id=\""<<v<<"\" ";
				v=name();
				if(!v.empty())
					output<<"name=\""<<v<<"\" ";

				output<<" type=\"text\" ";

				if(set())
					output<<"value=\""<<value_<<"\" ";
				else
					output<<"value=\""<<util::escape(loaded_string_)<<"\" ";

				if(disabled()) {
					if(flags & as_xhtml) 
						output<<"disabled=\"disabled\" ";
					else
						output<<"disabled ";
				}

				if(flags & as_xhtml)
					output<<"/>";
				else
					output<<">";
			}

			virtual void clear()
			{
				base_widget::clear();
				loaded_string_.clear();
			}

			///
			/// Load widget data
			///
			
			virtual void load(http::context &context)
			{
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
					ss.imbue(context.locale().get());
					ss>>value_;
					if(ss.fail() || !ss.eof())
						valid(false);
					else
						set(true);
				}
			}
			
			///
			/// Validate widget
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
			void init()
			{
				check_low_=check_high_=non_empty_=false;
			}

			T min_,max_,value_;

			bool check_low_;
			bool check_high_;
			bool non_empty_;
			std::string loaded_string_;
		};

		/// 
		/// \brief The password widget is a simple text widget with some different
		//  
		
		class CPPCMS_API password: public text {
		public:
			password();
			password(std::string name);
			password(std::string name,std::string msg);
			~password();

			void check_equal(password &p2);
			virtual bool validate();
		private:
			struct data;
			util::hold_ptr<data> d;
			password *password_to_check_;
		};

		

		class CPPCMS_API regex_field : public text {
		public:
			regex_field(util::regex const &e);
			regex_field(util::regex const &e,std::string name);
			regex_field(util::regex const &e,std::string name,std::string msg);
			~regex_field();
			
			virtual bool validate();
		private:
			util::regex const *expression_;
			struct data;
			util::hold_ptr<data> d;
		};

		class CPPCMS_API email : public regex_field {
		public:
			email();
			~email();
			email(std::string name);
			email(std::string name,std::string msg);
		private:
			static util::regex const email_expression_;
			struct data;
			util::hold_ptr<data> d;
		};
/*
		class checkbox: public base_widget {
		public:
			string input_value;
			bool value;
			checkbox(string name="",string msg="") : base_widget(name,msg),input_value("1")
			{
				set(false);
			};
			virtual string render_input(int how);
			void set(bool v) { value=v; is_set=true; };
			void set(string const &s) { input_value=s; };
			bool get() { return value; };
			virtual void load(cgicc::Cgicc const &cgi);
		};

		class select_multiple : public base_widget {
			int min;
		public:
			int size;
			select_multiple(string name="",int s=0,string msg="") : base_widget(name,msg),min(-1),size(s) {};
			set<string> chosen;
			map<string,string> available;
			void add(string val,string opt,bool selected=false);
			void add(int val,string opt,bool selected=false);
			void add(string v,bool s=false) { add(v,v,s); }
			void add(int v,bool s=false) {
				ostringstream ss;
				ss<<v;
				add(v,ss.str(),s);
			}
			set<string> &get() { return chosen; };
			set<int> geti();
			void set_min(int n) { min=n; };
			virtual string render_input(int how);
			virtual bool validate();
			virtual void load(cgicc::Cgicc const &cgi);
			virtual void clear();
		};

		class select_base : public base_widget {
		public:
			string value;
			struct option {
				string value;
				string option;
			};
			list<option> select_list;
			select_base(string name="",string msg="") : base_widget(name,msg){};
			void add(string value,string option);
			void add(string v) { add(v,v); }
			void add(int value,string option);
			void add(int v) {
				ostringstream ss;
				ss<<v;
				add(v,ss.str());
			}
			void set(string value);
			void set(int value);
			string get();
			int geti();
			virtual bool validate();
			virtual void load(cgicc::Cgicc const &cgi);
		};

		class select : public select_base {
			int size;
		public:
			select(string n="",string m="") : select_base(n,m),size(-1) {};
			void set_size(int n) { size=n;}
			virtual string render_input(int how);
		};

		class radio : public select_base {
			bool add_br;
		public:
			radio(string name="",string msg="") : select_base(name,msg),add_br(false) {}
			void set_vertical() { add_br=true; }
			virtual string render_input(int how);
		};

		class hidden : public text {
		public:
			hidden(string n="",string msg="") : text(n,msg) { set_nonempty(); };
			virtual string render(int how);
		};

		class submit : public base_widget {
		public:
			string value;
			bool pressed;
			submit(string name="",string button="",string msg="") : base_widget(name,msg), value(button),pressed(false) {};
			virtual string render_input(int);
			virtual void load(cgicc::Cgicc const &cgi);
		};  */

	} // widgets


} //cppcms

#endif // CPPCMS_FORM_H
