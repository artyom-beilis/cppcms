#include <booster/streambuf.h>
#include "test.h"
#include <string.h>
#include <iostream>
#include <fstream>

class output : public booster::io_device
{
public:
	output(std::string *b,size_t l=std::string::npos) :
		lim(l),
		buf(b)
	{
	}
	size_t write(char const *p,size_t n)
	{
		if(buf->size()+n > lim)
			n=lim-buf->size();
		buf->append(p,n);
		return n;
	}
private:
	size_t lim;
	std::string *buf;
};

class input : public booster::io_device
{
public:
	input(std::string *b) : pos(0),buf(b) {}
	size_t read(char *p,size_t n)
	{
		if( n + pos > buf->size())
			n=buf->size() - pos;
		memcpy(p,buf->c_str()+pos,n);
		pos+=n;
		return n;
	}
private:
	size_t pos;
	std::string *buf;
};


class seekable : public booster::io_device {
public:
	seekable() : pos(0) {}
	size_t read(char *p,size_t n)
	{
		if(n + pos > dev.size()) {
			if(pos > dev.size())
				n=0;
			else
				n=dev.size() - pos;
		}
		if(n>0)
			memcpy(p,&dev[pos],n);
		pos+=n;
		return n;
	}
	size_t write(char const *p,size_t n)
	{
		if(n+pos>dev.size())
			dev.resize(n+pos,0);
		if(n>0)
			memcpy(&dev[pos],p,n);
		pos+=n;
		return n;
	}

	long long seek(long long p,pos_type how)
	{
		switch(how) {
		case set:
			pos = p;
			return pos;
		case cur:
			pos+=p;
			return pos;
		case end: 
			pos=dev.size()+p;
			return pos;
		default:
			return -1;
		}
	}

private:
	size_t pos;
	std::vector<char> dev;
};


void foo(){}

int main()
{
	try {
		{
			std::cout << "Testing input device" << std::endl;
			std::string test("Hello World\ndone");
			input id(&test);
			booster::streambuf buf;
			buf.device(id);
			std::istream in(&buf);
			std::string s;
			in >> s ;
			TEST(s=="Hello");
			in >> s;
			TEST(s=="World");
			in >> s;
			TEST(s=="done");
			TEST(in.eof());
		}
		{
			std::cout << "Testing input device, small buffer" << std::endl;
			std::string test("Hello World\ndone");
			input id(&test);
			booster::streambuf buf;
			buf.set_buffer_size(2);
			buf.device(id);
			std::istream in(&buf);
			std::string s;
			in >> s ;
			TEST(s=="Hello");
			in >> s;
			TEST(s=="World");
			in >> s;
			TEST(s=="done");
			TEST(in.eof());
		}
		{
			std::cout << "Testing input device, putback" << std::endl;
			std::string test("Hello World\ndone");
			input id(&test);
			booster::streambuf buf;
			buf.set_buffer_size(2);
			buf.device(id);
			std::istream in(&buf);
			std::string s;
			TEST(in.get() == 'H');
			TEST(in.get() == 'e');
			TEST(in.get() == 'l');
			TEST(in.get() == 'l');
			TEST(in.putback('l'));
			TEST(in.putback('l'));
			TEST(in.putback('e'));
			TEST(in.get() == 'e');
			TEST(in.get() == 'l');
			TEST(in.get() == 'l');
			TEST(in.get() == 'o');
			TEST(in.putback('o'));
			TEST(in.get() == 'o');
			TEST(in.get() == ' ');
			TEST(in.unget());
			TEST(in.get() == ' ');
			TEST(!in.putback('x'));
		}
		{
			std::cout << "Testing input device, putback fail" << std::endl;
			std::string test("Hello World\ndone");
			input id(&test);
			booster::streambuf buf;
			buf.set_buffer_size(2);
			buf.device(id);
			std::istream in(&buf);
			std::string s;
			TEST(in.get() == 'H');
			TEST(in.get() == 'e');
			TEST(in.get() == 'l');
			TEST(in.get() == 'l');
			TEST(!in.unget() || !in.unget() || !in.unget());
		}
		{
			std::cout << "Testing output device" << std::endl;
			std::string test;
			output id(&test,5);
			booster::streambuf buf;
			buf.device(id);
			std::ostream out(&buf);
			out << "test";
			TEST(test=="");
			out << std::flush;
			TEST(test=="test");
			TEST(out);
			out << "ab" << std::flush;
			TEST(!out);
			TEST(test=="testa");
		}
		{
			std::cout << "Testing output device, small buffer size" << std::endl;
			std::string test;
			output id(&test);
			booster::streambuf buf;
			buf.set_buffer_size(2);
			buf.device(id);
			std::ostream out(&buf);
			std::string test_msg = "To be or not to be, that is the question!";
			out << test_msg;
			TEST(test_msg.size() - 3 <= test.size());
			TEST(test.size() <= test_msg.size());
			out << std::flush;
			TEST(test==test_msg);
		}
		{
			std::cout << "Testing output device, reset" << std::endl;
			std::string test;
			output id(&test);
			booster::streambuf buf;
			buf.device(id);
			std::ostream out(&buf);
			out << "test";
			buf.reset_device();
			TEST(test=="test");
		}
		{
			std::cout << "Testing seek fault" << std::endl;
			std::string dummy="foo";
			output od(&dummy);
			input id(&dummy);
			booster::streambuf os;
			os.device(od);
			booster::streambuf is;
			is.device(id);
			std::istream in(&is);
			std::ostream out(&os);
			TEST(in);
			TEST(out);
			in.seekg(0);
			TEST(!in);
			out.seekp(0);
			TEST(!out);
		}
		{
			std::cout << "Testing tell fault" << std::endl;
			std::string dummy="foo";
			output od(&dummy);
			input id(&dummy);
			booster::streambuf os;
			os.device(od);
			booster::streambuf is;
			is.device(id);
			std::istream in(&is);
			std::ostream out(&os);
			TEST(in);
			TEST(out);
			TEST(int(in.tellg())==-1);
			TEST(int(out.tellp())==-1);
		}
		{
			std::cout << "Testing random access device" << std::endl;
			seekable sd;
			booster::streambuf buf;
			buf.device(sd);
			std::iostream io(&buf);
			
			TEST(int(io.tellg())==0);
			TEST(int(io.tellp())==0);
			io << "test";
			TEST(int(io.tellg())==4);
			TEST(int(io.tellp())==4);
			TEST(io);
			TEST(io.seekp(1));
			io << "x";
			TEST(int(io.tellp())==2);
			TEST(io.seekg(0));
			TEST(io);
			std::string tmp;
			io >> tmp;
			TEST(tmp=="txst");
			TEST(io);
			TEST(int(io.tellg())==4);
			TEST(int(io.tellp())==4);
			io.clear();
			io << "xxxx";
			TEST(io);
			TEST(int(io.tellp())==8);
			TEST(int(io.tellg())==8);
			TEST(io.seekg(0));
			io >> tmp;
			TEST(tmp=="txstxxxx");
		}
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	std::cout << "ok" << std::endl;
	return 0;

}
