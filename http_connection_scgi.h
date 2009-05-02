#ifndef CPPCMS_HTP_CONNECTION_SCGI_H
#define CPPCMS_HTP_CONNECTION_SCGI_H
#include "http_connection.h"
#include "asio_config.h"
namespace cppcms { namespace http {

template<typename Socket>
class scgi_connection : public connection {
	aio::streambuf buf_;
	std::vector<char> data_;
	std::map<std::string,std::string> env_;
	Socket socket_;
	void on_done(error_code const &e,boost::fuction<void(bool>) callback)
	{
		callback(bool(e));
	}
	int check_size(aio::buffer &buf)
	{
		std::istream s(&buf);
		size_t n;
		s>>n;
		if(s.failbit())
			return -1;
		return n;
	}
	bool parse_env(std::vector<char> const &s)
	{
		if(s.back()!=',')
			return false;
		std::vector<char>::const_iterator b,e,p;
		b=s.begin(); e=s.begin()+s.size()-1;
		while(b!=e) {
			p=std::find(b,e,0);
			if(p==e)
				return false;
			std::string key(b,p);
			b=p+1;
			p=std::find(b,e,0);
			if(p==e)
				return false;
			std::string val(b,p);
			b=p+1;
			env_.insert(std::make_pair(key,val));
		}
		if(env_.find("CONTENT_LENGTH")==env_.end())
			return false;
		return true;
	}
public:
	virtual bool keep_alive() { return false; }
	virtual size_t read(void *buffer,size_t s)
	{
		return aio::read(socket_,aio::buffer(buffer,s));
	}
	virtual size_t write(void const *buffer,size_t s)
	{
		return aio::write(socket_,aio::buffer(buffer,s));
	}
	virtual bool prepare()
	{
		try {
			aio::read_until(socket_,buf_,':');
			int n=check_size(buf);
			if(n<0)
				return false;
			data_.resize(n+1,0);
			if(aio::read(socket_,aio::buffer(data))!=n+1)
				return false;
			if(!parse_env(data))
				return false;

		}
		catch(std::exception const &e) {
			return false;
		}
		return true;
	}
	virtual aio::io_service &io_service()
	{
		return socket_.io_service();
	}
	virtual void async_read(void *buffer,size_t s,boost::function<void(bool)> c)
	{
		aio::async_read(socket_,aio::buffer(buffer,s),
			boost::bind(&scgi_connection<Socket>::on_done,shared_from_this(),aio::placeholders::error,c));
	}
	virtual void async_write(void const *buffer,size_t s,boost::function<void(bool)> c)
	{
		aio::async_write(socket_,aio::buffer(buffer,s),
			boost::bind(&scgi_connection<Socket>::on_done,shared_from_this(),aio::placeholders::error,c));
	}
	virtual void async_prepare(boost::function<void(bool)> c)
	{
		aio::async_read_until(socket_,buf_,':',
			boost::bind(&scgi_connection<Socket>::on_net_read,shared_from_this(),aio::placeholders::error,c));
	}
private:
	void on_net_read(error_code const &e,boost::function<void(bool)> c)
	{
		if(e) {
			c(false);
			return;
		}
		int n=check_size(buf);
		if(n<0) {
			c(false) ;
			return;
		}
		data_.resize(n+1,0);
		aio::async_read(socket_,aio::buffer(data_),
			boost::bind(&scgi_connection<Socket>::on_env_read,aio::placeholders::error,c));
	}
	void on_env_read(error_code const &e,boost::function<void(bool)>)
	{
		if(e) {
			c(false);
			return;
		}
		if(!parse_env(data)) {
			c(false);
			return;
		}
		c(true);
	}
public:
	virtual std::string getenv(std::string const &key)
	{
		std::map<std::string,std::string>::iterator p;
		p=env_.find(key);
		if(p==env_.end())
			return std::string();
		return p->second;
	}

}; // connection_scgi

} } 
#endif
