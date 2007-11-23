#include "url.h"

#include "worker_thread.h"

using namespace std;
using namespace boost;

URL_Parser::~URL_Parser()
{
	unsigned i;
	for(i=0;i<patterns.size();i++) {
		delete patterns[i].callback;
	}
}

void URL_Parser::add(char const *exp,int id)
{
	URL_Def url_def;
	
	url_def.pattern=regex(exp);
	url_def.type=URL_Def::ID;
	url_def.id=id;
	url_def.callback=NULL;
	url_def.url=NULL;
	
	patterns.push_back(url_def);
}

void URL_Parser::add(char const *exp,URL_Parser &url)
{
	URL_Def url_def;
	
	url_def.pattern=regex(exp);
	url_def.type=URL_Def::URL;
	url_def.id=0;
	url_def.callback=NULL;
	url_def.url=&url;
	
	patterns.push_back(url_def);
}

void URL_Parser::add(char const *exp,callback_t callback)
{
	URL_Def url_def;
	
	callback_signal_t *signal = new callback_signal_t;
	signal->connect(callback);
	
	url_def.pattern=regex(exp);
	url_def.type=URL_Def::CALLBACK;
	url_def.id=0;
	url_def.callback=signal;
	url_def.url=NULL;
	
	patterns.push_back(url_def);

}

int URL_Parser::parse(string &query)
{
	unsigned i;
	for(i=0;i<patterns.size();i++) {
		boost::regex &r=patterns[i].pattern;
		string tmp;
		if(boost::regex_match(query.c_str(),result,r)){
			switch(patterns[i].type) {
			case URL_Def::ID: 
				return patterns[i].id;
			case URL_Def::URL: 
				tmp=result[1];
				return patterns[i].url->parse(tmp);
			case URL_Def::CALLBACK:
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

int URL_Parser::parse()
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

string URL_Parser::operator[](int i)
{
	return result[i];
}
