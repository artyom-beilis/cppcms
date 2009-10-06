#include <locale>
#include <string>

#include "defs.h"
#include "hold_ptr.h"

namespace cppcms { namespace locale {

	class collate_impl;

	class CPPCMS_API collate : public std::collate<char> {
	public:
		
		typedef enum {
			primary = 0,
			alphabetic_sensitive = primary,
			secondary = 1,
			diacritic_sensitive = secondary,
			tertiary = 2,
			case_sensitive = tertiary,
			quaternary = 3
		} level_type;

		int compare(	level_type level,
				char const *p1_start,char const *p1_end,
				char const *p2_start,char const *p2_end) const;

		int compare(level_type level,std::string const &l,std::string const &r) const;
		long hash(level_type level,std::string const &s) const;		
		long hash(level_type level,char const *b,char const *e) const;
		std::string transform(level_type level,std::string const &s) const;
		std::string transform(level_type level,char const *b,char const *e) const;
		
		static collate *create(std::locale const &l);

	private:

		collate(collate_impl *impl,size_t refs = 0);
		virtual ~collate();
		
		/// Standard functions... override
		virtual int do_compare(	char const *p1_start,char const *p1_end,
					char const *p2_start,char const *p2_end) const
		{
			return compare(primary,p1_start,p1_end,p2_start,p2_end);
		}
		virtual std::string do_transform(char const *b,char const *e) const
		{
			return transform(primary,b,e);
		}

		virtual long do_hash(char const *b,char const *e) const
		{
			return hash(primary,b,e);
		}
		

		util::hold_ptr<collate_impl> impl_;

	};

} } // cppcms::locale
