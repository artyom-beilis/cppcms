#ifndef CPPCMS_JSON_H
#define CPPCMS_JSON_H

#include <cppcms/util/copy_ptr.h>
#include <cppcms/config.h>

#include <vector>
#include <map>
#include <string>
#include <ostream>


namespace cppcms {
namespace json {

	class value;

	typedef std::vector<value> array;
	typedef std::map<std::string,value> object;

	typedef enum { 
		is_null,
		is_number,
		is_boolean,
		is_string,
		is_object,
		is_array
	} json_type;
	
	enum {
		utf8 = 0,
		us_ascii = 1,
		compact = 0,
		readable = 4
	};

	class value_data;
	class value {
		void write(std::ostream &out,int tabs,bool utf) const; 
	public:
		
		json_type type() const;
		
		bool is_null() const;
		void null();


		double real() const;
		double real(double) const;
		int integer() const;
		int integer(int) const;
		bool boolean() const;
		bool boolean(bool) const;

		std::string str() const;
		std::wstring wstr() const;
#ifdef CPPCMS_HAVE_CPP0X_UNICODE_CHARRECTERS
		std::u16string u16str() const;
		std::u32string u32str() const;
#endif
		std::string str(std::string) const;
		std::wstring wstr(std::wstring) const;
#ifdef CPPCMS_HAVE_CPP0X_UNICODE_CHARRECTERS
		std::u16string u16str(std::u16string) const;
		std::u32string u32str(std::u32string) const;
#endif

		object const &object() const;
		object &object();
		array const &array() const;
		array &array();

		value const &operator=(double v);
		value const &operator=(bool v);
		value const &operator=(int v);
		value const &operator=(std::string);
		value const &operator=(std::wstring);
		value const &operator=(char const *);
		value const &operator=(wchar_t const *);
#ifdef CPPCMS_HAVE_CPP0X_UNICODE_CHARRECTERS
		value const &operator=(std::u16string);
		value const &operator=(std::u32string);
		value const &operator=(char16_t const *);
		value const &operator=(char32_t const *);
#endif
		value const &operator=(object const &);
		value const &operator=(array const &);

		value &operator[](unsigned pos);
		value const &operator[](unsigned pos) const
		value const &operator()(std::string const &path) const;
		value &operator()(std::string const &path);
		value const &operator[](std::string const &entry) const;
		value &operator[](std::string const &entry);

		std::string save(int how=(utf8 | compact));


		value();
		value(value const &);
		value const &operator=(value const &);
		~value();

	private:
		util::copy_ptr<value_data> d;
	}

	value parse(std::string const &);
	value parse(char const *begin,char const *end);
	std::ostream &operator<<(std::ostream &out,value const &v);

} // json
} // cppcms

#endif
