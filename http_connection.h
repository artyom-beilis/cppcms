#ifndef CPPCMS_HTTP_CONNECTION_H
#define CPPCMS_HTTP_CONNECTION_H
#include "base_asio_config.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

namespace cppcms { namespace http {

class connection {
public:
	virtual bool prepare() = 0;
	virtual std::string getenv(std::string const &key) = 0;
	virtual size_t read(void *buffer,size_t s) = 0;
	virtual size_t write(void const *buffer,size_t s) = 0;
	virtual bool keep_alive() = 0;
	virtual ~connection(){}
};

class async_connection : public connection {
public:
	virtual void async_prepare(boost::function<void(bool)>) = 0;
	virtual void async_read(void *buffer,size_t s,boost::function<void(bool)>) = 0;
	virtual size_t async_write(void const *buffer,size_t s,boost::function<void(bool)>) = 0;
	virtual void cancel();
};


} } // cppcms::http

#endif

