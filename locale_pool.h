#ifndef CPPCMS_LOCALE_POOL_H
#define CPPCMS_LOCALE_POOL_H


#include "defs.h"
#include "hold_ptr.h"
#include "noncopyable.h"
#include <string>

namespace cppcms {
namespace locale {
	class CPPCMS_API pool : util::noncopyable {
	public:
		void load(std::string name);
		void add_gettext_domain(std::string name);
		std::locale const &get(std::string const &name) const;

		pool(std::string path);
		~pool();
	private:
		struct data;
		util::hold_ptr<data> d;
	};

} // locale
} // cppcms 


#endif
