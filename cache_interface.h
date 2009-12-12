#ifndef CPPCMS_CACHE_INTERFACE_H
#define CPPCMS_CACHE_INTERFACE_H

#include <string>
#include <set>

#include "defs.h"
#include "noncopyable.h"
#include "intrusive_ptr.h"
#include "hold_ptr.h"

namespace cppcms {

	namespace impl {
		class base_cache;
	}
	namespace http {
		class context;
	};

	template<typename Object>
	struct serialization_traits;

	class CPPCMS_API cache_interface : public util::noncopyable {
	public:
		cache_interface(http::context &context);
		~cache_interface();

		
		void rise(std::string const &trigger);
		void add_trigger(std::string const &trigger);
		void clear();
		void reset();
		bool stats(unsigned &keys,unsigned &triggers);
		
		bool has_cache();
		bool nocache();


		bool fetch_page(std::string const &key);
		void store_page(std::string const &key,int timeout=-1);


		bool fetch_frame(std::string const &key,std::string &result,bool notriggers=false);
		void store_frame(std::string const &key,
				 std::string const &frame,
				 std::set<std::string> const &triggers=std::set<std::string>(),
				 int timeout=-1,
				 bool notriggers=false);

		void store_frame(std::string const &key,
				 std::string const &frame,
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
			store_data<Serializable>(key,data,std::set<std::string>(),timeout,notriggers);
		}

	private:


		void store(	std::string const &key,
				std::string const &data,
				std::set<std::string> const &triggers,
				int timeout,
				bool notriggers);

		bool fetch(	std::string const &key,
				std::string &buffer,
				bool notriggers);

		struct data;
		util::hold_ptr<data> d;
		http::context *context_;
		std::set<std::string> triggers_;
		intrusive_ptr<impl::base_cache> cache_module_;

		uint32_t page_compression_used_ : 1;
		uint32_t reserved : 31;
	};


}

#endif
