#ifndef BOOSTER_NOWIDE_CONVERT_H
#define BOOSTER_NOWIDE_CONVERT_H

#include <booster/config.h>
#include <stdexcept>

#if defined(BOOSTER_WIN_NATIVE) || defined(BOOSTER_DOXYGEN_DOCS)
namespace booster {
	namespace nowide {

		///
		/// \brief This exception is thrown if invalid UTF-8 or UTF-16 is given as input
		///
		class BOOSTER_API bad_utf : public std::runtime_error {
		public:
			bad_utf();
		};

		///
		/// Convert between UTF-16 and UTF-8 string, implemented only on Windows platform
		///
		BOOSTER_API std::string convert(wchar_t const *s);
		///
		/// Convert between UTF-8 and UTF-16 string, implemented only on Windows platform
		///
		BOOSTER_API std::wstring convert(char const *s);
		///
		/// Convert between UTF-16 and UTF-8 string, implemented only on Windows platform
		///
		inline std::string convert(std::wstring const &s) 
		{
			return convert(s.c_str());
		}
		///
		/// Convert between UTF-8 and UTF-16 string, implemented only on Windows platform
		///
		inline std::wstring convert(std::string const &s) 
		{
			return convert(s.c_str());
		}

	} // nowide
} // booster
#endif

#endif
