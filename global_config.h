#ifndef _GLOBAL_CONFIG_H
#define _GLOBAL_CONFIG_H

#include <string>
#include <map>
#include <vector>
#include "cppcms_error.h"
#include <boost/any.hpp>

namespace cppcms {

using namespace std;

class cppcms_config {

	enum { WORD, INT, DOUBLE, STR };
	
	typedef std::pair<int,string> tocken_t;
	typedef std::pair<string,string> key_t;

	map<string,boost::any> data;

	string filename;

	int line_counter;

	bool loaded;

	bool get_tocken(FILE *,tocken_t &T);

	template<typename T>
	T const *get(string const &name) const {
		map<string,boost::any>::const_iterator it;
		if((it=data.find(name))==data.end()) {
			return NULL;
		}
		T const *res=boost::any_cast<T>(&it->second);
		if(!res) {
			throw cppcms_error("Type mistmach for field "+name);
		}
		return res;
	};

	template<typename T>
	T const &get(string const &name,T const &def) const
	{
		T const *p=get<T>(name);
		if(!p) return def;
		return *p;
	};
	template<typename T>
	T const &get_throw(string const &name) const
	{
		T const *p=get<T>(name);
		if(!p) throw cppcms_error("Configuration parameter "+name+" not found");
		return *p;
	};
	template<typename T>
	T const &get_nothrow(string const &name) const
	{
		T const *p=get<T>(name);
		static const T v;
		if(!p) return v;
		return *p;
	};


public:

	size_t size() const { return data.size(); };
	void load(char const *filename);
	void load(int argc,char *argv[],char const *def=NULL);

	cppcms_config() { loaded = false;};

	long lval(string m) const {
		return get_throw<long>(m);
	};
	long lval(string m,long def) const {
		return get<long>(m,def);
	};
	double dval(string m) const {
		return get_throw<double>(m);
	};
	double dval(string m,double def) const {
		return get<double>(m,def); 
	};
	string const &sval(string m) const {
		return get_throw<string>(m);
	};
	string sval(string m,string def) const {
		return get<string>(m,def);
	};
	vector<long> const &llist(string m) const {
		return get_nothrow<vector<long> >(m);
	}
	vector<double> const &dlist(string m) const{
		return get_nothrow<vector<double> >(m);
	};

	vector<string> const &slist(string m) const {
		return get_nothrow<vector<string> >(m);
	};

};

} // namespace cppcms


#endif /* _GLOBAL_CONFIG_H */
