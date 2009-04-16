#ifndef CPPCMS_GLOBAL_CONFIG_H
#define CPPCMS_GLOBAL_CONFIG_H

#include <string>
#include <map>
#include <vector>
#include "cppcms_error.h"
#include <boost/any.hpp>

namespace cppcms {

using namespace std;

class cppcms_config {
public:
	typedef std::map<string,boost::any> data_t;
	typedef std::pair<data_t::const_iterator,data_t::const_iterator> range_t;
	
private:

	enum { WORD, INT, DOUBLE, STR };
	
	typedef std::pair<int,string> tocken_t;
	typedef std::pair<string,string> key_t;

	data_t data;
	string filename;
	int line_counter;
	bool loaded;
	bool get_tocken(FILE *,tocken_t &T);

public:
	template<typename T>
	T const *get_ptr(string const &name) const {
		std::map<string,boost::any>::const_iterator it;
		if((it=data.find(name))==data.end()) {
			return NULL;
		}
		T const *res=boost::any_cast<T>(&it->second);
		if(!res) {
			throw cppcms_error("Type mistmach for field "+name);
		}
		return res;
	}

	template<typename T>
	T const &get(string const &name,T const &def) const
	{
		T const *p=get_ptr<T>(name);
		if(!p) return def;
		return *p;
	}
	template<typename T>
	T const &get(string const &name) const
	{
		T const *p=get_ptr<T>(name);
		if(!p) throw cppcms_error("Configuration parameter "+name+" not found");
		return *p;
	}
	template<typename T>
	T const &get_default(string const &name) const
	{
		T const *p=get_ptr<T>(name);
		static const T v;
		if(!p) return v;
		return *p;
	}

	data_t const &get_data() const 
	{
		return data;
	}

	size_t size() const { return data.size(); }

	void load(char const *filename);
	void load(int argc,char *argv[],char const *def=NULL);

	range_t prefix(string pref) const {
		range_t res;
		res.first=data.lower_bound(pref+'.');
		res.second=data.upper_bound(pref+char('.'+1));
		return res;
	}

	cppcms_config() { loaded = false;}

	// { begin depricated
	int lval(string m) const { return ival(m); }
	int lval(string m,int def) const { return ival(m,def); }
	vector<int> const &llist(string m) const { return ilist(m); }
	// } end depricated
	int ival(string m) const {
		return get<int>(m);
	}
	int ival(string m,int def) const {
		return get<int>(m,def);
	}
	double dval(string m) const {
		return get<double>(m);
	}
	double dval(string m,double def) const {
		return get<double>(m,def); 
	}
	string const &sval(string m) const {
		return get<string>(m);
	}
	string sval(string m,string def) const {
		return get<string>(m,def);
	}
	vector<int> const &ilist(string m) const {
		return get_default<vector<int> >(m);
	}
	vector<double> const &dlist(string m) const {
		return get_default<vector<double> >(m);
	}

	vector<string> const &slist(string m) const {
		return get_default<vector<string> >(m);
	}

};

} // namespace cppcms


#endif /* _GLOBAL_CONFIG_H */
