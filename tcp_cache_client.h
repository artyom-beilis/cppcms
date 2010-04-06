#ifndef TCP_CHACHE_H
#define TCP_CHACHE_H
#include "base_cache.h"
#include "cache_interface.h"
#include "session_storage.h"
#include "tcp_connector.h"
#include <string>

namespace cppcms {
namespace impl {
	class messenger;
	struct tcp_operation_header;


	class tcp_cache : public tcp_connector {
	public:

		tcp_cache(	std::vector<std::string> const &ip_list,
				std::vector<int> const &port_list) 
		:
			tcp_connector(ip_list,port_list)
		{
		}

		static const int up_to_date = -1;
		static const int not_found = 0;
		static const int found = 1;

		int fetch(	std::string const &key,
				std::string &data,
				std::set<std::string> *triggers,
				time_t &timeout,
				uint64_t &generation,
				bool transfer_if_not_updated=false);
		void rise(std::string const &trigger);
		void clear();
		void stats(unsigned &keys,unsigned &triggers);
		void store(	std::string const &key,
				std::string const &data,
				std::set<std::string> const &triggers,
				time_t timeout);
		~tcp_cache();
	};

} // impl
} // cppcms

#endif
