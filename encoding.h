#ifndef CPPCMS_ENCODING_H
#define CPPCMS_ENCODING_H

#include <string>
#include <map>
#include <locale>
#include "defs.h"
#include "hold_ptr.h"
#include "copy_ptr.h"
#include "noncopyable.h"
#include "refcounted.h"
#include "intrusive_ptr.h"
			
// For unit test;
int cppcms_validator_test_function(); 

namespace cppcms {
	namespace encoding {
		
		typedef bool (*encoding_tester_type)(char const *begin,char const *end);
		
		class iconv_validator;

		class CPPCMS_API validator {
		public:
			validator(encoding_tester_type tester);
			validator(std::string charset);

			validator(validator const &other);
			validator const &operator=(validator const &other);


			bool valid(char const *begin,char const *end);
			bool valid(std::string const &str);

			~validator();
		private:
			std::string charset_;
			encoding_tester_type tester_;
			iconv_validator *iconv_;

			struct data;
			util::copy_ptr<data> d;
		};
		
		class CPPCMS_API validators_set : 
			public refcounted,
			public util::noncopyable
		{
		public:
			validators_set();
			~validators_set();

			void add(std::string const &encoding,encoding_tester_type tester);
			validator operator[](std::string) const;
		private:
			struct data;
			util::hold_ptr<data> d;
			friend int ::cppcms_validator_test_function(); 
			std::map<std::string,encoding_tester_type> predefined_;
		};

		char const CPPCMS_API *native_utf32_encoding();
		char const CPPCMS_API *native_utf16_encoding();
		char const CPPCMS_API *native_wchar_encoding();

		class CPPCMS_API converter : util::noncopyable {
		public:
			converter(std::string charset);
			~converter();

			std::string to_utf8(char const *begin,char const *end);
			std::basic_string<uint16_t> to_utf16(char const *begin,char const *end);
			std::basic_string<uint32_t> to_utf32(char const *begin,char const *end);

			std::string from_utf8(char const *begin,char const *end);
			std::string from_utf16(uint16_t const *begin,uint16_t const *end);
			std::string from_utf32(uint32_t const *begin,uint32_t const *end);
		private:
			std::string charset_;
			struct data;
			util::hold_ptr<data> d;
		};


	}  // encoding
} // cppcms


#endif
