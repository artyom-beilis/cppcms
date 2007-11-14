#include "global_config.h"
#include <stdio.h>


Global_Config global_config;

bool Global_Config::get_tocken(FILE *f,toncken_t &T)
{
	while(getc(f)!=EOF) {
		
	}
}

void Global_Config::load(char const *fname)
{
	FILE *f=fopen(fname,"r");
	line_counter=1;
	if(!f) {
		throw HTTP_Error(string("Failed to open file:")+fname);
	}
	tocken_t T;
	key_t key;
	int state=0;
	while(get_tocken(f,T) && state != 5) {
		switch(state) {
		case 0: if(T.first != WORD) {
				state=5;
			}else{
				key.first=T.second;
				state=1;
			}
			break;
		case 1: if(T.first != '.')
				state=5;
			else 
				state=2;
			break;
		case 2: if(T.first!=WORD){
				state=5;
			}else{
				state=3;
				key.second=T.second;
			}
			break;
		case 3: if(T.first!= '=') 
				state=5;
			else
				state=4;
			break;
		case 4: if(T.first==INT) {
				long val=atol(T.second.c_str());
				long_map.push_back(key,val);
			}
			else if(T.first==DOUBLE) {
				double val=atof(T.second.c_str());
				double_map.push_back(key,val);
			}
			else if(T.first==STR){
				string_map.push_back(key,T.second);
			}
			else {
				state=5;
				break;
			}
			state=0;
			break;
		}
	}
	fclose(f);
	if(state!=0) {
		HTTP_Error(string("Parsing error at line ")+line_counter);
	}
}
