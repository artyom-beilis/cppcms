#include "json.h"
#include <iostream>
#include <fstream>

int main(int argc,char **argv)
{
	using namespace cppcms;
	if(argc!=3)
		return 1;
	std::ifstream in(argv[2]);
	if(!in)
		return 1;
	
	json::value v;
	if(!v.load(in,true))
		return 1;
	std::string path=argv[1];
	try {
		std::cout<<v.get<double>(path);
		return 0;
	}
	catch(json::bad_value_cast const  &e) {}
	try {
		std::cout<<v.get<std::string>(path);
		return 0;
	}
	catch(json::bad_value_cast const  &e) {}
	try {
		std::vector<std::string> vs=v.get<std::vector<std::string> >(path);
		std::string sep="";
		for(unsigned i=0;i<vs.size();i++) {
			std::cout<<sep<<vs[i];
			sep=" ";
		}
		return 0;
	}
	catch(json::bad_value_cast const  &e) {}
	return 1;
}
