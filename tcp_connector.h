#ifndef CPPCMS_TCP_CONNECTOR_H
#define CPPCMS_TCP_CONNECTOR_H

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>

namespace cppcms {

class messenger;
struct tcp_operation_header;

class tcp_connector : private boost::noncopyable
{
protected:
	messenger *tcp;
	int conns;
	virtual unsigned hash(std::string const &key);
	messenger &get(std::string const &key);
	void broadcast(tcp_operation_header &h,std::string &data);
public:
	tcp_connector(std::vector<std::string> const &ip_list,std::vector<int> const &port_list);

	virtual ~tcp_connector();
};


}


#endif
