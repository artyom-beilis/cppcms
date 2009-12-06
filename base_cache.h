#ifndef BASE_CACHE_H
#define BASE_CACHE_H

#include <string>
#include <set>
#include "defs.h"
#include "refcounted.h"

namespace cppcms {

	class base_cache : public refcounted {
	public:
		virtual bool fetch(std::string const &key,std::string &a,std::set<std::string> &tags); 
		virtual void store(std::string const &key,std::set<std::string> const &triggers,time_t timeout,std::string const &value);
		virtual void rise(std::string const &trigger);
		virtual void clear();
		virtual void stats(unsigned &keys,unsigned &triggers);
		virtual ~base_cache();
	};

}

#endif
