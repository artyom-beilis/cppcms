#include "text_tool.h"

#include <iostream>

void Text_Tool::init()
{
	ptr=0;
}

void Text_Tool::getline(string &in)
{
	string tmp;
	if(ptr==string::npos){
		input=L_EOF;
		return;
	}
	size_t pos=in.find('\n',ptr);
	if(pos==string::npos){
		if(ptr<in.size()) {
			tmp=in.substr(ptr);
			ptr=string::npos;
		}
		else {
			input=L_EOF;
			return;
		}
	}
	else {
		tmp=in.substr(ptr,pos-ptr);
		ptr=pos+1;
	}

	int blanks=0,i;
	for(i=0;i<tmp.size() && blanks<4;i++) {
		char c=tmp[i];
		if(c=='\r') continue;
		else if(c==' ') blanks++;
		else if(c=='\t') blanks = (blanks | 0x3) + 1;
		else break;
	}
	if(blanks==4) {
		basic_to_html(tmp.substr(i));
		input=L_CODE;
		return;
	}
	int c=tmp[i];
	if(c=='>') {
		to_html(tmp.substr(i+1));
		input=L_QUOTE;
		return;
	}
	if(c=='-' && tmp[i+1]==' ') {
		to_html(tmp.substr(i+2));
		input=L_UL;
		return;
	}
	if(isdigit(c)) {
		int p=i;
		while(isdigit(tmp[p]))p++;
		if(tmp[p]=='.' && tmp[p+1]==' ') {
			to_html(tmp.substr(p+2));
			input=L_OL;
			return;
		}
	}
	if(c=='#') {
		int p=i+1;
		header_level=1;
		while(tmp[p]=='#') { p++; header_level++;}
		if(header_level>6) header_level=6;
		to_html(tmp.substr(p));
		input=L_H;
		return;
	}
	if(tmp.size()-i==0) {
		input=L_BLANK;
		return;
	}
	to_html(tmp);
	input=L_TEXT;
}

void Text_Tool::text2html(char const *s,string &content)
{
	int i;
	content="";
	if(s==NULL) return;
	int len=strlen(s);
	content.reserve(len*3/2);
	for(i=0;i<len;i++) {
		char c=s[i];
		switch(c){
			case '<': content+="&lt;"; break;
			case '>': content+="&gt;"; break;
			case '&': content+="&amp;"; break;
			case '\"': content+="&quot;"; break;
			default: content+=c;
		}
	}
}

void Text_Tool::basic_to_html(string s)
{
	text2html(s,content);
	content+="\n";
}

void Text_Tool::to_html(string s)
{
	int i;
	content="";
	content.reserve(s.size()*3/2);
	bool bold_on=false;
	bool it_on=false;
	enum { B, I };
	int last;

	for(i=0;i<s.size();i++) {
		char c=s[i];
		switch(c){
		case '<': content+="&lt;"; break;
		case '>': content+="&gt;"; break;
		case '&': content+="&amp;"; break;
		case '*':
			if(s[i+1]!=' ' && (i>0 ? s[i-1]==' ':true) && !bold_on) {
				content+="<b>"; bold_on=true; last=B;
			}
			else if(bold_on && s[i+1]==' ') {
				content+="</b>"; bold_on=false;
			}
			else {
				content+='*';
			}
			break;
		case '_':
			if(s[i+1]!=' ' && (i>0 ? s[i-1]==' ':true) && !it_on) {
				content+="<i>"; it_on=true; last=I;
			}
			else if(it_on && s[i+1]==' ') {
				content+="</i>"; it_on=false;
			}
			else {
				it_on+='_';
			}
			break;
		case '-':
			if(s[i+1]=='-') {
				i++;
				content+="&#8212;";
			}
			else content+="-";
			break;
		case '\\':
			if(s[i+1]=='\\') {
				i++;
				content+="<br/>\n";
			}
			else content+="\\";
			break;
		case '[':
			{
				size_t p1,p2;
				if((p1=s.find("](",i))!=string::npos
				    && (p2=s.find(")",p1))!=string::npos)
				{
					string text,url;
					string otext=s.substr(i+1,p1-(i+1));
					string ourl=s.substr(p1+2,p2-(p1+2));
					text2html(otext,text);
					text2html(ourl,url);
					content+="<a href=\"";
					content+=url;
					content+="\">";
					content+=text;
					content+="</a>";
					i=p2;
				}
				else {
					content+="[";
				}
			}
			break;
		default:
			content+=c;
		}
	}
	if(bold_on && it_on && last==I) {
		content+="</i>";
		it_on=false;
	}
	if(bold_on) {
		content+="</b>";
	}
	if(it_on){
		content+="</i>";
	}
	content+="\n";
}

void Text_Tool::markdown2html(char const *c_in,string &out)
{
#warning "Inefficient -- fix me"
	string in=c_in;
	init();
	out="";
	out.reserve(in.size()*3/2);
	state=NOTHING;
	while(state!=FINISH) {
		getline(in);
		switch(state) {
		case NOTHING: break;
		case QUOTE:
			if(input!=L_QUOTE) {
				out+="</p></blockquote>\n";
				state=NOTHING;
			}
			else {
				out+=content;
			}
			break;
		case CODE:
			if(input!=L_CODE) {
				out+="</pre>\n";
				state=NOTHING;
			}
			else {
				out+=content;
			}
			break;
		case UL:
			if(input==L_TEXT){
				out+=content;
			}
			else if(input==L_UL){
				out+="</li>\n";
				out+="<li>\n";
				out+=content;
			}
			else {
				out+="</li></ul>\n";
				state=NOTHING;
			}
			break;
		case OL:
			if(input==L_TEXT){
				out+=content;
			}
			else if(input==L_OL){
				out+="</li>\n";
				out+="<li>\n";
				out+=content;
			}
			else {
				out+="</li></ol>\n";
				state=NOTHING;
			}
			break;
		case P:
			if(input!=L_TEXT) {
				out+="</p>\n";
				state=NOTHING;
			}
			else {
				out+=content;
			}
			break;
		};
		if(state==NOTHING) {
			switch(input){
			case L_BLANK: break;
			case L_TEXT:
				out+="<p>\n";
				out+=content;
				state=P;
				break;
			case L_H:
				{
					char buf[16];
					snprintf(buf,16,"<h%d>",header_level);
					out+=buf;
					out+=content;
					snprintf(buf,16,"</h%d>\n",header_level);
					out+=buf;
				}
				break;
			case L_QUOTE:
				out+="<blockquote></p>\n";
				out+=content;
				state=QUOTE;
				break;
			case L_CODE:
				out+="<pre>\n";
				out+=content;
				state=CODE;
				break;
			case L_UL:
				out+="<ul><li>\n";
				out+=content;
				state=UL;
				break;
			case L_OL:
				out+="<ol><li>\n";
				out+=content;
				state=OL;
				break;
			case L_EOF:
				state=FINISH;
				break;
			}
		}
	}
}
