#ifndef CPPCMS_APPLICATIONS_POOL_H
#define CPPCMS_APPLICATIONS_POOL_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"

#include <memory>
#include <string>

namespace cppcms {

	class application;
	class service;

	class CPPCMS_API applications_pool {
	public:

		struct factory : public util::noncopyable {
			virtual std::auto_ptr<application> operator()(service &) const = 0;
			virtual ~factory(){}
		};

		void mount(std::string pattern,std::auto_ptr<factory> aps,int select=0);
		std::auto_ptr<application> get(std::string path,std::string &match);
		void put(std::auto_ptr<application> app);

		applications_pool(service &srv,int pool_size_limit);
		~applications_pool();

	private:
		service *srv_;
		struct data;
		util::hold_ptr<data> d;
	};

} // cppcms



#endif
