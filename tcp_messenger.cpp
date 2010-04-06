#define CPPCMS_SOURCE
#include "tcp_messenger.h"
#include "cppcms_error.h"

namespace cppcms {
namespace impl {

void messenger::connect(std::string ip,int port) 
{
	ip_=ip;
	port_=port;
	boost::system::error_code e;
	socket_.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip),port),e);
	if(e) throw cppcms_error("connect:"+e.message());
	boost::asio::ip::tcp::no_delay nd(true);
	socket_.set_option(nd);
}

messenger::messenger(std::string ip,int port) :
		socket_(srv_)
{
	connect(ip,port);
}
messenger::messenger() :
	socket_(srv_)
{
}

void messenger::transmit(tcp_operation_header &h,std::string &data)
{
	bool done=false;
	int times=0;
	do {
		try {
			// FIXME use buffers
			boost::asio::write(socket_,boost::asio::buffer(&h,sizeof(h)));
			if(h.size>0) {
				boost::asio::write(socket_,boost::asio::buffer(data,h.size));
			}
			boost::asio::read(socket_,boost::asio::buffer(&h,sizeof(h)));
			if(h.size>0) {
				std::vector<char> d(h.size);
				boost::asio::read(socket_,boost::asio::buffer(d,h.size));
				data.assign(d.begin(),d.begin()+h.size);
			}
			done=true;
		}
		catch(boost::system::system_error const &e) {
			if(times) {
				throw cppcms_error(std::string("tcp_cache:")+e.what());
			}
			socket_.close();
			boost::system::error_code er;
			socket_.connect(boost::asio::ip::tcp::endpoint(
						boost::asio::ip::address::from_string(ip_),port_),er);
			if(er) throw cppcms_error("reconnect:"+er.message());
			times++;
		}
	}while(!done);
} 

} // impl
	
} // namespace cppcms

