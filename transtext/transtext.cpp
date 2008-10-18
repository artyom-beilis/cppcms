#include "transtext.h"
#include "mo_file.h"
#include <cstring>
#include <cstdlib>
#include <ctype.h>
#include <string>
#include <iostream>

namespace cppcms {

namespace transtext {
using namespace lambda;
using namespace std;

struct default_plural : public plural {
	int operator()(int n) const { return n==1 ? 0 : 1; };
	~default_plural() {};	
};

trans_thread_safe::trans_thread_safe()
{
	data=NULL;
	converter=new default_plural;
}

trans_thread_safe::~trans_thread_safe()
{
	if(data){
		thr_safe_gettext_unload(data);
	}
	delete converter;
}

static plural *get_converter(char const *header)
{
	char const *ptr,*ptr2;
	string tmp;
	plural *result;
	if(!header) goto error;
	ptr=strstr(header,"Plural-Forms:");
	if(!ptr) goto error;
	ptr=strstr(ptr,"plural");
	if(!ptr) goto error;
	if(ptr[6]=='s') { // Prevent detecting plurals as plural
		ptr=strstr(ptr+6,"plural");
	}
	if(!ptr) goto error;
	ptr+=6;
	while(*ptr && isblank(*ptr)) ptr++;
	if(*ptr!='=') goto error;
	ptr++;
	ptr2=strstr(ptr,";");
	if(!ptr2) goto error;
	tmp.append(ptr,ptr2-ptr);
	result=compile(tmp.c_str());
	if(!result) goto error;
	return result;
error:
	return new default_plural();
}

void trans_thread_safe::load(char const * locale,char const *domain_name, char const * dirname)
{
	if(data) thr_safe_gettext_unload(data);
	string path_to_mo_file=string(dirname)+"/" + locale +"/LC_MESSAGES/" + domain_name + ".mo";

	data=thr_safe_gettext_load(path_to_mo_file.c_str());
	char const *header=thr_safe_gettext_text_lookup(data,"",0);
	delete converter;
	converter=get_converter(header);
}

char const *trans_thread_safe::gettext(char const *s) const
{
	char const *t=thr_safe_gettext_text_lookup(data,s,0);
	return t ? t : s;
}

char const *trans_thread_safe::ngettext(char const *s,char const *p,int n) const
{
	int idx=(*converter)(n);
	if(idx<0) idx=0;
	char const *t=thr_safe_gettext_text_lookup(data,s,idx);
	if(!t) {
		return n==1 ? s : p;
	}
	return t;
}


} // namespace transtex

} // namespace cppcms
