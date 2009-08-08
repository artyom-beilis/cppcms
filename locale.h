#ifndef CPPCMS_LOCALE_H
#define CPPCMS_LOCALE_H

#include "defs.h"
#include "hold_ptr.h"
#include "noncopyable.h"
#include "locale_gettext.h"

namespace cppcms {
	class service;
	namespace locale {
		
		class CPPCMS_API l10n : public util::noncopyable {
		public:
			std::locale const &get();
			void set(std::locale const &);
			void gettext_domain(std::string s);
			std::string gettext_domain();
			void locale(std::string);
			std::string locale();			

			char const *gt(char const *s);
			char const *ngt(char const *s,char const *p,int n);


			l10n(cppcms::service &srv);
			~l10n();

		private:
			void setup();

			struct data;
			cppcms::service &service_;
			util::hold_ptr<data> d;
		};
	}// 
} // cppcms


#endif
