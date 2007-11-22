#include "templates.h"

#include "global_config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
using namespace std;


bool Renderer::debug_defined=false;
bool Renderer::debug=false;

Renderer::Renderer(Templates_Set &tset,int id,Content &cont)
{
	if(!debug_defined) {
		debug=global_config.lval("templates.debug_level",0);
		debug_defined=true;
	}
	templates_set=&tset;
	tmpl=tset.get(id);
	if(!tmpl && debug) {
		char buf[32];
		snprintf(buf,32,"%d",id);
		throw HTTP_Error(string("Failed to load template")+buf);
	}
	pos=0;
	stack_size=0;
	content=&cont;
	returns_stack.reserve(10);
	templates_stack.reserve(10);
}

int Renderer::render(string &s)
{
	if(!tmpl) return 0;
	Content &cont=*content;
	Base_Template *tmp_tmpl;
	int templ_id;
	for(;;) {
		if(pos<0 || pos>(int64_t)(tmpl->len-sizeof(Tmpl_Op))){
			throw HTTP_Error("Template overflow");
		}
		Tmpl_Op *op=(Tmpl_Op*)(tmpl->mem_ptr+pos);
		pos+=sizeof(Tmpl_Op);
	
		Variable var;

		switch(op->opcode){
			case 	OP_VAR:
			case	OP_GOTO_IF_TRUE:
			case	OP_GOTO_IF_FALSE:
			case	OP_GOTO_IF_DEF:
			case	OP_GOTO_IF_NDEF:
			case	OP_INCLUDE_REF:	
				var=cont[op->parameter];
				break;
		}
		switch(op->opcode) {
			case	OP_INLINE:
				if(pos+op->parameter>tmpl->len){
					throw HTTP_Error("Template overflow");
				}
				s.append(tmpl->mem_ptr+pos,
					 op->parameter);
				pos+=op->parameter;
				break;
			case	OP_CALL:
				return op->parameter;
			case	OP_VAR:
				if(var.isstr()) {
					s+=var.gets();
				}
				else if(debug) {
					cerr<<"A"<<op->parameter<<endl;
					throw HTTP_Error("Undefined variable");
				}
				break;
			case	OP_INCLUDE_REF:
				if(var.isint()){
					templ_id=var.geti();
				}
				else {
					throw HTTP_Error("Undefined variable in INCLUDE_REF");
				}
			case	OP_INCLUDE:
				if(op->opcode==OP_INCLUDE){
					templ_id=op->parameter;
				}
				if((tmp_tmpl=templates_set->get(templ_id))==NULL){
					if(debug)
						throw HTTP_Error("Undefined template");
					break;
				}
				if(returns_stack.size()<(unsigned)stack_size+1){
					returns_stack.push_back(pos);
					templates_stack.push_back(tmpl);
				}
				else {
					returns_stack[stack_size]=pos;
					templates_stack[stack_size]=tmpl;
				}
				tmpl=tmp_tmpl;
				stack_size++;
				pos=0;
				break;
			case	OP_GOTO_IF_TRUE:
				if(var.isint()){
					if(var.geti()){
						pos=op->jump;
					}
				}
				else if(debug){
					cerr<<"B"<<op->parameter<<endl;
					throw HTTP_Error("Undefined variable");
				}
				break;
			case	OP_GOTO_IF_FALSE:
				if(var.isint()){
					if(!var.geti()){
						pos=op->jump;
					}
				}
				else if(debug){
					cerr<<"C"<<op->parameter<<endl;
					throw HTTP_Error("Undefined variable");
				}
				break;
			case	OP_GOTO_IF_DEF:
				if(var.isdef()){
					pos=op->jump;
				}
				break;
			case	OP_GOTO_IF_NDEF:
				if(!var.isdef()){
					pos=op->jump;
				}
				break;
			case	OP_GOTO:
				pos=op->jump;
				break;
			case	OP_STOP:
				if(stack_size==0){
					return 0;
				}
				stack_size--;
				pos=returns_stack[stack_size];
				tmpl=templates_stack[stack_size];
				break;
			default:
				throw HTTP_Error("Unknown opcode");
		}
	}
}

void Templates_Set::load()
{
	load(global_config.sval("templates.file").c_str());
}

void Templates_Set::load(char const *file,int use_mmap)
{
	if(use_mmap==-1) {
		use_mmap=global_config.lval("templates.use_mmap",0);
	}
	if(use_mmap) {
		// Rationale:
		// In case of many processes openning same file
		struct stat statbuf;
		char *buf;
		if((fd=open(file,O_RDONLY))<0 
		    || fstat (fd,&statbuf) < 0
		    || (buf=(char*)mmap(0,statbuf.st_size,
					PROT_READ,MAP_SHARED, fd, 0))==(char*)-1)
		{
			throw HTTP_Error(string("Falied to open file:")+file);
		}
		file_size=statbuf.st_size;
		base_ptr=buf;
	}
	else {
		FILE *f;
		f=fopen(file,"r");
		if(!f) {
			throw HTTP_Error(string("Falied to open file:")+file);
		}
		fseek(f,0,SEEK_END);
		file_size=ftell(f);
		rewind(f);
		char *buf=new char [file_size];
		base_ptr=buf;
		if((int)fread(buf,1,file_size,f)!=file_size) {
			fclose(f);
			throw HTTP_Error(string("Falied to read file:")+file);
		}
		fclose(f);
	}
	setup_map();
}

void Templates_Set::setup_map()
{
	if(file_size<4) {
		throw HTTP_Error("Incorrect file format");
	}
	map_size=*(uint32_t*)base_ptr;
	if(4+4*map_size>file_size || map_size<0){
		throw HTTP_Error("Incorrect file format");
	}
	map=((uint32_t*)base_ptr)+1;
	templates.reserve(map_size);
	int offset=4+4*map_size;
	int i;
	for(i=0;i<map_size;i++){
		if((int64_t)map[i]+offset>file_size){
			HTTP_Error("Incorrect file format");
		}
		templates.push_back(Base_Template(base_ptr+offset,map[i]));
		offset+=map[i];
	}
	if(offset!=file_size)
		HTTP_Error("Incorrect file format");
}

Base_Template *Templates_Set::get(int id)
{
	if(id<0 || id>=map_size) {
		return NULL;
	}
	return &(templates[id]);
}
