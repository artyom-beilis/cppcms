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

namespace cppcms {

	class CPPCMS_API base_form {
	public:

		virtual void render(std::ostream &output,unsigned int flags) = 0;
		virtual void load(http::context &) = 0;
		virtual bool validate() = 0;
		virtual void clear() = 0;



		enum {	
			as_html = 0,
			as_xhtml= 1,

			error_with    = 0 << 1
			error_without = 1 << 1,

			as_p	= 0 << 2,
			as_table= 1 << 2,
			as_ul 	= 2 << 2,
			as_dl	= 3 << 2,
			as_space= 4 << 2

			// to be extended
		}

		virtual ~base_form(){}
	};



	class CPPCMS_API form :	public util::noncopyable,
				public base_form 
	{
	public:


		form();
		virtual ~form();


		///
		/// Render all widgets of this form to the \a output, using 
		/// base_form \a flags
		///
		
		virtual void render(std::ostream &output,unsigned int flags);

		virtual void load(http::context &);
		virtual bool validate();
		virtual void clear();

		///
		/// Adds \a subform to form, the ownership is not transferred to
		/// to the parent, it is useful for normal widgets
		///

		void add(base_form &subform);

		///
		/// Add \a subform to form, the owenrshit is transferred to
		/// the parent widget and subform will be destroyed together with
		/// the parent form
		///

		void attach(base_form *subform);

		inline form &operator + (base_form &f)
		{
			append(&f); 
			return *this; 
		}

		class iterator : public std::iterator<std::input_iterator_tag,base_from>
		{
		public:
			iterator();
			~iterator();
			iterator(iterator const &other);
			iterator const &operator = (iterator const &other);

			pointer_type operator->() const
			{
				return get()
			}
			reference operator*() const
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
			
			void equal(iterator const &other) const;
			void next();
			pointer_type get() const;

			util::copy_ptr<data> d;

		};

		iterator begin();
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

		class CPPCMS_API base_widget : public base_form {
		public:

			base_widget(std::string name,std::string msg="");

			base_widget();
			base_widget(base_widget const &other);
			base_widget const &operator = (base_widget const &other);
			virtual ~base_widget();
			
				
			bool set();
			bool valid();
			std::string id();
			std::string name();
			std::string message()
			std::string error_message();
			std::string help();

			void set(bool);
			void valid(bool);
			void id(std::string);
			void name(std::string);
			void message(std::string)
			void error_message(std::string);
			void help(std::string);


			virtual void render(std::ostream &output,unsigned int flags);
			virtual void render_input_start(std::ostream &output,unsigned flags) = 0;
			virtual void render_input_end(std::ostream &output,unsigned flags) = 0;
			virtual void render_error(unsigned flags);
			virtual void clear();
			virtual bool validate();

		private:
			std::string id_;
			std::string name_;
			std::string msg_;
			std::string error_msg_;
			std::string help_;

			bool is_valid_;
			bool is_set_;

			struct data;
			util::copy_ptr<data> d;
		};



		class text : public base_widget {
		protected:
			string type;
			unsigned low;
			int high;
		public:
			string value;
			text(string name="",string msg="") : base_widget(name,msg) , type("text"){ low=0,high=-1; };

			void set_limits(int min,int max) { low=min,high=max; }
			void set_nonempty() { low = 1, high=-1;};
			void set_limits(string e,int min,int max) { error_msg=e; set_limits(min,max); }
			void set_nonempty(string e){ error_msg=e; set_nonempty(); };
			virtual string render_input(int how);
			void set(string const &s);
			string &str();
			string const &get();
			virtual bool validate();
			virtual void load(cgicc::Cgicc const &cgi);
		};





		template<typename T>
		class number: public text {
		public:
			number(string name="",string msg="") :
				text(name,msg),
				value_(0),
				check_low_(false),
				check_high_(false),
				non_empty_(false)
			{
			}
			
			void non_empty()
			{
				non_empty_=true;
			}

			T value()
			{
				if(!set())
					throw cppcms_error("Value not loaded");
				return value_;
			}

			void value(T v)
			{
				set(true);
				value_=v;
			}

			void low(T a)
			{
				min_=a;
				check_low_=true;
				set_nonempty();
			}

			void high(T b)
			{
				max_=b;
				check_high_=true;
				set_nonempty();
			}

			void range(T a,T b)
			{
				set_low(a);
				set_high(b);
			}

			void render_input_start(std::ostream &output,unsigned flags)
			{
				output<<"<input ";
			}

			void render_input_end(std::ostream &output,unsigned flags)
			{
				render_element(output,id(),"id");
				render_element(output,name(),"name");
				output<<" type=\"text\" ";

				if(set())
					output<<"value="<<value_<<"\" ";
				else
					render_element(output,loaded_string_,"value");

				if(flags & as_xhtml)
					outout<<"/>";
				else
					output<<">"
			}

			void load(http::context &context)
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
				}
			}

			bool validate()
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
					valud(false);
					return false;
				}
				return true;
			}

		private:

			void render_element(std::ostream &out,std::string const &v,char const *field)
			{
				if(v.empty())
					return;
				out<<" "<<field<<"=\""<<util::escape(v)<<"\" ";
			}

			T min_,max_,value_;

			bool check_low_;
			bool check_high_;
			bool non_empty_;
			std::string loaded_string_;
		};






		class password: public text {
			password *other;
		public:
			password(string name="",string msg="") : text(name,msg),other(0) {} ;
			void set_equal(password &p2) { other=&p2; } ;
			virtual bool validate();
			virtual string render_input(int how);
		};
		class textarea: public text {
		public:
			int rows,cols;
			textarea(string name="",string msg="") : text(name,msg) { rows=cols=-1; };
			virtual string render_input(int how);
		};

		class regex_field : public text {
			util::regex const *exp;
		public:
			regex_field() : exp(0) {}	
			regex_field(util::regex const &e,string name="",string msg="") : text(name,msg),exp(&e) {}
			virtual bool validate();
		};

		class email : public regex_field {
		public:
			email(string name="",string msg="");
		};

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
		};

	} // widgets


} //cppcms

#endif // CPPCMS_FORM_H
