#ifndef _TEMPLATES_H
#define _TEMPLATES_H


#include <string>
#include <stdio.h>
#include <vector>

#include <sys/mman.h>

#include "share/bytecode.h"
#include "textstream.h"

using namespace std;


class Variable {
public: 
	typedef enum { NONE, INTEGER, STRING  } type_t;
private:
	type_t type;
	int int_val;
	char const *str_val;
	string str_data;
public:
	Variable() { type = NONE; str_val=NULL; int_val=0; };
	void str(char const *s) {
		str_data=s; str_val=str_data.c_str(); type = STRING;
	};
	Variable(string &s) {
		str(s.c_str());
	};
	Variable(char const *s) { str_val=s; type = STRING; };
	Variable(int b) { int_val = b; type = INTEGER; };
	void operator=(int b) { *this=Variable(b); };
	void operator=(string &s) { *this=Variable(s); };
	void operator=(const char *s) { *this=Variable(s); };
	void reset() { type = NONE; str_val="" ; };
	int geti() { return int_val; };
	char const *gets(){ return str_val; };
	bool isstr() { return type == STRING; };
	bool isdef() { return type != NONE; };
	bool isint() { return type == INTEGER; };
	type_t get_type() { return type; };
};

class Content {
	int size;
	vector<Variable> vars;
public:
	void reset()
	{
		int i;
		for(i=0;i<size;i++){
			vars.push_back(Variable());
		}
	};
	Content(int size) { 
		this->size=size;
		vars.reserve(size);
		reset();
	};
	Variable &operator[](int i) {
		if(i<0 || i>=size) {
			throw HTTP_Error("Out of content bounds");
		}
		return vars[i];
	};
};

class Base_Template {
	friend class Renderer;
	friend class Templates_Set;

	char const *mem_ptr;
	int len;
public:
	Base_Template() { mem_ptr=NULL;};
	Base_Template(char const *ptr, int l) { mem_ptr=ptr; len=l; };
};

class Templates_Set {
	friend class Base_Template;
	char const *base_ptr;
	vector<Base_Template> templates;
	uint32_t *map;
	int map_size;
	int file_size;
	int fd;
	void setup_map(void);
public:
	Templates_Set() {
		base_ptr=0;
		map=0;
		fd=-1;
	};
	~Templates_Set() {
		if(fd!=-1){
			munmap((void*)base_ptr,file_size);
			close(fd);
		}
		else {
			delete [] base_ptr;
		}
	}
	Base_Template *get(int id);
	void load(char const *file,int use_mmap=-1);
	void load(void);
	
};

class Renderer {
	static bool debug;
	static bool debug_defined;
	vector<Base_Template *> templates_stack;
	vector<int> returns_stack;
	Templates_Set *templates_set;
	int stack_size;

	Base_Template *tmpl;
	int pos;
	Content *content;
public:
	Renderer(Templates_Set &tset,int id,Content &cont);
	int render(string &s);
	int render(Text_Stream &out) { return render(out.text);};
};

#endif /* _TEMPLATES_H */
