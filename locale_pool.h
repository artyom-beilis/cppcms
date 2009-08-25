#ifndef CPPCMS_LOCALE_POOL_H
#define CPPCMS_LOCALE_POOL_H


#include "defs.h"
#include "hold_ptr.h"
#include "noncopyable.h"
#include <string>
#include <locale>


namespace cppcms {
namespace json { class value; }

namespace locale {
	class CPPCMS_API pool : util::noncopyable {
	public:
		pool(json::value const &conf);
		~pool();

		std::locale const &get(std::string const &name) const;

	private:
		struct data;
		util::hold_ptr<data> d;
	};

} // locale
} // cppcms 


#endif
