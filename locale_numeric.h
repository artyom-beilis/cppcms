#ifndef CPPCMS_LOCALE_NUMERIC_H
#define CPPCMS_LOCALE_NUMERIC_H
#include <locale>

#include "defs.h"
#include "hold_ptr.h"

namespace cppcms {
	namespace locale {
		
		
		class numeric_impl;
		
		class CPPCMS_API numeric : public std::locale::facet {
		public:

			static std::locale::id id;

			static numeric *create(std::locale const &l);

			typedef enum {
				format_normal = 0,	///< Normal, locale default formatting
				format_scientific,	///< scientific format
				format_percent,		///< percent format
				format_currency,	///< currency format
				format_iso_currency,	///< currency with ISO currency marker -- '$' -> 'USD'
				format_spellout,	///< spellout number if rules availible
				format_ordinal,		///< display ordinal format 1st, 2nd etc.
				format_numbering,	///< display alternative number format Roman, Hebrew

				format_count,
			} format_type;

			std::string format(format_type type,double value) const;
			std::string format(format_type type,double value,int presision) const;

			std::string format(format_type type,long long value) const;
			std::string format(format_type type,unsigned long long value) const;

			std::string format(format_type type,long value) const;
			std::string format(format_type type,unsigned long value) const;

			std::string format(format_type type,int value) const;
			std::string format(format_type type,unsigned value) const;
			
			bool parse(format_type type,std::string const &,double &value) const;

			bool parse(format_type type,std::string const &,int &value) const;			
			bool parse(format_type type,std::string const &,unsigned int &value) const;			

			bool parse(format_type type,std::string const &,long &value) const;			
			bool parse(format_type type,std::string const &,unsigned long &value) const;			

			bool parse(format_type type,std::string const &,long long &value) const;			
			bool parse(format_type type,std::string const &,unsigned long long &value) const;			

		private:
			numeric(numeric_impl *,size_t refs=0);
			virtual ~numeric();

			util::hold_ptr<numeric_impl> impl_;

		};
	}
}

#endif
