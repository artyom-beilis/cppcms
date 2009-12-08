#include "asio_config.h"
// MUST BE FIRST TO COMPILE CORRECTLY UNDER CYGWIN

#include "tcp_cache_protocol.h"
#include "session_storage.h"
#include "archive.h"
#include "thread_cache.h"
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#   include <boost/shared_ptr.hpp>
#   include <boost/enable_shared_from_this.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
#   include <cppcms_boost/shared_ptr.hpp>
#   include <cppcms_boost/enable_shared_from_this.hpp>
    namespace boost = cppcms_boost;
#endif
#include <ctime>
#include <cstdlib>
#include <pthread.h>
#include <signal.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/date_time/posix_time/posix_time.hpp>
#else // Internal Boost
#   include <cppcms_boost/date_time/posix_time/posix_time.hpp>
#endif

#include "session_file_storage.h"
#ifdef EN_SQLITE_SESSIONS
#include "session_sqlite_storage.h"
#endif

using namespace std;
using namespace cppcms;
using boost::shared_ptr;



class session : public boost::enable_shared_from_this<session> {
	vector<char> data_in;
	string data_out;
	cppcms::tcp_operation_header hout;
	cppcms::tcp_operation_header hin;

public:
	tcp::socket socket_;
	base_cache &cache;
	session_server_storage &sessions;
	session(aio::io_service &srv,base_cache &c,session_server_storage &s) : 
		socket_(srv), cache(c),sessions(s)
	{
	}
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
		case opcodes::session_save:	save(); break;
		case opcodes::session_load:	load(); break;
		case opcodes::session_remove:	remove(); break;
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
	session_server_storage &sessions;
	void on_accept(error_code const &e,boost::shared_ptr<session> s)
	{
		if(!e) {
			tcp::no_delay nd(true);
			s->socket_.set_option(nd);
			s->run();
			start_accept();
		}
	}
	void start_accept()
	{
		boost::shared_ptr<session> s(new session(acceptor_.io_service(),cache,sessions));
		acceptor_.async_accept(s->socket_,boost::bind(&tcp_cache_server::on_accept,this,aio::placeholders::error,s));
	}
public:
	tcp_cache_server(	aio::io_service &io,
				string ip,
				int port,
				base_cache &c,
				session_server_storage &s) : 
		acceptor_(io,
			  tcp::endpoint(aio::ip::address::from_string(ip),
			  port)),
		cache(c),
		sessions(s)
	{
		start_accept();
	}
};

struct params {
	bool en_cache;
	enum { none , files , sqlite3 } en_sessions;
	string session_backend;
	string session_file;
	string session_dir;
	int items_limit;
	int gc_frequency;
	int files_no;
	int port;
	string ip;
	int threads;

	void help()
	{
		cerr<<	"Usage cppcms_tcp_scale [parameter]\n"
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
			else if(param=="--gc" && next) {
				gc_frequency=atoi(next);
				argv++;
			}
			else if(param=="--limit" && next) {
				items_limit=atoi(next);
				argv++;
			}
			else if(param=="--session-files") {
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
#endif
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


void *thread_function(void *ptr)
{
	aio::io_service &io=*(aio::io_service *)(ptr);
	bool stop=false;
	try{
		while(!stop) {
			try {
				io.run();
				stop=true;
			}
			catch(cppcms_error const &e) {
				// Not much to do...
				// Object will be destroyed automatically 
				// Because it does not resubmit itself
				fprintf(stderr,"CppCMS Error %s\n",e.what());
			}
		}
	}
	catch(exception const &e)
	{
		fprintf(stderr,"Fatal:%s",e.what());
	}
	catch(...){
		fprintf(stderr,"Unknown exception");
	}
	io.stop();
	return NULL;
}


int main(int argc,char **argv)
{
	try 
	{
		params par(argc,argv);

		aio::io_service io;

		auto_ptr<base_cache> cache;
		auto_ptr<session_server_storage> storage;
		auto_ptr <garbage_collector> gc;

		if(par.en_cache)
			cache.reset(new thread_cache(par.items_limit));
		else
			cache.reset(new base_cache());

		if(par.en_sessions==params::files) {
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
		

		tcp_cache_server srv_cache(io,par.ip,par.port,*cache,*storage);

		sigset_t new_mask;
		sigfillset(&new_mask);
		sigset_t old_mask;
		pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

		vector<pthread_t> threads;
		threads.resize(par.threads);

		int i;
		for(i=0;i<par.threads;i++){
			if(pthread_create(&threads[i],NULL,thread_function,&io)!=0) {
				perror("pthread_create failed:");
				io.stop();
				for(i=i-1;i>=0;i--) {
					pthread_join(threads[i],NULL);
				}
			}
		}

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
		
		cout<<"Catched signal:"<<sig<<" exiting..."<<endl;

		io.stop();

		for(i=0;i<par.threads;i++) {
			pthread_join(threads[i],NULL);
		}
	}
	catch(std::exception const &e) {
		cerr<<"Error:"<<e.what()<<endl;
		return 1;
	}
	cout<<"Done"<<endl;
	return 0;
}

