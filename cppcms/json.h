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
#ifndef CPPCMS_JSON_H
#define CPPCMS_JSON_H

#include <cppcms/defs.h>
#include <booster/copy_ptr.h>

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <typeinfo>


namespace cppcms {
///
/// \brief This namespace includes all JSON parsing and formatting related classes and functions
///
namespace json {




	class value;

	///
	/// Special object that is convertible to null json value
	///
	struct null {};
	///
	/// Special object that is convertible to undefined json value
	///
	struct undefined {};

	inline bool operator==(undefined const &l,undefined const &r) {return true;}
	inline bool operator!=(undefined const &l,undefined const &r) {return false;}
	inline bool operator==(null const &l,null const &r) {return true;}
	inline bool operator!=(null const &l,null const &r) {return false;}

	///
	/// The json::array - std::vector of json::value's
	///
	typedef std::vector<value> array;
	///
	/// The json::object - std::map of json::value's
	///
	typedef std::map<std::string,value> object;

	#ifdef CPPCMS_DOXYGEN_DOCS

	///
	/// The type traits schema for converting json values to/from orinary objects
	/// i.e. serialization from JSON to C++ object
	///
	template<typename T>
	struct traits {
		///
		/// Convert json value \a v to an object of type T and return it
		///
		static T get(value const &v);
		///
		/// Convert an object \a in to a json::value \a v
		///
		static void set(value &v,T const &in);
	};

	#else
	
	template<typename T>
	struct traits;

	#endif


	///
	/// The type of json value
	///
	typedef enum {
		is_undefined,	///< Undefined value
		is_null,	///< null value
		is_boolean,	///< boolean value
		is_number,	///< numeric value
		is_string,	///< string value
		is_object,	///< object value 
		is_array	///< array value
	} json_type;


	enum {
		compact = 0, ///< Print JSON values in most compact format
		readable = 1 ///< Print JSON values in human readable format (with identention)
	};
	
	///
	/// \brief The error that is thrown in case of bad conversion of json::value to ordinary value
	///
	/// When implementing json::traits for complex classes you are expected to throw this exception
	/// in case of invalid formatting
	///
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

	///
	/// Read json object from input stream
	///
	std::istream CPPCMS_API &operator>>(std::istream &in,value &v);

	///
	/// Write json object to output stream
	///
	std::ostream CPPCMS_API &operator<<(std::ostream &out,value const &v);

	///
	/// Write human readable representation of json_type 
	///
	std::ostream CPPCMS_API &operator<<(std::ostream &out,json_type);

	///
	/// This class is central representation of json objects. It can be a value from any type
	/// including object, array and undefined
	///
	class CPPCMS_API value {
	public:

		///
		/// Get the type of the value
		/// 
		json_type type() const;
		
		///
		/// Returns true if type()==json::is_undefined
		///
		bool is_undefined() const;
		///
		/// Returns true if type()==json::is_null
		///
		bool is_null() const;

		///
		/// Convert value to boolean, throws bad_value_cast if value's type is not boolean
		///
		bool const &boolean() const;
		///
		/// Convert value to double, throws bad_value_cast if value's type is not number
		///
		double const &number() const;
		///
		/// Convert value to strng, throws bad_value_cast if value's type is not string
		///
		std::string const &str() const;
		///
		/// Convert value to json::object, throws bad_value_cast if value's type is not object
		///
		json::object const &object() const;
		///
		/// Convert value to json::array, throws bad_value_cast if value's type is not array
		///
		json::array const &array() const;

		///
		/// Get reference to bool variable that represents this value, throws bad_value_cast if type is invalid
		///
		bool &boolean();
		///
		/// Get reference to double variable that represents this value, throws bad_value_cast if type is invalid
		///
		double &number();
		///
		/// Get reference to string variable that represents this value, throws bad_value_cast if type is invalid
		///
		std::string &str();
		///
		/// Get reference to object variable that represents this value, throws bad_value_cast if type is invalid
		///
 		json::object &object();
		///
		/// Get reference to array variable that represents this value, throws bad_value_cast if type is invalid
		///
		json::array &array();

		///
		/// Set value to undefined type
		///
		void undefined();
		///
		/// Set value to null type
		///
		void null();

		///
		/// Set value to boolean type and assign it
		///
		void boolean(bool);
		///
		/// Set value to numeric type and assign it
		///
		void number(double );
		///
		/// Set value to string type and assign it
		///
		void str(std::string const &);
		///
		/// Set value to object type and assign it
		///
		void object(json::object const &);
		///
		/// Set value to array type and assign it
		///
		void array(json::array const &);


		///
		/// Convert the value to type T, using json::traits, throws bad_value_cast if conversion is not possible
		///
		template<typename T>
		T get_value() const
		{
			return traits<T>::get(*this);
		}
		
		///
		/// Convert the object \a v of type T to the value
		/// 
		template<typename T>
		void set_value(T const &v)
		{
			traits<T>::set(*this,v);
		}

		///
		/// Searches a value in the path \a path
		///
		/// For example if the json::value represents { "x" : { "y" : 10 } }, then find("x.y") would return
		/// a reference to value that hold a number 10, find("x") returns a reference to a value
		/// that holds an object { "y" : 10 } and find("foo") would return value of undefined type.
		///
		value const &find(std::string const &path) const; 		

		///
		/// Searches a value in the path \a path, if not found throw bad_value_cast.
		///
		/// For example if the json::value represents { "x" : { "y" : 10 } }, then find("x.y") would return
		/// a reference to value that hold a number 10, find("x") returns a reference to a value
		/// that holds an object { "y" : 10 } and find("foo") throws
		///
		value const &at(std::string const &path) const;  
		///
		/// Searches a value in the path \a path, if not found throw bad_value_cast.
		///
		/// For example if the json::value represents { "x" : { "y" : 10 } }, then find("x.y") would return
		/// a reference to value that hold a number 10, find("x") returns a reference to a value
		/// that holds an object { "y" : 10 } and find("foo") throws
		///
		value &at(std::string const &path);

		///
		/// Sets the value \a v at the path \a path, if the path invalid, creates it.
		///
		void at(std::string const &path,value const &v);

		
		///
		/// Creates a value from and object \a v of type T
		///
		template<typename T>
		value(T const &v)
		{
			set_value(v);
		}

		///
		/// Returns the type of variable in path, if not found returns undefined
		///
		/// Same as find(path).type()
		///
		json_type type(std::string const &path) const
		{
			return find(path).type();
		}

		///
		/// Convert an object \a v of type T to a value at specific path, same as at(path,value(v))
		///
		template<typename T>
		void set(std::string const &path,T const &v)
		{
			at(path,value(v));
		}

		///
		/// Get a string value from a path \a path. If the path is not invalid or the object
		/// is not of type string at this path, returns \a def instead
		///
		std::string get(std::string const &path,char const *def) const
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
		
		///
		/// Get an object of type T from the path \a path. Throws bad_value_cast if such path does not
		/// exists of conversion can't be done
		///
		template<typename T>
		T get(std::string const &path) const
		{
			return at(path).get_value<T>();
		}

		///
		/// Get an object of type T from the path \a path. Returns \a def if such path does not
		/// exists of conversion can't be done
		///
		template<typename T>
		T get(std::string const &path,T const &def) const
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

		///
		/// Returns a reference to the node \a name of the value. 
		/// For value = {"x",10} and name == "x" return a value that holds 10.
		///
		/// If value is not object it's type set to object.
		/// If such node does not exits, it is created with undefined value
		///
		value &operator[](std::string const &name);

		///
		/// Returns reference to the node \a name of the value.
		/// For value = {"x",10} and name == "x" return a value that holds 10.
		///
		/// If value is not object or such node does not exits, throws bad_value_cast
		///
		value const &operator[](std::string const &name) const;

		///
		/// Returns a reference to \a n 'th entry of the array. If the value is not an array it is reset to array,
		/// of the array is too small it is resized to size of at least n+1
		///
		value &operator[](size_t n);
		///
		/// Returns a reference to \a n 'th entry of array, if the value is not array or n is too big, throws
		/// bad_value_cast
		///
		value const &operator[](size_t n) const;

		///
		/// Convert a value to std::string, if \a how has value \a readable it is converted with indentation
		///
		std::string save(int how=compact) const;
		///
		/// Write a value to std::ostream, if \a how has value \a readable it is converted with indentation
		///
		void save(std::ostream &out,int how=compact) const;
		///
		/// Read a value from std::istream.
		///
		/// Note: only JSON object and JSON array are considered valid values
		///
		/// \param in the std::istream used to read the data
		/// \param full  require EOF once the object is read, otherwise consider it as syntax error
		/// \param line_number  return a number of the line where syntax error occurred
		/// \result returns true if the value was read successfully, otherwise returns false to indicate a syntax error.
		///
		bool load(std::istream &in,bool full,int *line_number=0);

		///
		/// Compare two values objects, return true if they are same
		///
		bool operator==(value const &other) const;
		///
		/// Compare two values objects, return false if they are same
		///
		bool operator!=(value const &other) const;

		///
		/// Copy constructor
		///
		value(value const &other) :
			d(other.d)
		{
		}
		///
		/// Assignment operator
		///
		value const &operator=(value const &other)
		{
			d=other.d;
			return *this;
		}
		///
		/// Default value - creates a value of undefined type
		///
		value()
		{
		}
		
		///
		/// Destructor
		///

		~value()
		{
		}

		///
		/// Swaps two values, does not throw.
		///
		void swap(value &other)
		{
			d.swap(other.d);
		}

	private:

		void write(std::ostream &out,int tabs) const;
		void write_value(std::ostream &out,int tabs) const;

		struct _data;
		struct CPPCMS_API copyable {

			_data *operator->() { return &*d; }
			_data &operator*() { return *d; }
			_data const *operator->() const { return &*d; }
			_data const &operator*() const { return *d; }

			copyable();
			copyable(copyable const &r);
			copyable const &operator=(copyable const &r);
			~copyable();

			void swap(copyable &other) 
			{
				d.swap(other.d);
			}
		private:
			booster::copy_ptr<_data> d;
		} d;

		friend struct copyable;
	};


	
	/// \cond INTERNAL

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
	};


	#define CPPCMS_JSON_SPECIALIZE(type,method) 	\
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
	};

	CPPCMS_JSON_SPECIALIZE(bool,boolean);
	CPPCMS_JSON_SPECIALIZE(double,number);
	CPPCMS_JSON_SPECIALIZE(std::string,str);
	CPPCMS_JSON_SPECIALIZE(json::object,object);
	CPPCMS_JSON_SPECIALIZE(json::array,array);

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
		typedef char vtype[n];
		static void set(value &v,vtype const &in)
		{					
			v.str(in);
		}					
	};
	template<int n>					
	struct traits<char const [n]> {			
		typedef char const vtype[n];
		static void set(value &v,vtype const &in)
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
	
	/// \endcond


} // json
} // cppcms

#endif
