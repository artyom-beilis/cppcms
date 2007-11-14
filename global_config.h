#ifndef _GLOBAL_CONFIG_H
#define _GLOBAL_CONFIG_H

#include <string>
#include <map>

#include "http_error.h"

using namespace std;

class Global_Config {
	
	enum { WORD, INT, DOUBLE, STR };
	
	typedef pair<int,string> tocken_t;
	
	typedef pair<string,string> key_t;
	
	std::map<string,long> long_map;
	std::map<string,double> double_map;
	std::map<string,string> string_map;
	
	string filename;
	
	int line_counter;
	bool loaded;
	
	bool get_tocken(FILE *,tocken_t &T);
public:
	
	void load(char const *filename);
	void load(int argc,char *argv[],char const *def=NULL);

	Global_Config() { loaded = false;};
	~Global_Config() {};
	long lval(string major) {
		std::map<string,long>::iterator it;
		if((it=long_map.find(major))!=long_map.end()) {
			return it->second;
		}
		throw HTTP_Error("Undefined configuration "+major);
	};
	long lval(string major,long def) {
		std::map<string,long>::iterator it;
		if((it=long_map.find(major))!=long_map.end()) {
			return it->second;
		}
		return def;			
	};
	double dval(string major){
		std::map<string,double>::iterator it;
		if((it=double_map.find(major))!=double_map.end()) {
			return it->second;
		}
		throw HTTP_Error("Undefined configuration "+major);
	};
	double dval(string major,double def) {
		std::map<string,double>::iterator it;
		if((it=double_map.find(major))!=double_map.end()) {
			return it->second;
		}
		return def;			
	};
	string const &sval(string major){
		std::map<string,string>::iterator it;
		if((it=string_map.find(major))!=string_map.end()) {
			return it->second;
		}
		throw HTTP_Error("Undefined configuration "+major);
	};
	string sval(string major,string def){
		std::map<string,string>::iterator it;
		if((it=string_map.find(major))!=string_map.end()) {
			return it->second;
		}
		return def;			
	};
	
};

extern Global_Config global_config;

#endif /* _GLOBAL_CONFIG_H */
