#ifndef CPPCMS_VIEWS_POOL_H
#define CPPCMS_VIEWS_POOL_H

#include "defs.h"
#include "noncopyable.h"
#include "base_view.h"

#include <memory>
#include <map>
#include <ostream>

namespace cppcms {

	class CPPCMS_API views_pool : public util::noncopyable {
	public:
		typedef std::auto_ptr<base_view> (*view_factory_type)(std::ostream &,base_content *c);
		typedef std::map<std::string,view_factory_type> mapping_type;

		template<typename View,typename Content>
		std::auto_ptr<base_view> view_builder(std::ostream &stream,base_content *c) 
		{
			std::auto_ptr<base_view> p(new T(stream,dynamic_cast<Content &>(*c)));
			return p;
		};

		views_pool();
		views_pool(json::value const &settings);
		~views_pool();
	
		void render(std::string template_name,std::ostream &out,base_content &content);
		void render(std::string skin,std::string template_name,std::ostream &out,base_content &content);
		
		void add_view(std::string skin,mapping_type const &mapping);

		static views_pool &static_instance();
	private:

		struct data;
		struct skin;
		util::hold_ptr<data> d;
	};

}


#endif
