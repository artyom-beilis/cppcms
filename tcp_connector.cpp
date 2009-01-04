#include "asio_config.h"
// MUST BE FIRST TO COMPILE CORRECTLY UNDER CYGWIN
#include "tcp_messenger.h"
#include "tcp_connector.h"

namespace cppcms {

tcp_connector::tcp_connector(vector<string> const& ip,vector<int> const &port)
{
	if(ip.size()<1 || port.size()!=ip.size()) {
		throw cppcms_error("Incorrect parameters for tcp cache");
	}
	conns=ip.size();
	tcp=new messenger[conns];
	try {
		for(int i=0;i<conns;i++) {
			tcp[i].connect(ip[i],port[i]);
		}
	}
	catch(...) {
		delete [] tcp;
		tcp=NULL;
		throw;
	}
}

tcp_connector::~tcp_connector()
{
	delete [] tcp;
}

void tcp_connector::broadcast(tcp_operation_header &h,string &data)
{
	int i;
	for(i=0;i<conns;i++) {
		tcp_operation_header ht=h;
		string dt=data;
		tcp[i].transmit(ht,data);
	}
}

unsigned tcp_connector::hash(string const &key)
{
	if(conns==1) return 0;
	unsigned val=0,i;
	for(i=0;i<key.size();i++) {
		val+=251*key[i]+103 % 307;
	}
	return val % conns;;
}

messenger &tcp_connector::get(string const &key)
{
	return tcp[hash(key)];
}

} // cppcms

