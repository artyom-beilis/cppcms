#include "global_config.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>


namespace cppcms {

bool cppcms_config::get_tocken(FILE *f,tocken_t &T)
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

void cppcms_config::load(char const *fname)
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
					long val=atol(T.second.c_str());
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
					vector<long> vl;
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


void cppcms_config::load(int argc,char *argv[],char const *def)
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

}
