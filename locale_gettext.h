#ifndef CPPCMS_LOCALE_SPLITTER_H
#define CPPCMS_LOCALE_SPLITTER_H

#include "defs.h"
#include "hold_ptr.h"
#include <locale>

namespace cppcms {
namespace locale {
	
	class CPPCMS_API gettext : public std::locale::facet {
	public:
		static std::locale::id id;
		

		struct CPPCMS_API tr {
		public:
			virtual char const *gettext(char const *m) const;
			virtual char const *ngettext(char const *s,char const *p,int n) const;
			virtual ~tr();
		};

		gettext(std::size_t refs=0);
		~gettext();


		tr const &dictionary(char const *domain) const
		{
			return do_dictionary(domain);
		}
		
		bool load(std::string locale_name,std::string path,std::string domain);

	protected:

		virtual tr const &do_dictionary(char const *domain) const;

	private:


		struct data;
		util::hold_ptr<data> d;
	};

} // locale
} // cppcms



#endif
