#include "url.h"

#include "worker_thread.h"

using namespace std;
using namespace boost;

void URL_Parser::set_regex(char const *r)
{
	if(!patterns)
		reserve(default_size);
	if(filled>=size) {
		throw HTTP_Error("Too many pattersn added use reserve");
	}
	patterns[filled].pattern=regex(r);
}

void URL_Parser::add(char const *exp,int id)
{
	set_regex(exp);
	patterns[filled].type=URL_Def::ID;
	patterns[filled].id=id;
	filled++;
	
}

void URL_Parser::add(char const *exp,URL_Parser &url)
{
	set_regex(exp);
	patterns[filled].type=URL_Def::URL;
	patterns[filled].url=&url;
	filled++;
	
}

void URL_Parser::add(char const *exp,callback_t callback)
{
	set_regex(exp);
	patterns[filled].type=URL_Def::CALLBACK;
	patterns[filled].callback.connect(callback);
	filled++;
}

int URL_Parser::parse(string &query)
{
	int i;
	for(i=0;i<filled;i++) {
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
				patterns[i].callback(	result[1],result[2],
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
