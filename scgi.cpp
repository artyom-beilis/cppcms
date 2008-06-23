#include "scgi.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <boost/scoped_array.hpp>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

namespace cppcms {
using namespace std;
namespace scgi {

static int safe_write(int fd,char const *s,size_t n)
{
	if(n==0) return 0;
	int wr,written=0;
	while(n) {
		wr=::write(fd,s,n);
		if(wr<0) {
			if(errno==EINTR)
				continue;
			return -1;
		}
		if(wr==0)
			break;
		n-=wr;
		written+=wr;
		s+=wr;
	}
	return written;
}
	
int scgi_outbuffer::overflow(int c)
{
	int len=pptr()-pbase();
	if(len) {
		int n=safe_write(fd,pbase(),len);
		pbump(-n);
	}
	if(c!=EOF) {
		char b=c;
		if(safe_write(fd,&b,1)<1)
			return EOF;
	}
	return 0;
}

streamsize scgi_outbuffer::xsputn(char const *s,streamsize n)
{
	return safe_write(fd,s,n);
}

scgi_outbuffer::~ scgi_outbuffer()
{
}


}; // namespace scgi

bool scgi_session::prepare()
{
	char tmpbuf[16];
	if(read(tmpbuf,15)!=15) {
		return false;
	}
	tmpbuf[15]=0;
	char *ptr;
	if((ptr=strstr(tmpbuf,":"))==NULL) {
		return false;
	}
	*ptr=0;
	unsigned len;
	if(sscanf(tmpbuf,"%d",&len)!=1)
		return false;

	char *data_buffer=new char[len+2];
	boost::scoped_array<char> dealloc(data_buffer);
	unsigned delta=15-(ptr-tmpbuf+1);
	memset(data_buffer,0,len+1);
	memcpy(data_buffer,ptr+1,delta);
	unsigned read_size=len+1-delta;
	if(read_size==0){
		return false;
	}
	if(read(data_buffer+delta,read_size)!=read_size || data_buffer[len]!=',')
		return false;
	data_buffer[len]=0;
	unsigned n=0;
	while(n<len) {
		char *p1=data_buffer+n;
		char *p2=p1+strlen(p1)+1;
		if(p2-data_buffer>(int)len) {
			return false;
		}
		envmap[p1]=p2;
		p2=p2+strlen(p2)+1;
		n=p2-data_buffer;
	}

	cgi_ptr=new cgicc::Cgicc(this);
	return true;
}

size_t scgi_session::read(char *s, size_t n)
{
	int v;
	int size=n;
	n=0;
	while(size>0 && (v=::read(socket,s,size))!=size){
		if(v<0) {
			if(errno==EINTR)
				continue;
			return EOF;
		}
		else {
			n+=v;
			s+=v;
			size-=v;
		}
	}
	return n+size;
}

scgi_session::~scgi_session()
{
	delete cgi_ptr;
	shutdown(socket,SHUT_RDWR);
	close(socket);
}

cgicc_connection &scgi_session::get_connection()
{
	if(cgi_ptr){
		return *this;
	}
	throw cppcms_error("SCGI session not prepared");
}

scgi_api::scgi_api(string socket,int backlog)
{
	fd=-1;
	size_t p;
	if((p=socket.find(':'))!=string::npos) {
		struct sockaddr_in a;
		memset (&a, 0, sizeof (a));
		a.sin_family = AF_INET;
		a.sin_port = htons (atoi(socket.c_str()+p+1));
		if(p!=0){
			socket=socket.substr(0,p-1);
			if(inet_aton(socket.c_str(),&a.sin_addr)<0) {
				throw cppcms_error("Invalid IP" + socket);
			}
		}
		fd=::socket(AF_INET,SOCK_STREAM,0);
		if(fd<0) {
			throw cppcms_error(errno,"socket");
		}
		int yes=1;
		if(setsockopt(fd,SOL_SOCKET, SO_REUSEADDR,(char *)&yes,sizeof(yes))<0) {
			throw cppcms_error(errno,"reuse addr");
		}
		if(bind(fd,(struct sockaddr *) &a, sizeof (a)) < 0) {
			throw cppcms_error(errno,"bind");
		}
	}
	else {
		struct sockaddr_un addr;
		fd=::socket(AF_UNIX, SOCK_STREAM, 0);
		if(fd<0) throw cppcms_error(errno,"socket");
		int yes=1;
		if(setsockopt(fd,SOL_SOCKET, SO_REUSEADDR,(char *)&yes,sizeof(yes))<0) {
			throw cppcms_error(errno,"reuse addr");
		}
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, socket.c_str() ,sizeof(addr.sun_path) - 1);
		unlink(socket.c_str());
		if(bind(fd,(struct sockaddr *) &addr,sizeof(addr))<0) {
			throw cppcms_error(errno,"bind");
		}
	}
	listen(fd,backlog);
}

scgi_api::~scgi_api()
{
	if(fd!=-1)
		close(fd);
}

int scgi_api::get_socket()
{
	return fd;
}

cgi_session *scgi_api::accept_session()
{
	int socket=::accept(fd,NULL,NULL);
	if(socket<0) {
		return NULL;
	}
	return new scgi_session(socket);
}

string scgi_session::getenv( char const  *var)
{
	map<string,string>::iterator p;
	p=envmap.find(var);
	if(p==envmap.end())
		return "";
	return p->second;
}

string scgi_session::env(char const *variable) { return getenv(variable);};
cgicc::Cgicc &scgi_session::cgi() { return *cgi_ptr; };
ostream &scgi_session::cout() { return out_stream; };


}; // cppcms
