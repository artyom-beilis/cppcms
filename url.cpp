#include "url.h"

#include "worker_thread.h"

using namespace std;

std::vector<std::pair<boost::regex,int> > URL_Parser::all_urls;

void URL_Parser::add(char const *exp,int id)
{
	all_urls.push_back(pair<boost::regex,int>(boost::regex(exp),id));
}

void URL_Parser::add(URL_Def *list)
{
	while(list->url) {
		add(list->url,list->id);
		list++;
	}
}

void URL_Parser::init(Worker_Thread *w)
{
	worker=w;
}

int URL_Parser::parse()
{
	unsigned int i;
	string query=worker->env->getPathInfo();
	for(i=0;i<all_urls.size();i++) {
		boost::regex &r=all_urls[i].first;
		if(boost::regex_match(query.c_str(),result,r)){
			return all_urls[i].second;
		}
	}
	return -1;
}

string URL_Parser::operator[](int i)
{
	return result[i];
}
