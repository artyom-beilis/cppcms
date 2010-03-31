#ifndef CPPCMS_JSON_H
#define CPPCMS_JSON_H

#include "defs.h"
#include "copy_ptr.h"

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <typeinfo>


namespace cppcms {
namespace json {




	class value;

	struct null {};
	struct undefined {};

	inline bool operator==(undefined const &l,undefined const &r) {return true;}
	inline bool operator!=(undefined const &l,undefined const &r) {return false;}
	inline bool operator==(null const &l,null const &r) {return true;}
	inline bool operator!=(null const &l,null const &r) {return false;}


	typedef std::vector<value> array;
	typedef std::map<std::string,value> object;

	template<typename T>
	struct traits {
		static void set(value &v,T const &in);
	};


	typedef enum {
		is_undefined,
		is_null,
		is_boolean,
		is_number,
		is_string,
		is_object,
		is_array
	} json_type;


	enum {
		compact = 0,
		readable = 1
	};
	
	class CPPCMS_API bad_value_cast : public std::bad_cast {
	public:

		bad_value_cast(); 
		bad_value_cast(std::string const &s);
		bad_value_cast(std::string const &s,json_type actual);
		bad_value_cast(std::string const &s,json_type expected, json_type actual);

		virtual ~bad_value_cast() throw();
		virtual const char* what() const throw();
	private:
		std::string msg_;
	};
	
	class value;

	std::istream CPPCMS_API &operator>>(std::istream &in,value &v);
	std::ostream CPPCMS_API &operator<<(std::ostream &out,value const &v);
	std::ostream CPPCMS_API &operator<<(std::ostream &out,json_type);

	class CPPCMS_API value {
	public:

		json_type type() const;
		
		bool is_undefined() const;
		bool is_null() const;

		bool const &boolean() const;
		double const &number() const;
		std::string const &str() const;
		json::object const &object() const;
		json::array const &array() const;

		bool &boolean();
		double &number();
		std::string &str();
		json::object &object();
		json::array &array();

		void undefined();
		void null();

		void boolean(bool);
		void number(double );
		void str(std::string const &);
		void object(json::object const &);
		void array(json::array const &);


		template<typename T>
		T get_value() const
		{
			return traits<T>::get(*this);
		}
		
		template<typename T>
		void set_value(T const &v)
		{
			traits<T>::set(*this,v);
		}


		// returns empty if not found
		value const &find(std::string) const; 		

		// throws if not found
		value const &at(std::string) const;  
		value &at(std::string);

		// sets 
		void at(std::string,value const &v);

		

		template<typename T>
		value(T const &v)
		{
			set_value(v);
		}

		///
		/// Returns the type of variable in path, if not found returns undefined
		///
		json_type type(std::string path) const
		{
			return find(path).type();
		}

		///
		/// Set value at specific path
		///
		template<typename T>
		void set(std::string path,T const &v)
		{
			at(path,value(v));
		}

		std::string get(std::string path,char const *def) const
		{
			value const &v=find(path);
			if(v.is_undefined())
				return def;
			try {
				return v.get_value<std::string>();
			}
			catch(std::bad_cast const &e) {
				return def;
			}
		}

		template<typename T>
		T get(std::string path) const
		{
			return at(path).get_value<T>();
		}

		template<typename T>
		T get(std::string path,T const &def) const
		{
			value const &v=find(path);
			if(v.is_undefined())
				return def;
			try {
				return v.get_value<T>();
			}
			catch(std::bad_cast const &e) {
				return def;
			}
		}

		value &operator[](std::string name);
		value const &operator[](std::string name) const;
		value &operator[](size_t n);
		value const &operator[](size_t n) const;


		std::string save(int how=compact) const;
		void save(std::ostream &out,int how=compact) const;
		bool load(std::istream &in,bool full,int *line_number=0);

		bool operator==(value const &other) const;
		bool operator!=(value const &other) const;


		value(value const &other) :
			d(other.d)
		{
		}
		value const &operator=(value const &other)
		{
			d=other.d;
			return *this;
		}
		value()
		{
		}

		~value()
		{
		}

		void swap(value &other)
		{
			d.swap(other.d);
		}

	private:

		void write(std::ostream &out,int tabs) const;
		void write_value(std::ostream &out,int tabs) const;

		struct data;
		struct copyable {

			data *operator->() { return &*d; }
			data &operator*() { return *d; }
			data const *operator->() const { return &*d; }
			data const &operator*() const { return *d; }

			copyable();
			copyable(copyable const &r);
			copyable const &operator=(copyable const &r);
			~copyable();

			void swap(copyable &other) 
			{
				d.swap(other.d);
			}
		private:
			util::copy_ptr<data> d;
		} d;

		friend struct copyable;
	};


/*****************************************************************

	template<typename T>
	struct traits {
		static T get(value const &v);
		static void set(value &v,T const &in);
		static json::value schema();
	};

*******************************************************************/

	template<typename T1,typename T2>
	struct traits<std::pair<T1,T2> > {
		static std::pair<T1,T2> get(value const &v)
		{
			if(v.object().size()!=2)
				throw bad_value_cast("Object with two members expected");
			std::pair<T1,T2> pair(v.get_value<T1>("first"),v.get_value<T2>("second"));
			return pair;
		}
		static void set(value &v,std::pair<T1,T2> const &in)
		{
			v=json::object();
			v.set_value("first",in.first);
			v.set_value("second",in.second);
		}
		static json::value schema()
		{
			json::value sch;
			sch.set("type","object");
			sch.set("properties.first.type",traits<T1>::schema());
			sch.set("properties.second.type",traits<T1>::schema());
			return sch;
		}
	};

	template<typename T>
	struct traits<std::vector<T> > {
		static std::vector<T> get(value const &v)
		{
			std::vector<T> result;
			json::array const &a=v.array();
			result.resize(a.size());
			for(unsigned i=0;i<a.size();i++) 
				result[i]=a[i].get_value<T>();
			return result;
		}
		static void set(value &v,std::vector<T> const &in)
		{
			v=json::array();
			json::array &a=v.array();
			a.resize(in.size());
			for(unsigned i=0;i<in.size();i++)
				a[i].set_value(in[i]);
		}
		static json::value schema()
		{
			json::value sch;
			sch.set("type","array");
			sch.set("items.type",traits<T>::schema());
			return sch;
		}
	};


	#define CPPCMS_JSON_SPECIALIZE(type,method,sch) \
	template<>					\
	struct traits<type> {				\
		static type get(value const &v)		\
		{					\
			return v.method();		\
		}					\
		static void set(value &v,type const &in)\
		{					\
			v.method(in);			\
		}					\
		static json::value schema()		\
		{					\
			return sch;			\
		}					\
	};

	CPPCMS_JSON_SPECIALIZE(bool,boolean,"boolean");
	CPPCMS_JSON_SPECIALIZE(double,number,"number");
	CPPCMS_JSON_SPECIALIZE(std::string,str,"string");
	CPPCMS_JSON_SPECIALIZE(json::object,object,"object");
	CPPCMS_JSON_SPECIALIZE(json::array,array,"array");

	#undef CPPCMS_JSON_SPECIALIZE

	template<>					
	struct traits<int> {				
		static int get(value const &v)		
		{					
			return static_cast<int>(v.number());
		}					
		static void set(value &v,int const &in)
		{					
			v.number(in);			
		}					
		static json::value schema()
		{		
			return "integer";
		}			
	};
	template<>					
	struct traits<json::null> {				
		static void set(value &v,json::null const &in)
		{					
			v.null();
		}					
	};

	template<int n>					
	struct traits<char[n]> {			
		typedef char array_type[n];
		static void set(value &v,array_type const &in)
		{					
			v.str(in);
		}					
	};

	template<>					
	struct traits<char const *> {			
		static void set(value &v,char const * const &in)
		{					
			v.str(in);
		}					
	};




} // json
} // cppcms

#endif
