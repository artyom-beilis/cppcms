#ifndef CPPCMS_CACHE_INTERFACE_H
#define CPPCMS_CACHE_INTERFACE_H

#include <string>
#include <set>

#include "defs.h"
#include "noncopyable.h"
#include "intrusive_ptr.h"

namespace cppcms {

	class base_cache;

	class CPPCMS_API cache_interface : public util::noncopyable {
	public:
		cache_interface(http::context &context);
		~cache_interface();

		
		void rise(std::string const &trigger);
		void add_trigger(std::string const &trigger);
		void clear();
		bool stats(unsigned &keys,unsigned &triggers);
		bool has_cache();


		bool fetch_page(std::string const &key);
		void store_page(std::string const &key,int timeout=-1);


		bool fetch_frame(std::string const &key,std::string &result,bool notriggers=false);
		void store_frame(std::string const &key,
				 std::string const &frame,
				 std::set<std::string> const &triggers=std::set<std::string>(),
				 int timeout=-1,
				 bool notriggers=false);

		void store_frame(std::string const &key,
				 string const &frame,
				 int timeout,
				 bool notriggers=false);

		template<typename Serializable>
		bool fetch_data(std::string const &key,Serializable &data,bool notriggers=false)
		{
			std::string buffer;
			if(!fetch(key,buffer,notriggers))
				return false;
			serialization_traits<Serializable>::load(buffer,data);
			return true;
		}

		template<typename Serializable>
		void store_data(std::string const &key,Serializable const &data,
				std::set<std::string> const &triggers=std::set<std::string>(),
				int timeout=-1,bool notriggers=false)
		{
			std::string buffer;
			serialization_traits<Serializable>::save(data,buffer);
			store(key,data,triggers,timeout,notriggers);
		}

		template<typename Serializable>
		void store_data(std::string const &key,Serializable const &data,int timeout,bool notriggers=false)
		{
			store_data<Serializable>(key,data,set<std::string>(),timeout,notriggers);
		}

	private:

		void store(	std::string const &key,
				char const *buffer,
				size_t size,
				std::set<std::string> const &triggers,
				int timeout,
				bool notriggers);

		bool fetch(	std::string const &key,
				std::string &buffer,
				bool notriggers)

		struct data;
		util::hold_ptr<data> d;
		std::set<std::string> triggers_;
		intrusive_ptr<base_cache> cache_module_;
	};


}

#endif
