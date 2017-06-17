#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/session_interface.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/service.h>

#include <iostream>
#include <sstream>


std::string int2str(int v)
{
	std::ostringstream ss;
	ss << v;
	return ss.str();
}

std::string to_hex(std::string const &v)
{
	std::string result;
	for(size_t i=0;i<v.size();i++) {
		unsigned char c=v[i];
		char const *msg = "0123456789abcdef";
		result += msg[c >>  4];
		result += msg[c & 0xF];
	}
	return result;
}

unsigned char hex2c(char c)
{
	if('0' <= c && c<='9')
		return c-'0';
	if('a' <=c && c<= 'f')
		return c-'a' + 10;
	if('A' <=c && c<= 'F')
		return c-'A' + 10;
	return 0;
}

std::string from_hex(std::string const &v)
{
	std::string r;
	for(size_t i=0;i<v.size() / 2;i++) {
		char h=v[i*2];
		char l=v[i*2+1];
		char v = (hex2c(h) << 4) | hex2c(l);
		r+=v;
	}
	return r;
}

class tester : public cppcms::application {
public:
	tester(cppcms::service &s) : cppcms::application(s)
	{
	}

	void main(std::string /*url*/)
	{
		std::ostringstream final_result;
		try {
			for(int i=1;;i++) {
				std::string id = "_" + int2str(i);
				std::string op = request().get("op" + id);
				if(op.empty())
					break;
				std::string key = request().get("key" + id);
				std::string value = request().get("value" + id);
				std::string result = "ok";
				
				std::cerr << "Got " << op << std::endl;

				if(op=="is_set") {
					result = session().is_set(key) ? "yes" : "no";
				}
				else if(op == "erase")  {
					session().erase(key);
				}
				else if(op == "clear") {
					session().clear();
				}
				else if(op == "is_exposed") {
					result = session().is_exposed(key) ? "yes" : "no";
				}
				else if(op == "expose") {
					session().expose(key,atoi(value.c_str()));
				}
				else if(op == "get") {
					result = session().get(key);
				}
				else if(op == "set") {
					session().set(key,value);
				}
				else if(op == "get_binary") {
					result = to_hex(session().get(key));
				}
				else if(op == "set_binary") {
					session().set(key,from_hex(value));
				}
				else if(op == "get_age") {
					result = int2str(session().age());
				}
				else if(op == "set_age") {
					session().age(atoi(value.c_str()));
				}
				else if(op == "default_age") {
					session().default_age();
				}
				else if(op == "get_expiration") {
					result = int2str(session().expiration());
				}
				else if(op == "set_expiration") {
					session().expiration(atoi(value.c_str()));
				}
				else if(op == "default_expiration") {
					session().default_expiration();
				}
				else if(op == "get_on_server") {
					result = session().on_server() ? "yes" : "no";
				}
				else if(op == "set_on_server") {
					session().on_server(atoi(value.c_str()));
				}
				else if(op == "reset_session") {
					session().reset_session();
				}
				else if(op == "csrf_token") {
					result = "t=" + session().get_csrf_token();
				}
				else if(op == "keys") {
					result = "";
					std::set<std::string> keys = session().key_set();
					for(std::set<std::string>::iterator p=keys.begin();p!=keys.end();++p) {
						if(!result.empty())
							result += ",";
						result += "[" + *p + "]";
					}
				}
				else {
					result = "invalid op=" + op;
				}
				std::cerr << " Res " << result << std::endl;
				final_result << i << ":" << result << ";";
			}
		}
		catch(std::exception const &e) {
			response().set_plain_text_header();
			response().out()  << "ERROR:" << e.what();
			return;
		}
		std::cerr << "OUT: " << final_result.str() << std::endl;
		response().set_plain_text_header();
		response().out() << final_result.str();
	}
};

int main(int argc,char ** argv)
{
    try {
        cppcms::service app(argc,argv);
        app.applications_pool().mount(cppcms::create_pool<tester>());
        app.run();
    }
    catch(std::exception const &e) {
        std::cerr<<e.what()<<std::endl;
    }
}


