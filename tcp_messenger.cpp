#include "tcp_messenger.h"

namespace cppcms {

void messenger::connect(string ip,int port) 
{
	ip_=ip;
	port_=port;
	error_code e;
	socket_.connect(tcp::endpoint(aio::ip::address::from_string(ip),port),e);
	if(e) throw cppcms_error("connect:"+e.message());
	tcp::no_delay nd(true);
	socket_.set_option(nd);
}

messenger::messenger(string ip,int port) :
		socket_(srv_)
{
	connect(ip,port);
}
messenger::messenger() :
	socket_(srv_)
{
}

void messenger::transmit(tcp_operation_header &h,string &data)
{
	bool done=false;
	int times=0;
	do {
		try {
			aio::write(socket_,aio::buffer(&h,sizeof(h)));
			if(h.size>0) {
				aio::write(socket_,aio::buffer(data,h.size));
			}
			aio::read(socket_,aio::buffer(&h,sizeof(h)));
			if(h.size>0) {
				vector<char> d(h.size);
				aio::read(socket_,aio::buffer(d,h.size));
				data.assign(d.begin(),d.begin()+h.size);
			}
			done=true;
		}
		catch(system_error const &e) {
			if(times) {
				throw cppcms_error(string("tcp_cache:")+e.what());
			}
			socket_.close();
			error_code er;
			socket_.connect(
					tcp::endpoint(
					aio::ip::address::from_string(ip_),port_),er);
			if(er) throw cppcms_error("reconnect:"+er.message());
			times++;
		}
	}while(!done);
}
	
} // namespace cppcms

