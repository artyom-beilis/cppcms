#ifndef CPPCMS_LOCALE_NUMERIC_H
#define CPPCMS_LOCALE_NUMERIC_H
#include <locale>

#include "defs.h"
#include "hold_ptr.hpp"

namespace cppcms {
	namespace locale {
		class num_put_impl;
		class CPPCMS_API num_put : public std::num_put<char> {
		public:
			static num_put *create(std::locale const &l);
			typedef enum {
				format_default,
				format_currency,
				format_percent,
				format_scientific,
				format_spellout,
				format_ordinal,
				format_numbering
			} format_type;

			iter_type put(format_type format,int width,int presision, char fill, iter_type out,int64_t value);
			iter_type put(format_type format,int width,int presision, char fill, iter_type out,double value);

		protected:
			num_put(num_put_impl *,size_t refs=0);
			virtual ~num_put();

			virtual iter_type do_put (iter_type out, std::ios_base& str, char_type fill, bool val) const;
			virtual iter_type do_put (iter_type out, std::ios_base& str, char_type fill, long val) const;
			virtual iter_type do_put (iter_type out, std::ios_base& str, char_type fill, unsigned long val) const;
			virtual iter_type do_put (iter_type out, std::ios_base& str, char_type fill, long long val) const;
			virtual iter_type do_put (iter_type out, std::ios_base& str, char_type fill, unsigned long long val) const;
			virtual iter_type do_put (iter_type out, std::ios_base& str, char_type fill, double val) const;
			virtual iter_type do_put (iter_type out, std::ios_base& str, char_type fill, long double val) const;
			virtual iter_type do_put (iter_type out, std::ios_base& str, char_type fill, const void* val) const;
		private:
			hold_ptr<num_put_impl> impl_;
			std::num_put<char> const &std() const;
			bool use_std(std::ios_base &str) const;
			format_type get_type(std::ios_base &str,bool is_double=false) const;
		};
	}
}

#endif
