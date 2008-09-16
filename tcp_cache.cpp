#include "config.h"
#include "tcp_cache.h"

#ifdef USE_BOOST_ASIO
#include <boost/asio.hpp>
namespace aio = boost::asio;
using boost::system::error_code;
using boost::system::system_error;
#else
#include <asio.hpp>
namespace aio = asio;
using asio::error_code;
using asio::system_error;
#endif
#include "tcp_cache_protocol.h"
#include "archive.h"

using aio::ip::tcp;

namespace cppcms {

class messenger {
	aio::io_service srv_;
	tcp::socket socket_;
	string ip_;
	int port_;
public:
	messenger(string ip,int port) :
		socket_(srv_)
	{
		ip_=ip;
		port_=port;
		error_code e;
		socket_.connect(tcp::endpoint(aio::ip::address::from_string(ip),port),e);
		if(e) throw cppcms_error("connect:"+e.message());
		tcp::no_delay nd(true);
		socket_.set_option(nd);
	}
	void transmit(tcp_operation_header &h,string &data)
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
	
};

tcp_cache::tcp_cache(string ip,int port)
{
	tcp=new messenger(ip,port);
}

tcp_cache::~tcp_cache()
{
	delete tcp;
}

void tcp_cache::rise(string const &trigger)
{
	tcp_operation_header h={0};
	h.opcode=opcodes::rise;
	h.size=trigger.size();
	string data=trigger;
	h.operations.rise.trigger_len=trigger.size();
	tcp->transmit(h,data);
}

void tcp_cache::clear()
{
	tcp_operation_header h={0};
	h.opcode=opcodes::clear;
	h.size=0;
	string empty;
	tcp->transmit(h,empty);
}

bool tcp_cache::fetch_page(string const  &key,string &output,bool gzip)
{
	string data=key;
	tcp_operation_header h={0};
	h.opcode=opcodes::fetch_page;
	h.size=data.size();
	h.operations.fetch_page.gzip=gzip;
	h.operations.fetch_page.strlen=data.size();
	tcp->transmit(h,data);
	if(h.opcode==opcodes::page_data) {
		output=data;
		return true;
	}
	return false;
}

bool tcp_cache::fetch(string const &key,archive &a,set<string> &tags)
{
	string data=key;
	tcp_operation_header h={0};
	h.opcode=opcodes::fetch;
	h.size=data.size();
	h.operations.fetch.key_len=data.size();
	tcp->transmit(h,data);
	if(h.opcode!=opcodes::data)
		return false;
	char const *ptr=data.c_str();
	a.set(ptr,h.operations.data.data_len);
	ptr+=h.operations.data.data_len;
	int len=h.operations.data.triggers_len;
	while(len>0) {
		string tag;
		unsigned tmp_len=strlen(ptr);
		tag.assign(ptr,tmp_len);
		ptr+=tmp_len+1;
		len-=tmp_len+1;
		tags.insert(tag);
	}
	return true;
}

void tcp_cache::stats(unsigned &keys,unsigned &triggers)
{
	tcp_operation_header h={0};
	string data;
	h.opcode=opcodes::stats;
	tcp->transmit(h,data);
	if(h.opcode==opcodes::out_stats) {
		keys=h.operations.out_stats.keys;
		triggers=h.operations.out_stats.triggers;
	}
}

void tcp_cache::store(string const &key,set<string> const &triggers,time_t timeout,archive const &a)
{
	tcp_operation_header h={0};
	string data;
	h.opcode=opcodes::store;
	data.append(key);
	h.operations.store.key_len=key.size();
	data.append(a.get());
	h.operations.store.data_len=a.get().size();
	time_t now;
	time(&now);
	h.operations.store.timeout=timeout-now > 0 ? timeout-now : 0;
	unsigned tlen=0;
	for(set<string>::const_iterator p=triggers.begin(),e=triggers.end();p!=e;++p) {
		tlen+=p->size()+1;
		data.append(p->c_str(),p->size()+1);
	}
	h.operations.store.triggers_len=tlen;
	h.size=data.size();
	tcp->transmit(h,data);
}

}


#ifdef TCP_CACHE_UNIT_TEST

#include <assert.h>
#include <iostream>
#include <cstdlib>
int main(int argc,char **argv)
{
	using namespace cppcms;
	using namespace std;
	if(argc!=3) {
		cerr<<"Usage IP port"<<endl;
		return 1;
	}
	try {
		archive a;
		set<string> s;
		tcp_cache tcp(argv[1],atoi(argv[2]));
		assert(tcp.fetch("something",a,s)==false);
		time_t t;
		time(&t);
		t+=2;
		a.set("data",4);
		tcp.store("key",s,t,a);
		unsigned keys,triggers;
		tcp.stats(keys,triggers);
		assert(keys==1);
		assert(triggers==1);
		s.clear();
		a.set("");
		assert(tcp.fetch("key",a,s)==true);
		assert(s.size()==1);
		assert(*(s.begin())=="key");
		assert(a.get()=="data");
		sleep(3);
		assert(tcp.fetch("key",a,s)==false);
		a.set("");
		a<<string("msg1");
		a<<string("msg2");
		time(&t);
		t+=50;
		s.clear();
		s.insert("a");
		s.insert("b");
		tcp.store("k",s,t,a);
		string x;
		assert(tcp.fetch_page("k",x,true)==true);
		assert(x=="msg2");
		assert(tcp.fetch_page("k",x,false)==true);
		assert(x=="msg1");
		a.set("");
		s.clear();
		assert(tcp.fetch("k",a,s)==true);
		assert(s.size()==3);
		set<string>::iterator ptr=s.begin();
		assert(*ptr++=="a");
		assert(*ptr++=="b");
		assert(*ptr++=="k");
		tcp.rise("a");
		assert(tcp.fetch("k",a,s)==false);
		a.set("Something");
		s.clear();
		tcp.store("bb",s,t,a);
		assert(tcp.fetch("xx",a,s)==false);
		assert(tcp.fetch("bb",a,s)==true);
		tcp.clear();
		assert(tcp.fetch("bb",a,s)==false);
		cout<<"Done... OK!\n";
	}
	catch(std::exception const &e) {
		cerr<<e.what()<<endl;
	}
}

#endif
