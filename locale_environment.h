#ifndef CPPCMS_LOCALE_ENVIRONMENT_H
#define CPPCMS_LOCALE_ENVIRONMENT_H

#include "defs.h"
#include "hold_ptr.h"
#include "noncopyable.h"
#include "locale_gettext.h"

namespace cppcms {
	class service;
	namespace locale {
		
		class CPPCMS_API environment : public util::noncopyable {
		public:
			std::locale const &get();
			void set(std::locale const &);
			void gettext_domain(std::string s);
			std::string gettext_domain();
			void locale(std::string);
			std::string locale();			

			char const *gt(char const *s);
			char const *ngt(char const *s,char const *p,int n);


			environment(cppcms::service &srv);
			~environment();

		private:
			void setup();

			struct data;
			cppcms::service &service_;
			util::hold_ptr<data> d;
		};
	}// 
} // cppcms


#endif
