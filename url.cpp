#include "url.h"

#include "worker_thread.h"

using namespace std;
using namespace boost;
using namespace cppcms;
url_parser::~url_parser()
{
	unsigned i;
	for(i=0;i<patterns.size();i++) {
		delete patterns[i].callback;
	}
}

void url_parser::add(char const *exp,int id)
{
	url_def url_def;
	
	url_def.pattern=regex(exp);
	url_def.type=url_def::ID;
	url_def.id=id;
	url_def.callback=NULL;
	url_def.url=NULL;
	
	patterns.push_back(url_def);
}

void url_parser::add(char const *exp,url_parser &url)
{
	url_def url_def;
	
	url_def.pattern=regex(exp);
	url_def.type=url_def::URL;
	url_def.id=0;
	url_def.callback=NULL;
	url_def.url=&url;
	
	patterns.push_back(url_def);
}

void url_parser::add(char const *exp,callback_t callback)
{
	url_def url_def;
	
	callback_signal_t *signal = new callback_signal_t;
	signal->connect(callback);
	
	url_def.pattern=regex(exp);
	url_def.type=url_def::CALLBACK;
	url_def.id=0;
	url_def.callback=signal;
	url_def.url=NULL;
	
	patterns.push_back(url_def);

}

int url_parser::parse(string &query)
{
	unsigned i;
	for(i=0;i<patterns.size();i++) {
		boost::regex &r=patterns[i].pattern;
		string tmp;
		if(boost::regex_match(query.c_str(),result,r)){
			switch(patterns[i].type) {
			case url_def::ID: 
				return patterns[i].id;
			case url_def::URL: 
				tmp=result[1];
				return patterns[i].url->parse(tmp);
			case url_def::CALLBACK:
				(*patterns[i].callback)(result[1],result[2],
							result[3],result[4],
							result[5],result[6],
							result[7],result[8],
							result[0]);
				return 0;
			}
		}
	}
	return -1;
}

int url_parser::parse()
{
	string query;
	if(worker){
		query=worker->env->getPathInfo();
	}
	else {
		return not_found;
	}
	return parse(query);
}

string url_parser::operator[](int i)
{
	return result[i];
}
