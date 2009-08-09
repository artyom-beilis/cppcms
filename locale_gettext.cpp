#define CPPCMS_SOURCE
#include "locale_gettext.h"
#include "locale_mo_file.h"
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <cassert>
#include <memory>
#include <map>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>

namespace cppcms {
namespace locale {
namespace impl {
	namespace lambda {
		struct plural { // INTERFACE 
			virtual int operator()(int n) const = 0;
			virtual ~plural(){};
		};
		plural *compile(char const *expr);
	} 


	class trans_thread_safe : public gettext::tr {

		struct default_plural : public lambda::plural {
			int operator()(int n) const { return n==1 ? 0 : 1; };
			~default_plural() {};	
		};

		std::auto_ptr<dictionary> dic_;
		std::auto_ptr<lambda::plural> converter_;

		static lambda::plural *get_converter(char const *header)
		{
			using namespace std;
			char const *ptr,*ptr2;
			string tmp;
			lambda::plural *result;
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
			result=lambda::compile(tmp.c_str());
			if(!result) goto error;
			return result;
		error:
			return new default_plural();
		}
	public:
		trans_thread_safe(dictionary *dic) :
			dic_(dic)
		{
			char const *header=dic_->lookup("",0);
			converter_.reset(get_converter(header));
		}
		virtual ~trans_thread_safe()
		{
		}
		virtual char const *gettext(char const *s) const
		{
			char const *t=dic_->lookup(s,0);
			return t ? t : s;
		}

		virtual char const *ngettext(char const *s,char const *p,int n) const
		{
			int idx=(*converter_)(n);
			if(idx<0) idx=0;
			char const *t=dic_->lookup(s,idx);
			if(!t) {
				return n==1 ? s : p;
			}
			return t;
		}
	};

} // impl


// The actual gettext class

gettext::tr::~tr()
{
}

char const *gettext::tr::gettext(char const *m) const
{
	return m;
}

char const *gettext::tr::ngettext(char const *s,char const *p,int n) const
{
	return n==1 ? s : p;
}


std::locale::id gettext::id;

struct gettext::data {
	typedef std::map<std::string,boost::shared_ptr<tr> > translators_type;
	translators_type translators;
};


gettext::gettext(std::size_t refs) : 
	std::locale::facet(refs),
	d(new data)
{
}
gettext::~gettext()
{
}


bool gettext::load(std::string locale,std::string dir,std::string domain)
{
	std::auto_ptr<impl::dictionary> dic;

	static boost::regex lreg("^([a-zA-Z]+)(_([a-zA-Z])+)?(\\.([a-zA-Z0-9_\\-]+))?(\\@(.*))?$");
	boost::cmatch m;
	if(!boost::regex_match(locale.c_str(),m,lreg)) {
		return false;
	}
	
	std::string variants[4]={
		m[1]+m[2]+m[6],
		m[1]+m[2],
		m[1]+m[6],
		m[1]
	};
	for(int i=0;i<4;i++) {
		std::string path=dir + "/" + variants[i] +"/LC_MESSAGES/" + domain + ".mo";
		dic.reset(impl::dictionary::load(path.c_str()));
		if(dic.get()) {
			d->translators[domain].reset(new impl::trans_thread_safe(dic.release()));
			return true;
		}
	}
	return false;
}


gettext::tr const &gettext::do_dictionary(char const *domain) const
{
	data::translators_type::const_iterator p=d->translators.find(domain);
	if(p == d->translators.end()) {
		static const gettext::tr default_one;
		return default_one;
	}
	return *p->second;
}


namespace impl {
namespace lambda {

#define LOG(x) 

struct identity : public plural {
	virtual int operator()(int n) const { LOG(cout<<"id("<<n<<")\n";) return n; };
};

struct unary : public plural {
	plural *op1;
	unary(plural *ptr): op1(ptr) {};
	virtual ~unary() { delete op1; };
};


struct binary : public plural {
	plural *op1,*op2;
	binary(plural *p1,plural *p2): op1(p1),op2(p2) {};
	virtual ~binary() { delete op1; delete op2; };
};

struct number : public plural {
	int val;
	number(int v) : val(v) {};
	virtual int operator()(int n) const { LOG(cout<<"value="<<val<<endl;) return val; };
};

#ifdef DEBUG_LAMBDA
#define UNOP(name,oper) \
struct name: public unary { name(plural *op) : unary(op) {};\
virtual int operator()(int n) const { int v=(*op1)(n); cerr<<#oper<<v<<endl; return oper(v); }; };

#define BINOP(name,oper) \
struct name : public binary { name(plural *p1,plural *p2) : binary(p1,p2) {}; virtual int operator()(int n) const \
{ int v1=(*op1)(n); int v2=(*op2)(n); cout<<v1<<#oper<<v2<<endl; return v1 oper v2; }; };

#define BINOPD(name,oper) \
struct name : public binary { \
name(plural *p1,plural *p2) : binary(p1,p2) {};\
virtual int operator()(int n) const { int v1=(*op1)(n); int v2=(*op2)(n); \
cout<<v1<<#oper<<v2<<endl; \
return v2==0 ? 0 : v1 oper v2; };\
};
#else
#define UNOP(name,oper) \
struct name: public unary { name(plural *op) : unary(op) {}; virtual int operator()(int n) const { return  oper (*op1)(n); }; };

#define BINOP(name,oper) \
struct name : public binary { name(plural *p1,plural *p2) : binary(p1,p2) {}; virtual int operator()(int n) const { return (*op1)(n) oper (*op2)(n); }; };

#define BINOPD(name,oper) \
struct name : public binary { \
name(plural *p1,plural *p2) : binary(p1,p2) {};\
virtual int operator()(int n) const { int v1=(*op1)(n); int v2=(*op2)(n); return v2==0 ? 0 : v1 oper v2; };\
};
#endif
enum { END = 0 , SHL = 256,  SHR, GTE,LTE, EQ, NEQ, AND, OR, NUM, VARIABLE };

UNOP(l_not,!)
UNOP(minus,-)
UNOP(bin_not,~)

BINOP(mul,*)
BINOPD(div,/)
BINOPD(mod,%)
static int level10[]={3,'*','/','%'};

BINOP(add,+)
BINOP(sub,-)
static int level9[]={2,'+','-'};

BINOP(shl,<<)
BINOP(shr,>>)
static int level8[]={2,SHL,SHR};

BINOP(gt,>)
BINOP(lt,<)
BINOP(gte,>=)
BINOP(lte,<=)
static int level7[]={4,'<','>',GTE,LTE};

BINOP(eq,==)
BINOP(neq,!=)
static int level6[]={2,EQ,NEQ};

BINOP(bin_and,&)
static int level5[]={1,'&'};

BINOP(bin_xor,^)
static int level4[]={1,'^'};

BINOP(bin_or,|)
static int level3[]={1,'|'};

BINOP(l_and,&&)
static int level2[]={1,AND};

BINOP(l_or,||)
static int level1[]={1,OR};

struct conditional : public plural {
	plural *op1,*op2,*op3;
	conditional(plural *p1,plural *p2,plural *p3): op1(p1),op2(p2),op3(p3) {};
	virtual ~conditional() { delete op1; delete op2; delete op3; };
	virtual int operator()(int n) const { return (*op1)(n) ? (*op2)(n) : (*op3)(n); };
};


plural *bin_factory(int value,plural *left,plural *right)
{
	switch(value) {
	case '/': return new div(left,right);
	case '*': return new mul(left,right);
	case '%': return new mod(left,right);
	case '+': return new add(left,right);
	case '-': return new sub(left,right);
	case SHL: return new shl(left,right);
	case SHR: return new shr(left,right);
	case '>': return new  gt(left,right);
	case '<': return new  lt(left,right);
	case GTE: return new gte(left,right);
	case LTE: return new lte(left,right);
	case  EQ: return new  eq(left,right);
	case NEQ: return new neq(left,right);
	case '&': return new bin_and(left,right);
	case '^': return new bin_xor(left,right);
	case '|': return new bin_or (left,right);
	case AND: return new l_and(left,right);
	case  OR: return new l_or(left,right);
	default:
		delete left;
		delete right;
		return NULL;
	}
}

plural *un_factory(int value,plural *op)
{
	switch(value) {
	case '!': return new l_not(op);
	case '~': return new bin_not(op);
	case '-': return new minus(op);
	default:
		delete op;
		return NULL;
	}
}

static inline bool is_in(int v,int *p)
{
	int len=*p;
	p++;
	while(len && *p!=v) { p++;len--; }
	return len;
}


class tockenizer {
	char const *text;
	int pos;
	int next_tocken;
	int int_value;
	void step() 
	{
		while(text[pos] && isblank(text[pos])) pos++;
		char const *ptr=text+pos;
		char *tmp_ptr;
		if(strncmp(ptr,"<<",2)==0) { pos+=2; next_tocken=SHL; }
		else if(strncmp(ptr,">>",2)==0) { pos+=2; next_tocken=SHR; }
		else if(strncmp(ptr,"&&",2)==0) { pos+=2; next_tocken=AND; }
		else if(strncmp(ptr,"||",2)==0) { pos+=2; next_tocken=OR; }
		else if(strncmp(ptr,"<=",2)==0) { pos+=2; next_tocken=LTE; }
		else if(strncmp(ptr,">=",2)==0) { pos+=2; next_tocken=GTE; }
		else if(strncmp(ptr,"==",2)==0) { pos+=2; next_tocken=EQ; }
		else if(strncmp(ptr,"!=",2)==0) { pos+=2; next_tocken=NEQ; }
		else if(*ptr=='n') { pos++; next_tocken=VARIABLE; }
		else if(isdigit(*ptr)) { int_value=strtol(text+pos,&tmp_ptr,0); pos=tmp_ptr-text; next_tocken=NUM; }
		else if(*ptr=='\0') { next_tocken=0; }
		else { next_tocken=*ptr; pos++; }
		#ifdef DEBUG_LAMBDA
		if(next_tocken>=' ' && next_tocken<=127)
			std::cout<<"Tocken:"<<(char)next_tocken<<'\n';
		else if(next_tocken==NUM)
			std::cout<<"Number:"<<int_value<<"\n";
		else if(next_tocken==VARIABLE)
			std::cout<<"Variale\n";
		else 
			std::cout<<"TockenID:"<<next_tocken<<'\n';
		#endif
	}
public:
	tockenizer(char const *s) { text=s; pos=0; step(); };
	int get(int *val=NULL){
		int iv=int_value;
		int res=next_tocken;
		step();
		if(val && res==NUM){
			*val=iv;
		}
		return res;
	};
	int next(int *val=NULL) {
		if(val && next_tocken==NUM) {
			*val=int_value;
			return NUM;
		}
		return next_tocken;
	}
};


#define BINARY_EXPR(expr,hexpr,list)					\
	plural *expr()							\
	{								\
		LOG(std::cout<< #expr<<'\n';)				\
		plural *op1=NULL,*op2=NULL;				\
		if((op1=hexpr())==NULL) goto error;			\
		while(is_in(t.next(),list)) {				\
			LOG(cout<<"Concate with "<<(char)t.next()<<endl;)	\
			int o=t.get();					\
			if((op2=hexpr())==NULL) goto error;		\
			op1=bin_factory(o,op1,op2);			\
			assert(op1);					\
			op2=NULL;					\
		}							\
		return op1;						\
	error:								\
		delete op1;						\
		delete op2;						\
		return NULL;						\
	}								

struct parser {
	tockenizer &t;
	parser(tockenizer &tin) : t(tin) {};

	plural *value_expr()
	{
		LOG(std::cout<<"Value\n";)
		plural *op=NULL;
		if(t.next()=='(') {
			LOG(std::cout<<"Value ()\n";)
			t.get();
			if((op=cond_expr())==NULL) goto error;
			if(t.get()!=')') goto error;
			return op;
		}
		else if(t.next()==NUM) {
			int value;
			t.get(&value);
			LOG(std::cout<<"Value ("<<value<<")\n";)
			return new number(value);
		}
		else if(t.next()==VARIABLE) {
			t.get();
			LOG(std::cout<<"Value (n)\n";)
			return new identity();
		}
		return NULL;
	error:
		delete op;
		return NULL;
	};

	plural *un_expr()
	{
		plural *op1=NULL;
		static int level_unary[]={3,'-','!','~'};
		if(is_in(t.next(),level_unary)) {
			int op=t.get();
			if((op1=un_expr())==NULL) 
				goto error;
			switch(op) {
			case '-': 
				LOG(std::cout<<"Unary(-)\n";)
				return new minus(op1);
			case '!': 
				LOG(std::cout<<"Unary(!)\n";)
				return new l_not(op1);
			case '~': 
				LOG(std::cout<<"Unary(~)\n";)
				return new bin_not(op1);
			default:
				goto error;
			}
		}
		else {
			LOG(std::cout<<"Unary... none\n";)
			return value_expr();
		}
	error:
		delete op1;
		return NULL;
	};

	BINARY_EXPR(l10,un_expr,level10);
	BINARY_EXPR(l9,l10,level9);
	BINARY_EXPR(l8,l9,level8);
	BINARY_EXPR(l7,l8,level7);
	BINARY_EXPR(l6,l7,level6);
	BINARY_EXPR(l5,l6,level5);
	BINARY_EXPR(l4,l5,level4);
	BINARY_EXPR(l3,l4,level3);
	BINARY_EXPR(l2,l3,level2);
	BINARY_EXPR(l1,l2,level1);

	plural *cond_expr()
	{
		plural *cond=NULL,*case1=NULL,*case2=NULL;
		if((cond=l1())==NULL)
			goto error;
		if(t.next()=='?') {
			LOG(std::cout<<"Condtion... make\n";)
			t.get();
			if((case1=cond_expr())==NULL)
				goto error;
			if(t.get()!=':')
				goto error;
			if((case2=cond_expr())==NULL)
				goto error;
		}
		else {
			LOG(std::cout<<"Condtion... none\n";)
			return cond;
		}
		return new conditional(cond,case1,case2);
	error:
		delete cond;
		delete case1;
		delete case2;
		return NULL;
	}
public:
	plural *compile()
	{
		plural *res=cond_expr();
		if(res && t.next()!=END) {
			delete res;
			return NULL;
		};
		return res;
	}
};
#ifdef DEBUG_LAMBDA
string printtree(plural *p)
{
	identity *pi=dynamic_cast<identity *>(p);
	if(pi) { return string("n"); };
	unary *pu=dynamic_cast<unary *>(p);
	if(pu) { return string(typeid(*pu).name())+"("+printtree(pu->op1)+")"; }
	binary *pb=dynamic_cast<binary *>(p);
	if(pb) { return string(typeid(*pb).name())+"("+printtree(pb->op1)+","+printtree(pb->op2)+")"; };
	number *pn=dynamic_cast<number *>(p);
	if(pn) { char buf[32]; snprintf(buf,sizeof(buf),"(%d)",pn->val); return string(buf); };
	conditional *pc=dynamic_cast<conditional *>(p);
	if(pc) { return "("+printtree(pc->op1)+")?("+printtree(pc->op2)+"):("+printtree(pc->op3)+")"; };
	return string("PLURAL");
}
#endif

plural *compile(char const *str)
{
	LOG(cout<<"Compiling:["<<str<<"]\n";)
	tockenizer t(str);
	parser p(t);
	plural *tmp=p.compile();
	#ifdef DEBUG_LAMBDA
	cout<<printtree(tmp)<<endl;
	for(int i=0;i<15;i++) {
		cout<<"f("<<i<<")="<<(*tmp)(i)<<endl;
	}	
	#endif
	return tmp;
}


} // lambda
} // impl


} // locale
} // cppcms
