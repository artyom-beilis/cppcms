#include "worker_thread.h"
#include "global_config.h"
#include "thread_cache.h"
#include "base_view.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "manager.h"

using namespace cgicc;
namespace cppcms {

worker_thread::worker_thread(manager const &s) :
		url(this),
		app(s),
		cache(this),
		cout(&(this->out_buf))
{
	caching_module=app.cache->get();
	static const transtext::trans tr;
	gt=&tr;
} ;

worker_thread::~worker_thread()
{
	app.cache->del(caching_module);
}


void worker_thread::main()
{
	on_start();
	cout<<"<h1>Hello World</h2>\n";
	on_end();
}
void worker_thread::set_header(HTTPHeader *h)
{
	response_header=auto_ptr<HTTPHeader>(h);
};
void worker_thread::add_header(string s) {
	other_headers.push_back(s);
};

void worker_thread::set_cookie(cgicc::HTTPCookie const &c)
{
	response_header->setCookie(c);
}

void worker_thread::set_lang(string const &s)
{
	lang=s;
	gt=&app.gettext->get(s,"");
}

transtext::trans const *worker_thread::domain_gettext(string const &domain)
{
	return &app.gettext->get(lang,domain);
}

HTTPHeader &worker_thread::header()
{
	return *response_header;
}

void worker_thread::run(cgicc_connection &cgi_conn)
{
	cgi=&cgi_conn.cgi();
	env=&(cgi->getEnvironment());
	ostream &cgi_out=cgi_conn.cout();
	other_headers.clear();
	cache.reset();
	set_lang("");
	out_buf.str("");
	this->cgi_conn=&cgi_conn;

	set_header(new HTTPHTMLHeader);

	gzip=gzip_done=false;
	string encoding;

	if((encoding=cgi_conn.env("HTTP_ACCEPT_ENCODING"))!="") {
		if(strstr(encoding.c_str(),"gzip")!=NULL) {
			gzip=app.config.lval("gzip.enable",1);
		}
	}

	try {
		/**********/
		main();
		/**********/
		if(response_header.get() == NULL) {
			throw cppcms_error("Looks like a bug");
		}
	}
	catch(std::exception const &e) {
		string msg=e.what();
		cgi_out<<HTTPStatusHeader(500,msg);
		cgi_out<<"<html><body><p>"+msg+"</p><body></html>";
		gzip=gzip_done=false;
		other_headers.clear();
		out_buf.str("");
		return;
	}

	if(app.config.lval("server.disable_xpowered_by",0)==0) {
		add_header("X-Powered-By: " PACKAGE_NAME "/" PACKAGE_VERSION);
	}

	for(list<string>::iterator h=other_headers.begin();h!=other_headers.end();h++) {
		cgi_out<<*h<<"\n";
	}

	string out=out_buf.str();
	out_buf.str("");

	if(gzip) {
		if(out.size()>0) {
			if(gzip_done){
				cgi_out<<"Content-Length: "<<out.size()<<"\n";
			}
			cgi_out<<"Content-Encoding: gzip\n";
			cgi_out<<*response_header;
			if(gzip_done) {
				cgi_out<<out;
			}
			else{
				int level=app.config.ival("gzip.level",-1);
				int length=app.config.ival("gzip.buffer",-1);
				deflate(out,cgi_out,level,length);
			}
		}
		else {
			cgi_out<<*response_header;
		}
	}
	else {
		cgi_out<<"Content-Length: "<<out.size()<<"\n";
		cgi_out<<*response_header;
		cgi_out<<out;
	}
}

void worker_thread::render(string tmpl,string name,base_content &content,ostream &out )
{
	using cppcms::details::views_storage;
	base_view::settings s(this,&out);
	auto_ptr<base_view> p(views_storage::instance().fetch_view(tmpl,name,s,&content));
	if(!p.get()) throw cppcms_error("Template `"+name+"' not found in template set `" + tmpl +"'");
	p->render();	
};

void worker_thread::render(string tmpl,string name,base_content &content)
{
	render(tmpl,name,content,cout);
};

void worker_thread::render(string name,base_content &content,ostream &o)
{
	render(current_template,name,content,o);
};

void worker_thread::render(string name,base_content &content)
{
	render(current_template,name,content,cout);
};

}



