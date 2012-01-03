///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/json.h>
#include <cppcms/urandom.h>
#include <cppcms/crypto.h>
#include <iostream>
#include <fstream>

void help()
{
	std::cerr << 
	"Usage: cppcms_make_key --hmac HMAC [--hmac-file hmac_key_file] [--cbc CBC] \n"
	"                    [--cbc-file cbc_key_file.txt] [-h] [--output output_file.js]\n"
	"    -h:     display help\n"
	"    HMAC:   is one of md5, sha1, sha224, sah256, sha384 or sha512\n"
	"    CBC:    is one of aes, aes128, aes192, aes256\n"
	"\n"
	"    If the options --hmac-file or --cbc-file are not given \n"
	"    the keys are created inline withing the json configuration\n"
	"    --output output configuration file, default stdout\n"
	"\n"
	"For example:\n"
	"  cppcms_make_key --hmac sha1 --hmac-file hmac.txt --output config.js\n"
	"  cppcms_make_key --hmac sha1 --cbc aes --hmac-file hmac.txt --cbc-file cbc.txt\n";

}

std::string make_key(int size)
{
	std::vector<unsigned char> key(size,0);
	cppcms::urandom_device r;
	r.generate(&key[0],size);
	std::string s;
	static const char part[]="0123456789abcdef";
	for(int i=0;i<size;i++) {
		s+=part[(key[i]>>4) & 0xF];
		s+=part[key[i] & 0xF];
	}
	return s;
}

bool make_entry(cppcms::json::value &val,std::string type,std::string algo,std::string file,int bytes)
{
	val["session"]["client"][type]=algo;
	std::string key=make_key(bytes);
	if(!file.empty()) {
		std::ofstream f(file.c_str());
		if(!f) {
			std::cerr << "Failed to open file " << file << std::endl;
			return false;
		}
		f << key << std::endl;
		f.close();
		val["session"]["client"][type +"_key_file"]=file;
	}
	else {
		val["session"]["client"][type+"_key"] = key;
	}
	return true;
}

int main(int argc,char **argv)
{
	std::string hmac,hmac_file;
	std::string cbc,cbc_file;
	std::string output;
	for(int i=1;i<argc;i++) {
		std::string arg=argv[i];
		if(arg=="-h") {
			help();
			return 0;
		}
		else if(i+1==argc) {
			help();
			return 1;
		}
		i++;
		std::string next = argv[i];
		if(arg=="--hmac") 
			hmac = next;
		else if(arg=="--cbc") 
			cbc = next;
		else if(arg=="--hmac-file")
			hmac_file = next;
		else if(arg=="--hmac-cbc")
			cbc_file = next;
		else if(arg=="--output")
			output = next;
		else {
			help();
			return 1;
		}
	}
	if(hmac.empty()) {
		help();
		return 1;
	}

	cppcms::json::value val;

	std::auto_ptr<cppcms::crypto::message_digest> digest = cppcms::crypto::message_digest::create_by_name(hmac);
	if(!digest.get()) {
		std::cerr << "Unsupported HMAC " << hmac << std::endl;
		return 1;
	}
	if(!make_entry(val,"hmac",hmac,hmac_file,digest->digest_size()))
		return 1;
	if(!cbc.empty()) {
		std::auto_ptr<cppcms::crypto::cbc> p = cppcms::crypto::cbc::create(cbc);
		if(!p.get()) {
			std::cerr << "Unsupported CBC " << cbc << std::endl;
			return 1;
		}
		if(!make_entry(val,"cbc",cbc,cbc_file,p->key_size()))
			return 1;
	}

	if(output.empty()) {
		val.save(std::cout,cppcms::json::readable);
	}
	else {
		std::ofstream out(output.c_str());
		if(!out) {
			std::cerr << "Failed to create a file " << output << std::endl;
			return 1;
		}
		val.save(out,cppcms::json::readable);
	}
	return 0;

}
