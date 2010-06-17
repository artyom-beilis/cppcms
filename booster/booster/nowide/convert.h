#ifndef BOOSTER_NOWIDE_CONVERT_H
#define BOOSTER_NOWIDE_CONVERT_H

#include <booster/config.h>
#include <stdexcept>

#if defined(BOOSTER_WIN_NATIVE) || defined(BOOSTER_DOXYGEN_DOCS)
namespace booster {
	namespace nowide {

		class BOOSTER_API bad_utf : public std::runtime_error {
		public:
			bad_utf();
		};

		BOOSTER_API std::string convert(wchar_t const *s);
		BOOSTER_API std::wstring convert(char const *s);
		inline std::string convert(std::wstring const &s) 
		{
			return convert(s.c_str());
		}
		inline std::wstring convert(std::string const &s) 
		{
			return convert(s.c_str());
		}

	} // nowide
} // booster
#endif

#endif
