#include "asio_config.h"
// MUST BE FIRST TO COMPILE CORRECTLY UNDER CYGWIN

#include "tcp_cache_protocol.h"
#include "cache_storage.h"
#include "cppcms_error.h"
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/thread.hpp>
#   include <boost/bind.hpp>
#   include <boost/shared_ptr.hpp>
#   include <boost/enable_shared_from_this.hpp>
#   include <boost/date_time/posix_time/posix_time.hpp>
#else // Internal Boost
#   include <cppcms_boost/thread.hpp>
#   include <cppcms_boost/bind.hpp>
#   include <cppcms_boost/shared_ptr.hpp>
#   include <cppcms_boost/enable_shared_from_this.hpp>
#   include <cppcms_boost/date_time/posix_time/posix_time.hpp>
    namespace boost = cppcms_boost;
#endif
#include <time.h>
#include <stdlib.h>
#ifndef CPPCMS_WIN32
#include <signal.h>
#endif

namespace aio = boost::asio;
using namespace cppcms::impl;

class session : public boost::enable_shared_from_this<session> {
	std::vector<char> data_in;
	std::string data_out;
	cppcms::impl::tcp_operation_header hout;
	cppcms::impl::tcp_operation_header hin;

public:
	aio::ip::tcp::socket socket_;
	cppcms::impl::base_cache &cache;
	//cppcms::session_storage &sessions;
	session(aio::io_service &srv,cppcms::impl::base_cache &c): //,session_server_storage &s) : 
		socket_(srv), cache(c) //,sessions(s)
	{
	}
	void run()
	{
		aio::async_read(socket_,aio::buffer(&hin,sizeof(hin)),
				boost::bind(&session::on_header_in,shared_from_this(),
						aio::placeholders::error));
	}
	void on_header_in(boost::system::error_code const &e)
	{
		if(e) return;
		data_in.clear();
		data_in.resize(hin.size);
		aio::async_read(socket_,aio::buffer(data_in,hin.size),
				boost::bind(&session::on_data_in,shared_from_this(),
						aio::placeholders::error));
	}
	
	void fetch()
	{
		std::string a;
		std::set<std::string> tags,*ptags=0;
		std::string key;
		key.assign(data_in.begin(),data_in.end());
		if(hin.operations.fetch.transfer_triggers)
			ptags=&tags;
		uint64_t generation;
		time_t timeout;
		if(!cache.fetch(key,&a,ptags,&timeout,&generation)) {
			hout.opcode=opcodes::no_data;
			return;
		}
		if(hin.operations.fetch.transfer_if_not_uptodate 
			&& generation==hin.operations.fetch.current_gen)
		{
			hout.opcode=opcodes::uptodate;
			return;
		}
		hout.opcode=opcodes::data;
		data_out.swap(a);
		hout.operations.data.data_len=data_out.size();
		if(ptags) {
			for(std::set<std::string>::iterator p=tags.begin(),e=tags.end();p!=e;++p) {
				data_out.append(p->c_str(),p->size()+1);
			}
		}
		hout.operations.data.triggers_len=data_out.size()-hout.operations.data.data_len;
		hout.size=data_out.size();
		
		hout.operations.data.generation=generation;
		time_t now=time(0);
		hout.operations.data.timeout = timeout > now ? timeout - now : 0;
	}

	void rise()
	{
		std::string key;
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
	bool load_triggers(std::set<std::string> &triggers,char const *start,unsigned len)
	{
		int slen=len;
		while(slen>0) {
			unsigned size=strlen(start);
			if(size==0) {
				return false;
			}
			std::string tmp;
			tmp.assign(start,size);
			slen-=size+1;
			start+=size+1;
			triggers.insert(tmp);
		}
		return true;
	}
	void store()
	{
		std::set<std::string> triggers;
		if(	hin.operations.store.key_len
			+hin.operations.store.data_len
			+hin.operations.store.triggers_len != hin.size
			|| hin.operations.store.key_len == 0)
		{
			hout.opcode=opcodes::error;
			return;
		}
		std::string ts;
		std::vector<char>::iterator p=data_in.begin()
			+hin.operations.store.key_len
			+hin.operations.store.data_len;
		ts.assign(p,p + hin.operations.store.triggers_len);
		if(!load_triggers(triggers,ts.c_str(),
					hin.operations.store.triggers_len))
		{
			hout.opcode=opcodes::error;
			return;
		}
		time_t timeout=time(0)+(time_t)hin.operations.store.timeout;
		std::string key;
		key.assign(data_in.begin(),data_in.begin()+hin.operations.store.key_len);
		std::string data;
		data.assign(data_in.begin()+hin.operations.store.key_len,
				data_in.begin() + hin.operations.store.key_len + hin.operations.store.data_len);
		cache.store(key,data,triggers,timeout);
		hout.opcode=opcodes::done;
	}
/*	
	void save()
	{
		if(hin.size <= 32)
		{
			hout.opcode=opcodes::error;
			return;
		}
		time_t timeout=hin.operations.session_save.timeout + time(NULL);
		string sid(data_in.begin(),data_in.begin()+32);
		string value(data_in.begin()+32,data_in.end());
		sessions.save(sid,timeout,value);
		hout.opcode=opcodes::done;
	}
	void load()
	{
		if(hin.size!=32) {
			hout.opcode=opcodes::error;
			return;
		}
		time_t timeout;
		int toffset;
		string sid(data_in.begin(),data_in.end());
		if(!sessions.load(sid,&timeout,data_out) && (toffset=(timeout-time(NULL))) < 0) {
			hout.opcode=opcodes::no_data;
			return;
		}
		hout.opcode=opcodes::session_load_data;
		hout.size=data_out.size();
		hout.operations.session_data.timeout=toffset;
	}
	void remove()
	{
		if(hin.size!=32) {
			hout.opcode=opcodes::error;
			return;
		}
		string sid(data_in.begin(),data_in.end());
		sessions.remove(sid);
	}
	*/
	void on_data_in(boost::system::error_code const &e)
	{
		if(e) return;
		memset(&hout,0,sizeof(hout));
		switch(hin.opcode){
		case opcodes::fetch:		fetch(); break;
		case opcodes::rise:		rise(); break;
		case opcodes::clear:		clear(); break;
		case opcodes::store:		store(); break;
		case opcodes::stats:		stats(); break;
		/*case opcodes::session_save:	save(); break;
		case opcodes::session_load:	load(); break;
		case opcodes::session_remove:	remove(); break;*/
		default:
			hout.opcode=opcodes::error;
		}
		aio::async_write(socket_,aio::buffer(&hout,sizeof(hout)),
			boost::bind(&session::on_header_out,shared_from_this(),
				aio::placeholders::error));
	}
	void on_header_out(boost::system::error_code const &e)
	{
		if(e) return;
		if(hout.size==0) {
			run();
			return ;
		}
		aio::async_write(socket_,aio::buffer(data_out.c_str(),hout.size),
			boost::bind(&session::on_data_out,shared_from_this(),
				aio::placeholders::error));
	}
	void on_data_out(boost::system::error_code const &e)
	{
		if(e) return;
		run();
	}

};

class tcp_cache_server  {
	aio::ip::tcp::acceptor acceptor_;
	cppcms::impl::base_cache &cache;
	//session_server_storage &sessions;
	void on_accept(boost::system::error_code const &e,boost::shared_ptr<session> s)
	{
		if(!e) {
			aio::ip::tcp::no_delay nd(true);
			s->socket_.set_option(nd);
			s->run();
			start_accept();
		}
	}
	void start_accept()
	{
		//boost::shared_ptr<session> s(new session(acceptor_.io_service(),cache,sessions));
		boost::shared_ptr<session> s(new session(acceptor_.io_service(),cache));
		acceptor_.async_accept(s->socket_,boost::bind(&tcp_cache_server::on_accept,this,aio::placeholders::error,s));
	}
public:
	tcp_cache_server(	aio::io_service &io,
				std::string ip,
				int port,
				cppcms::impl::base_cache &c
				//,session_server_storage &s
				) : 
		acceptor_(io,
			  aio::ip::tcp::endpoint(aio::ip::address::from_string(ip),
			  port)),
		cache(c)
	//	,sessions(s)
	{
		start_accept();
	}
};

struct params {
	bool en_cache;
	enum { none , files , sqlite3 } en_sessions;
	std::string session_backend;
	std::string session_file;
	std::string session_dir;
	int items_limit;
	int gc_frequency;
	int files_no;
	int port;
	std::string ip;
	int threads;

	void help()
	{
		std::cerr<<	
			"Usage cppcms_tcp_scale [parameter]\n"
			"    --bind IP          ipv4/ipv6 IPto bind (default 0.0.0.0)\n"
			"    --port N           port to bind -- MANDATORY\n"
			"    --threads N        number of threads, default 1\n"
			"    --cache            Enable cache module\n"
			"    --limit N          maximal Number of items to store\n"
			"                       mandatory if cache enabled\n"
			"    --session-files    Enable files bases session backend\n"
			"    --dir              Directory where files stored\n"
			"                       mandatory if session-files enabled\n"
			"    --gc N             gc frequencty seconds (default 600)\n"
			"                       it is enabled if threads > 1\n"
#ifdef EN_SQLITE_SESSIONS
			"    --session-sqlite3  Enable sqlite session backend\n"
			"    --file             Sqlite3 DB file. Mandatory for sqlite\n"
			"                       session backend\n"
			"    --dbfiles          Number of DB files, default 0\n"
			"                       0->1 file, 1-> 2 files, 2 -> 4 files, etc\n"
#endif
			"\n"
			"    At least one of   --session-files,"
#ifdef EN_SQLITE_SESSIONS
			" --session-sqlite3,"
#endif
			" --cache\n"
			"    should be defined\n"
			"\n";
	}
	params(int argc,char **argv) :
		en_cache(false),
		en_sessions(none),
		items_limit(-1),
		gc_frequency(-1),
		files_no(0),
		port(-1),
		ip("0.0.0.0"),
		threads(1)
	{
		using namespace std;
		argv++;
		while(*argv) {
			string param=*argv;
			char *next= *(argv+1);
			if(param=="--bind" && next) {
				ip=next;
				argv++;
			}
			else if(param=="--port" && next) {
				port=atoi(next);
				argv++;
			}
			else if(param=="--threads" && next) {
				threads=atoi(next);
				argv++;
			}
/*			else if(param=="--gc" && next) {
				gc_frequency=atoi(next);
				argv++;
			}*/
			else if(param=="--limit" && next) {
				items_limit=atoi(next);
				argv++;
			}
/*			else if(param=="--session-files") {
				en_sessions=files;
			}
			else if(param=="--dir" && next) {
				session_dir=next;
				argv++;
			}
#ifdef EN_SQLITE_SESSIONS
			else if(param=="--file" && next) {
				session_file=next;
				argv++;
			}
			else if(param=="--dbfiles" && next) {
				files_no=atoi(next);
				argv++;
			}
			else if(param=="--session-sqlite3") {
				en_sessions=sqlite3;
			}
#endif*/
			else if(param=="--cache") {
				en_cache=true;
			}
			else {
				help();
				throw runtime_error("Incorrect parameter:"+param);
			}
			argv++;
		}
		if(!en_cache && !en_sessions) {
			help();
			throw runtime_error("Neither cache nor sessions mods are defined");
		}
		if(en_sessions == files && session_dir.empty()) {
			help();
			throw runtime_error("parameter --dir undefined");
		}
		if(en_sessions == sqlite3 && session_file.empty()) {
			help();
			throw runtime_error("patameter --file undefined");
		}
		if(files_no == -1) files_no=1;
		if(port==-1) {
			help();
			throw runtime_error("parameter --port undefined");
		}
		if(en_cache && items_limit == -1) {
			help();
			throw runtime_error("parameter --limit undefined");
		}
		if(gc_frequency != -1) {
			if(threads == 1) {
				throw runtime_error("You have to use more then one thread to enable gc");
			}
		}
		if(threads > 1 && gc_frequency==-1) {
			gc_frequency = 600;
		}
	}
};
/*
class garbage_collector
{
	aio::deadline_timer timer;
	boost::shared_ptr<storage::io> io;
	int seconds;
	void submit()
	{
		timer.expires_from_now(boost::posix_time::seconds(seconds));
		timer.async_wait(boost::bind(&garbage_collector::gc,this,_1));
	}
	void gc(error_code const &e)
	{
		session_file_storage::gc(io);
		submit();
	}
public:
	garbage_collector(aio::io_service &srv,int sec,boost::shared_ptr<storage::io> io_) :
		timer(srv),
		seconds(sec),
		io(io_)
	{
		submit();	
	}
};
*/

void thread_function(aio::io_service *io)
{
	bool stop=false;
	try{
		while(!stop) {
			try {
				io->run();
				stop=true;
			}
			catch(cppcms::cppcms_error const &e) {
				// Not much to do...
				// Object will be destroyed automatically 
				// Because it does not resubmit itself
				fprintf(stderr,"CppCMS Error %s\n",e.what());
			}
		}
	}
	catch(std::exception const &e)
	{
		fprintf(stderr,"Fatal:%s",e.what());
	}
	catch(...){
		fprintf(stderr,"Unknown exception");
	}
	io->stop();
}


int main(int argc,char **argv)
{
	try 
	{
		params par(argc,argv);

		aio::io_service io;

		cppcms::intrusive_ptr<cppcms::impl::base_cache> cache;
		//auto_ptr<session_server_storage> storage;
		//auto_ptr <garbage_collector> gc;

		if(par.en_cache)
			cache = cppcms::impl::thread_cache_factory(par.items_limit);

/*		if(par.en_sessions==params::files) {
			boost::shared_ptr<storage::io> storage_io(new storage::thread_io(par.session_dir));
			storage.reset(new session_file_storage(storage_io));
			if(par.threads > 1 && par.gc_frequency > 0) {
				gc.reset(new garbage_collector(io,par.gc_frequency,storage_io));
			}
		}
#ifdef EN_SQLITE_SESSIONS
		else if(par.en_sessions==params::sqlite3) {
			boost::shared_ptr<storage::sqlite_N>
				sql(new storage::sqlite_N(par.session_file,1<<par.files_no,true,1000,5));
			storage.reset(new session_sqlite_storage(sql));
		}
#endif
		else {
			storage.reset(new empty_session_server_storage());
		}
		*/

		//tcp_cache_server srv_cache(io,par.ip,par.port,*cache,*storage);
		tcp_cache_server srv_cache(io,par.ip,par.port,*cache);
		
#ifndef CPPCMS_WIN32
		sigset_t new_mask;
		sigfillset(&new_mask);
		sigset_t old_mask;
		pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
#endif

		std::vector<boost::shared_ptr<boost::thread> > threads;
		
		int i;
		for(i=0;i<par.threads;i++){
			boost::shared_ptr<boost::thread> thread;
			thread.reset(new boost::thread(boost::bind(thread_function,&io)));
			threads.push_back(thread);
		}
#ifndef CPPCMS_WIN32
		// Restore previous mask
		pthread_sigmask(SIG_SETMASK,&old_mask,0);
		// Wait for signlas for exit
		sigset_t wait_mask;
		sigemptyset(&wait_mask);
		sigaddset(&wait_mask, SIGINT);
		sigaddset(&wait_mask, SIGQUIT);
		sigaddset(&wait_mask, SIGTERM);
		pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
		int sig = 0;
		sigwait(&wait_mask, &sig);
#else
		char c;
		std::cin >> c;
#endif
		
		std::cout<<"Catched signal: exiting..."<<std::endl;

		io.stop();


		for(i=0;i<par.threads;i++) {
			threads[i]->join();
		}
	}
	catch(std::exception const &e) {
		std::cerr<<"Error:"<<e.what()<<std::endl;
		return 1;
	}
	std::cout<<"Done"<<std::endl;
	return 0;
}

