#ifndef CPPCMS_ENCODING_H
#define CPPCMS_ENCODING_H

#include <string>
#include <map>
#include <locale>
#include "defs.h"
			
// For unit test;
int cppcms_validator_test_function(); 

namespace cppcms {
	namespace encoding {
		
		typedef bool (*encoding_tester_type)(char const *begin,char const *end);
		
		class iconv_validator;

		class CPPCMS_API validator {
			std::string charset_;
			encoding_tester_type tester_;
			iconv_validator *iconv_;

		public:

			validator(encoding_tester_type tester);
			validator(std::string charset);

			validator(validator const &other);
			validator const &operator=(validator const &other);


			bool valid(char const *begin,char const *end);
			bool valid(std::string const &str);

			~validator();
		};
		
		class CPPCMS_API validators_set {
			friend int ::cppcms_validator_test_function(); 
			std::map<std::string,encoding_tester_type> predefined_;
		public:
			validators_set();
			void add(std::string const &encoding,encoding_tester_type tester) 
			{
				predefined_[encoding]=tester;
			}
			validator operator[](std::string) const;
		};

		class CPPCMS_API converter {
			std::string encoding_;
		public:
			converter(std::string const &encoding);
			converter(converter const &other);
			converter const &operator=(converter const &other);
			~converter();

			std::wstring operator()(std::string const &);
			std::string operator()(std::wstring const &);
		};
		
		char const *native_unicode_encoding();
		char const *native_wchar_encoding();


		std::string CPPCMS_API to_string(std::wstring const &);
		std::string CPPCMS_API to_string(std::wstring const &,std::locale const &locale);
		std::string CPPCMS_API to_string(std::wstring const &,std::string const &encoding);

		std::wstring CPPCMS_API to_wstring(std::string const &);
		std::wstring CPPCMS_API to_wstring(std::string const &,std::locale const &locale);
		std::wstring CPPCMS_API to_wstring(std::string const &,std::string const &encoding);

	}  // encoding
} // cppcms


#endif
