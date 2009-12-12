#ifndef BASE_CACHE_H
#define BASE_CACHE_H

#include <string>
#include <set>
#include "defs.h"
#include "refcounted.h"

namespace cppcms {
namespace impl {
	class base_cache : public refcounted {
	public:
		virtual bool fetch(std::string const &key,std::string &a,std::set<std::string> *tags) = 0; 
		virtual void store(std::string const &key,std::string const &b,std::set<std::string> const &triggers,time_t timeout) = 0;
		virtual void rise(std::string const &trigger) = 0;
		virtual void clear() = 0;
		virtual void stats(unsigned &keys,unsigned &triggers) = 0;
		virtual ~base_cache()
		{
		}
	};

} // impl
} // cppcms
#endif
