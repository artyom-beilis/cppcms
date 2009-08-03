#define CPPCMS_SOURCE
#include "global_config.h"
#include "cppcms_error.h"
#include <map>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <boost/any.hpp>

using namespace std;
namespace cppcms {


class cppcms_config_impl {
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

	cppcms_config_impl() { loaded = false;}

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

bool cppcms_config_impl::get_tocken(FILE *f,tocken_t &T)
{
	int c;
	while((c=fgetc(f))!=EOF) {
		if(c=='.') {
			T.first='.';
			return true;
		}
		else if(c=='{') {
			T.first='{';
			return true;
		}
		else if(c=='}') {
			T.first='}';
			return true;
		}
		else if(c=='=') {
			T.first='=';
			return true;
		}
		else if(c=='\n') {
			line_counter++;
			continue;
		}
		else if(c==' ' || c=='\r' || c=='\t') {
			continue;
		}
		else if(isalpha(c)) {
			T.second="";
			T.second.reserve(32);
			T.second+=(char)c;
			while((c=fgetc(f))!=EOF && (isalnum(c) || c=='_')) {
				T.second+=(char)c;
			}
			if(c!=EOF){
				ungetc(c,f);
			}
			T.first=WORD;
			return true;
		}
		else if(isdigit(c) || c=='-') {
			T.second="";
			T.second.reserve(32);
			T.second+=(char)c;
			T.first=INT;
			while((c=fgetc(f))!=EOF && isdigit(c)) {
				T.second+=(char)c;
			}
			if(c=='.') {
				T.second+='.';
				T.first=DOUBLE;
				while((c=fgetc(f))!=EOF && isdigit(c)) {
					T.second+=(char)c;
				}
			}
			if(T.second=="-" || T.second=="." || T.second=="-.") {
				throw cppcms_error("Illegal charrecters");
			}
			if(c!=EOF) {
				ungetc(c,f);
			}
			return true;
		}
		else if(c=='\"') {
			T.first=STR;
			T.second="";
			T.second.reserve(128);
			for(;;){
				c=fgetc(f);
				if(c=='\\'){
					if((c=fgetc(f))=='\"' ) {
						T.second+='"';
						continue;
					}
					else {
						T.second+='\\';
					}
				}
				if(c==EOF){
					throw cppcms_error("Unexpected EOF ");
				}
				if(c=='\n') line_counter++;
				if(c=='\"') {
					return true;
				}
				T.second+=(char)c;
			}
		}
		else if(c=='#' || c==';'){
			while((c=fgetc(f))!=EOF) {
				if(c=='\n'){
					line_counter++;
					break;
				}
			}
			if(c==EOF) {
				return false;
			}

		}
		else {
			throw cppcms_error(string("Unexpected charrecter")+(char)c);
		}
	}
	return false;
}

void cppcms_config_impl::load(char const *fname)
{
	if(loaded){
		return;
	}
	FILE *f=fopen(fname,"r");
	line_counter=1;
	if(!f) {
		throw cppcms_error(string("Failed to open file:")+fname);
	}
	tocken_t T;
	string key;
	int state=0;
	try{
		while(get_tocken(f,T) && state != -1) {
			switch(state) {
			case 0: if(T.first != WORD) {
					state=-1;
				}else{
					key=T.second;
					state=1;
				}
				break;
			case 1: if(T.first != '.')
					state=-1;
				else
					state=2;
				break;
			case 2: if(T.first!=WORD){
					state=-1;
				}else{
					state=3;
					key+='.';
					key+=T.second;
				}
				break;
			case 3: if(T.first!= '=')
					state=-1;
				else
					state=4;
				break;
			case 4: if(T.first=='{') {
					state=5;
					break;
				}
				if(T.first==INT) {
					int val=atol(T.second.c_str());
					data[key]=val;
				}
				else if(T.first==DOUBLE) {
					double val=atof(T.second.c_str());
					data[key]=val;
				}
				else if(T.first==STR){
					data[key]=T.second;
				}
				else {
					state=-1;
					break;
				}
				state=0;
				break;
			case 5:
				if(T.first==INT || T.first==DOUBLE || T.first==STR) {
					int fp=T.first;
					vector<int> vl;
					vector<double> vd;
					vector<string> vs;
					do {
						if(T.first=='}') {
							state=0;
						}
						else if(T.first==fp){
							switch(T.first) { 
							case INT: vl.push_back(atol(T.second.c_str())); break;
							case DOUBLE: vd.push_back(atof(T.second.c_str())); break;
							case STR: vs.push_back(T.second); break;
							}
						}
						else {
							state=-1;
						}
					}while(state==5 && get_tocken(f,T));

					if(state==0) {
						switch(fp) {
						case INT: data[key]=vl; break;
						case DOUBLE: data[key]=vd; break;
						case STR: data[key]=vs; break;
						};
					}
				}
				else 
					state=-1;
				break;
			}
		}
		if(state!=0) {
			throw cppcms_error("Parsing error");
		}
	}
	catch (cppcms_error &err){
		fclose(f);
		char stmp[32];
		snprintf(stmp,32," at line %d",line_counter);
		throw cppcms_error(string(err.what())+stmp);
	}
	fclose(f);
	loaded=true;
}


void cppcms_config_impl::load(int argc,char *argv[],char const *def)
{
	if(loaded) {
		return;
	}
	char const *def_file=def;
	int i;
	for(i=1;i<argc;i++) {
		if(strncmp(argv[i],"--config=",9)==0) {
			def_file=argv[i]+9;
			break;
		}
		else if(strcmp(argv[i],"-c")==0 && i+1<argc) {
			def_file=argv[i+1];
			break;
		}
	}
	if(def_file==NULL) {
		def_file=getenv("CPPCMS_CONFIG");
	}
	if(def_file==NULL) {
		throw cppcms_error("Configuration file not defined");
	}
	load(def_file);
}


int cppcms_config::ival(std::string m) const
{
	return pimpl_->ival(m);
}
int cppcms_config::ival(std::string m,int def) const
{
	return pimpl_->ival(m,def);
}
double cppcms_config::dval(std::string m) const
{
	return pimpl_->dval(m);
}
double cppcms_config::dval(std::string m,double def) const
{
	return pimpl_->dval(m,def);
}
std::string cppcms_config::sval(std::string m) const
{
	return pimpl_->sval(m);
}
std::string cppcms_config::sval(std::string m,std::string def) const
{
	return pimpl_->sval(m,def);
}
std::vector<int> const &cppcms_config::ilist(std::string m) const
{
	return pimpl_->ilist(m);
}
std::vector<double> const &cppcms_config::dlist(std::string m) const
{
	return pimpl_->dlist(m);
}
std::vector<std::string> const &cppcms_config::slist(std::string m) const
{
	return pimpl_->slist(m);
}
void cppcms_config::load(std::string const &filename)
{
	pimpl_->load(filename.c_str());
}
void cppcms_config::load(int argc,char *argv[],char const *def)
{
	pimpl_->load(argc,argv,def);
}
cppcms_config::cppcms_config():
	pimpl_(new cppcms_config_impl())
{
}
cppcms_config::~cppcms_config()
{
}
cppcms_config::cppcms_config(cppcms_config const &other) :
	pimpl_(other.pimpl_)
{
}

cppcms_config const &cppcms_config::operator=(cppcms_config const &other)
{
	pimpl_=other.pimpl_;
	return *this;
}


} /// cppcms
