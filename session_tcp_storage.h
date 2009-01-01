#ifndef CPPCMS_SESSION_TCP_STORAGE_H
#define CPPCMS_SESSION_TCP_STORAGE_H

#include "session_storage.h"
#include "tcp_connector.h"
#include "tcp_messenger.h"
#include "session_backend_factory.h"

namespace cppcms {

class cppcms_config;

class session_tcp_storage : public session_server_storage , public tcp_connector {
protected:
	virtual unsigned hash(std::string const &key);
public:
	session_tcp_storage(std::vector<std::string> const &ips,std::vector<int> const &ports) :
		tcp_connector(ips,ports)
	{
	}
	static session_backend_factory factory(cppcms_config const &);
	virtual void save(std::string const &sid,time_t timeout,std::string const &in);
	virtual bool load(std::string const &sid,time_t *timeout,std::string &out);
	virtual void remove(std::string const &sid);
};

} // cppcms
#endif
