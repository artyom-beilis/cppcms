#ifndef SESSION_STORAGE_H
#define SESSION_STORAGE_H

#include <boost/noncopyable.hpp>
#include <string>

namespace cppcms {

class session_server_storage : private boost::noncopyable {
public:
	virtual void save(std::string const &sid,time_t timeout,std::string const &in) = 0;
	virtual bool load(std::string const &sid,time_t *timeout,std::string &out) = 0;
	virtual void remove(std::string const &sid) = 0;
	virtual ~session_server_storage(){};
};

} // cppcms


#endif
