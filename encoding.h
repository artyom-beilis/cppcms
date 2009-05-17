#ifndef CPPCMS_ENCODING_H
#define CPPCMS_ENCODING_H

#include <string>
#include <map>

namespace cppcms {
	namespace encoding {
		
		typedef bool (*encoding_tester_type)(char const *begin,char const *end);
		
		class iconv_validator;

		class validator {
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
		
		class validators_set {
			friend int test_function(); 
			std::map<std::string,encoding_tester_type> predefined_;
		public:
			validators_set();
			void add(std::string const &encoding,encoding_tester_type tester) 
			{
				predefined_[encoding]=tester;
			}
			validator operator[](std::string) const;
		};

		class converter {
			std::string encoding_;
		public:
			converter(std::string const &encoding);
			converter(converter const &other);
			converter const &operator=(converter const &other);
			~converter();

			std::wstring operator()(std::string const &);
			std::wstring operator()(char const *begin,char const *end);
			std::string operator()(std::wstring const &);
			std::string operator()(wchar_t const *begin,wchar_t const *end);
		};

	}  // encoding
} // cppcms


#endif
