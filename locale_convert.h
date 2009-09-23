#ifndef CPPCMS_LOCALE_ICU_H
#define CPPCMS_LOCALE_ICU_H

#include "defs.h"
#include "config.h"
#include "hold_ptr.h"
#include <locale>


namespace cppcms {
namespace locale {

	class convert_impl;

	///
	/// \brief This class provides basic unicode aware string manipulation utilities
	///
	/// If CppCMS is compiled with ICU support it performs these operations using ICU
	/// library, otherwise it uses std::locale facets for these operations, so some
	/// functionality may have lower quality or not supported at all
	///

	class CPPCMS_API convert : public std::locale::facet {
	public:
		static std::locale::id id;

		typedef enum {	norm_default,	/// < Default Unicode normalization as provided by ICU
				norm_nfc,	/// < NFC Unicode normalization
				norm_nfd,	/// < NFD Unicode normalization
				norm_nfkc,	/// < NFKC Unicode normalization
				norm_nfkd	/// < NFKD Unicode normalization
		} norm_type;
		

		///
		/// Create convert facet.
		///
		/// \a charset, \a icu_locale and \a info facets should be assigned to \a source
		/// 

		convert(std::locale const &source,size_t refs=0);
		virtual ~convert();

		///
		/// Convert string in current locale representation (utf8 or other encoding)
		/// to upper case.
		///
		
		std::string to_upper(std::string const &str) const;
		///
		/// Convert string in current locale representation (utf8 or other encoding)
		/// to lower case.
		///
		
		std::string to_lower(std::string const &str) const;

		///
		/// Convert string in current locale representation (utf8 or other encoding)
		/// to title case.
		///
		std::string to_title(std::string const &str) const;

		///
		/// Perform Unicode normalization of the string in current locale representation
		/// (utf8 or other encoding). Such conversion may be meaningless for non-Unicode locale.
		///
		/// Note: if CppCMS is compiled without support of ICU this function does nothing.
		///

		std::string to_normal(std::string const &str,norm_type how = norm_default) const;

	private:
		
		util::hold_ptr<convert_impl> impl_;
			
	};


} // locale
} // cppcms


#endif
