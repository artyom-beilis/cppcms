#ifdef USE_BOOST_ASIO
#include <boost/asio.hpp>
namespace aio = boost::asio;
using boost::system::error_code;
#else
#include <asio.hpp>
namespace aio = asio;
using asio::error_code;
#endif
#include "tcp_cache_protocol.h"
#include "archive.h"
#include "thread_cache.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <ctime>
#include <cstdlib>

using namespace std;
using namespace cppcms;
using aio::ip::tcp;
using boost::shared_ptr;



class session : public boost::enable_shared_from_this<session> {
	vector<char> data_in;
	string data_out;
	cppcms::tcp_operation_header hout;
	cppcms::tcp_operation_header hin;

public:
	tcp::socket socket_;
	base_cache &cache;
	session(aio::io_service &srv,base_cache &c) : socket_(srv), cache(c) {}
	void run()
	{
		aio::async_read(socket_,aio::buffer(&hin,sizeof(hin)),
				boost::bind(&session::on_header_in,shared_from_this(),
						aio::placeholders::error));
	}
	void on_header_in(error_code const &e)
	{
		data_in.clear();
		data_in.resize(hin.size);
		aio::async_read(socket_,aio::buffer(data_in,hin.size),
				boost::bind(&session::on_data_in,shared_from_this(),
						aio::placeholders::error));
	}
	void fetch_page()
	{
		string key;
		key.assign(data_in.begin(),data_in.end());
		if(cache.fetch_page(key,data_out,hin.operations.fetch_page.gzip)) {
			hout.opcode=opcodes::page_data;
			hout.size=data_out.size();
			hout.operations.page_data.strlen=data_out.size();
		}
		else {
			hout.opcode=opcodes::no_data;
		}
	}
	void fetch()
	{
		archive a;
		set<string> tags;
		string key;
		key.assign(data_in.begin(),data_in.end());
		if(!cache.fetch(key,a,tags)) {
			hout.opcode=opcodes::no_data;
		}
		else {
			hout.opcode=opcodes::data;
			data_out=a.get();
			hout.operations.data.data_len=data_out.size();
			for(set<string>::iterator p=tags.begin(),e=tags.end();p!=e;++p) {
				data_out.append(p->c_str(),p->size()+1);
			}
			hout.operations.data.triggers_len=data_out.size()-hout.operations.data.data_len;
			hout.size=data_out.size();
		}
	}
	void rise()
	{
		string key;
		key.assign(data_in.begin(),data_in.end());
		cache.rise(key);
		hout.opcode=opcodes::done;
	}
	void clear()
	{
		cache.clear();
		hout.opcode=opcodes::done;
	}
	void stats()
	{
		unsigned k,t;
		cache.stats(k,t);
		hout.opcode=opcodes::out_stats;
		hout.operations.out_stats.keys=k;
		hout.operations.out_stats.triggers=t;
	}
	bool load_triggers(set<string> &triggers,char const *start,unsigned len)
	{
		int slen=len;
		while(slen>0) {
			unsigned size=strlen(start);
			if(size==0) {
				return false;
			}
			string tmp;
			tmp.assign(start,size);
			slen-=size+1;
			start+=size+1;
			triggers.insert(tmp);
		}
		return true;
	}
	void store()
	{
		set<string> triggers;
		if(	hin.operations.store.key_len
			+hin.operations.store.data_len
			+hin.operations.store.triggers_len != hin.size
			|| hin.operations.store.key_len == 0)
		{
			hout.opcode=opcodes::error;
			return;
		}
		string ts;
		vector<char>::iterator p=data_in.begin()
			+hin.operations.store.key_len
			+hin.operations.store.data_len;
		ts.assign(p,p + hin.operations.store.triggers_len);
		if(!load_triggers(triggers,ts.c_str(),
					hin.operations.store.triggers_len))
		{
			hout.opcode=opcodes::error;
			return;
		}
		time_t now;
		std::time(&now);
		time_t timeout=now+(time_t)hin.operations.store.timeout;
		string key;
		key.assign(data_in.begin(),data_in.begin()+hin.operations.store.key_len);
		string data;
		data.assign(data_in.begin()+hin.operations.store.key_len,
				data_in.begin() + hin.operations.store.key_len + hin.operations.store.data_len);
		archive a(data);
		cache.store(key,triggers,timeout,a);
		hout.opcode=opcodes::done;
	}
	void on_data_in(error_code const &e)
	{
		if(e) return;
		memset(&hout,0,sizeof(hout));
		switch(hin.opcode){
		case opcodes::fetch_page:	fetch_page(); break;
		case opcodes::fetch:		fetch(); break;
		case opcodes::rise:		rise(); break;
		case opcodes::clear:		clear(); break;
		case opcodes::store:		store(); break;
		case opcodes::stats:		stats(); break;
		default:
			hout.opcode=opcodes::error;
		}
		async_write(socket_,aio::buffer(&hout,sizeof(hout)),
			boost::bind(&session::on_header_out,shared_from_this(),
				aio::placeholders::error));
	}
	void on_header_out(error_code const &e)
	{
		if(e) return;
		if(hout.size==0) {
			run();
			return ;
		}
		async_write(socket_,aio::buffer(data_out.c_str(),hout.size),
			boost::bind(&session::on_data_out,shared_from_this(),
				aio::placeholders::error));
	}
	void on_data_out(error_code const &e)
	{
		if(e) return;
		run();
	}

};

class tcp_cache_server  {
	tcp::acceptor acceptor_;
	base_cache &cache;
	void on_accept(error_code const &e,shared_ptr<session> s)
	{
		if(!e) {
			s->run();
			start_accept();
		}
	}
	void start_accept()
	{
		shared_ptr<session> s(new session(acceptor_.io_service(),cache));
		acceptor_.async_accept(s->socket_,boost::bind(&tcp_cache_server::on_accept,this,aio::placeholders::error,s));
	}
public:
	tcp_cache_server(	aio::io_service &io,
				string ip,
				int port,
				base_cache &c) : 
		acceptor_(io,
			  tcp::endpoint(aio::ip::address::from_string(ip),
			  port)),
		cache(c)
	{
		start_accept();
	}
};


int main(int argc,char **argv)
{
	if(argc!=4) {
		cerr<<"Usage: tcp_cache_server ip port entries-limit"<<endl;
		return 1;
	}
	try 
	{
		aio::io_service io;
		thread_cache cache(atoi(argv[3]));
		tcp_cache_server srv_cache(io,argv[1],atoi(argv[2]),cache);
		io.run();
	}
	catch(std::exception const &e) {
		cerr<<"Error:"<<e.what()<<endl;
		return 1;
	}
	return 0;
}

