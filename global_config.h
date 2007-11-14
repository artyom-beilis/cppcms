#ifndef _GLOBAL_CONFIG_H
#define _GLOBAL_CONFIG_H

#include <string>
#include <map>
#include <worker_thread>

using namespace std;

class Global_Config {
	
	enum { WORD, INT, DOUBLE, STR };
	typedef pair<int,string val> tocken_t;
	
	typedef pair<string,string> key_t;
	map<key_t,long> long_map;
	map<key_t,double> double_map;
	map<key_t,string> string_map;
	string filename;
	int line_counter;
	bool loaded;
	bool get_tocken(FILE *,tocken_t &T);
public:
	
	void load(char const *filename);
	void load(int argc,char *argv[],char const *default=NULL);

	Global_Config() { loaded = false;};
	~Global_Config() {};
	long lval(string major,string minor);
	long lval(string major,string minor,long def);
	double dval(string major,string minor);
	double dval(string major,string minor,double def);
	string const &sval(string major,string minor);
	string sval(string major,string minor,string def);
	
};

extern Global_Config global_config;

#endif /* _GLOBAL_CONFIG_H */
