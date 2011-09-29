#include <booster/aio/io_service.h>
#include <booster/aio/stream_socket.h>
#include <booster/aio/buffer.h>
#include <booster/posix_time.h>
#include <booster/shared_ptr.h>
#include <iostream>
#include <string>

class client;

struct bind_io {
public:
	typedef void (client::*member_function)(booster::system::error_code const &e,size_t n);
	bind_io(member_function f,client *c) : c_(c) , m_(f)
	{
	}
	void operator()(booster::system::error_code const &e,size_t n) const;
private:
	client *c_;
	member_function m_;
};

struct bind_handler {
public:
	typedef void (client::*member_function)(booster::system::error_code const &e);
	bind_handler(member_function f,client *c) : c_(c) , m_(f)
	{
	}
	void operator()(booster::system::error_code const &e) const;
private:
	client *c_;
	member_function m_;
};

int total,max_total;
int stopped,max_stopped;
int total_read;

std::string first_request;

class client {
public:
	client(booster::aio::io_service &srv,std::string ip,int port,std::string url) :
		socket_(srv)
	{
		request_ =	"GET " + url + " HTTP/1.0\r\n"
					"Host:" + ip + "\r\n"
					"Connection:close\r\n"
					"\r\n";
		ep_ = booster::aio::endpoint(ip,port);
	}
	void run()
	{
		if(total>= max_total) {
			stopped++;
			if(stopped == max_stopped)
				socket_.get_io_service().stop();
			return;
		}
		is_first_ = total++ == 0;
		if(total % 100 == 0) {
			std::cout << total << std::endl;
		}
		socket_.open(booster::aio::pf_inet);
		socket_.set_option(booster::aio::basic_socket::tcp_no_delay,1);
		socket_.async_connect(ep_,bind_handler(&client::on_connected,this));
	}
	void on_connected(booster::system::error_code const &e)
	{
		handle_error(e);
		socket_.async_write(booster::aio::buffer(request_),bind_io(&client::on_written,this));
	}
	void on_written(booster::system::error_code const &e,size_t)
	{
		handle_error(e);
		socket_.async_read_some(booster::aio::buffer(chunk,sizeof(chunk)),bind_io(&client::on_read,this));
	}
	void on_read(booster::system::error_code const &e,size_t n)
	{
		if(is_first_) {
			first_request.append(chunk,n);
		}
		total_read+=n;
		if(e==booster::system::error_code(booster::aio::aio_error::eof,booster::aio::aio_error_cat)) {
			booster::system::error_code err;
			socket_.shutdown(booster::aio::stream_socket::shut_rdwr,err);
			socket_.close(err);
			run();
		}
		else {
			on_written(e,0);
		}
	}
	void handle_error(booster::system::error_code const &e)
	{
		if(e) {
			throw booster::system::system_error(e);
		}
	}
private:
	char chunk[65536];
	booster::aio::stream_socket socket_;
	booster::aio::endpoint ep_;
	std::string request_;
	bool is_first_;
};

void bind_io::operator()(booster::system::error_code const &e,size_t n) const
{
	(c_->*m_)(e,n);
}

void bind_handler::operator()(booster::system::error_code const &e) const
{
	(c_->*m_)(e);
}

int main()
{
	std::string ip="127.0.0.1";
	int port = 8080;
	std::string url="/hello";
	max_total = 10000;
	max_stopped = 10;
	
	booster::ptime start,end;
	
	try {
		std::vector<booster::shared_ptr<client> > clients(max_stopped);
		booster::aio::io_service srv;
		for(size_t i=0;i<clients.size();i++) {
			clients[i].reset(new client(srv,ip,port,url));
		}
		start = booster::ptime::now();
		for(size_t i=0;i<clients.size();i++)
			clients[i]->run();
		srv.run();
		end = booster::ptime::now();
	}
	catch(std::exception const &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	std::cout << first_request << std::endl;
	std::cout << "Req/s:" << total / booster::ptime::to_number(end-start) << std::endl;
	std::cout << "Size: " << total_read / total << std::endl;
}