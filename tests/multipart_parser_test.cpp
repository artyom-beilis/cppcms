///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
//#define DEBUG_MULTIPART_PARSER
#include "multipart_parser.h"
#include <iostream>
#include "test.h"
#include <string.h>

char const test_1_content[] = "multipart/form-data; boundary=xxyy";
char const test_1_file[] = 
"--xxyy\r\n"
"Content-Disposition: form-data; name=\"test1\"; filename=\"foo.txt\"\r\n"
"Content-Type: text/plain\r\n"
"\r\n"
"hello\r\n"
"\r\n"
"--xxyy\r\n"
"Content-Disposition: form-data; name=\"test2\"\r\n"
"\r\n"
"שלום\r\n"
"--xxyy\r\n"
"Content-Disposition: form-data; name=\"test3\"\r\n"
"\r\n"
"x\r"
"\r\n--xxyy\r\n"
"Content-Disposition: form-data; name=\"test4\"\r\n"
"\r\n"
"x\r\n-"
"\r\n--xxyy\r\n"
"Content-Disposition: form-data; name=\"test5\"\r\n"
"\r\n"
"x\r\n--"
"\r\n--xxyy\r\n"
"Content-Disposition: form-data; name=\"test6\"\r\n"
"\r\n"
"x\r\n--x"
"\r\n--xxyy\r\n"
"Content-Disposition: form-data; name=\"test7\"\r\n"
"\r\n"
"x\r\n--x-"
"\r\n--xxyy--\r\n";

char const test_2_content[] = "multipart/form-data; boundary=-AbC";
char const test_2_file[] = 
"---AbC\r\n"
"Content-Disposition: form-data; name=\"test1\"  \r\n"
"\r\n"
"hello"
"\r\n"
"---AbC--\r\n";


std::string getcontent(std::istream &in)
{
	std::string res;
	while(!in.eof()) {
		char c;
		in.get(c);
		if(in.gcount() == 1)
		res+=c;
	}
	return res;
}

struct random_consumer {
	random_consumer(int bs,cppcms::impl::multipart_parser &p,cppcms::impl::multipart_parser::files_type &f) :
		block_size(bs),
		parser(&p),
		files(&f)
	{
	}
	int block_size;
	cppcms::impl::multipart_parser *parser;
	cppcms::impl::multipart_parser::files_type *files;
	cppcms::impl::multipart_parser::parsing_result_type operator()(char const *buffer,int size)
	{
		while(size > 0) {
			int block = 1;
			if(block_size > 1)
				block = (rand() % (block_size-1) + 1);
			else if(block_size < 0) 
				block=size;

			if(block > size)
				block=size;
			cppcms::impl::multipart_parser::parsing_result_type res = parser->consume(buffer,block);
			buffer+=block;
			size-=block;
			if(res==cppcms::impl::multipart_parser::eof) {
				cppcms::impl::multipart_parser::files_type fblock = parser->get_files();
				for(unsigned i=0;i<fblock.size();i++) {
					files->push_back(fblock[i]);
				}
				return res;
			}
			if(res==cppcms::impl::multipart_parser::parsing_error)
				return res;
			if(res==cppcms::impl::multipart_parser::got_something) {
				cppcms::impl::multipart_parser::files_type fblock = parser->get_files();
				TEST(!fblock.empty());
				for(unsigned i=0;i<fblock.size();i++) {
					files->push_back(fblock[i]);
				}
			}
		}
		TEST(!"Never get there");
		return cppcms::impl::multipart_parser::eof;
	}

};

int main(int argc,char **argv)
{
	if(argc!=2) {
		std::cerr << "Required path to tests folder" << std::endl;
		return 1;
	}
	try {
		int block_size[]={ 1, -1, 5, 3, 10 };
		for(unsigned i=0;i<sizeof(block_size)/sizeof(block_size[0]);i++) {
			std::cerr << "Testing for max-block-size = " << block_size[i] << std::endl;
			{
				cppcms::impl::multipart_parser parser;
				TEST(parser.set_content_type(test_1_content));
				cppcms::impl::multipart_parser::files_type files;
				random_consumer c(block_size[i],parser,files);
				TEST(c(test_1_file,strlen(test_1_file))==cppcms::impl::multipart_parser::eof);
				TEST(files.size()==7);
				TEST(files[0]->name()=="test1");
				TEST(files[0]->filename()=="foo.txt");
				TEST(files[0]->mime()=="text/plain");
				std::string content = getcontent(files[0]->data());
				TEST(content=="hello\r\n");
				TEST(files[1]->name()=="test2");
				TEST(files[1]->filename()=="");
				TEST(files[1]->mime()=="");
				TEST(getcontent(files[1]->data())=="שלום");
				TEST(getcontent(files[2]->data())=="x\r");
				TEST(getcontent(files[3]->data())=="x\r\n-");
				TEST(getcontent(files[4]->data())=="x\r\n--");
				TEST(getcontent(files[5]->data())=="x\r\n--x");
				TEST(getcontent(files[6]->data())=="x\r\n--x-");
			}
			{
				cppcms::impl::multipart_parser parser;
				TEST(parser.set_content_type(test_2_content));
				cppcms::impl::multipart_parser::files_type files;
				random_consumer c(block_size[i],parser,files);
				TEST(c(test_2_file,strlen(test_2_file))==cppcms::impl::multipart_parser::eof);
				TEST(files.size()==1);
				TEST(files[0]->name()=="test1");
				TEST(files[0]->filename()=="");
				TEST(files[0]->mime()=="");
				std::string content = getcontent(files[0]->data());
				TEST(content=="hello");
			}
			std::string boundaries[4]= { 
				"multipart/form-data; boundary=---------------------------11395741071221114234100471568",
				"multipart/form-data; boundary=----WebKitFormBoundaryuYwgZwieYHQ3+AR8",
				"multipart/form-data; boundary=----------jrFLC4hxayof1KyJEgmmCw",
				"multipart/form-data; boundary=---------------------------7da3a5810028"
			};
			std::string filenames[4] = {
				"firefox",
				"chrome",
				"opera",
				"ie" 
			};
			for(int i=0;i<4;i++) {
				std::cout << "-- Browser " << filenames[i] << std::endl;
				std::string file_name = std::string(argv[1])+"/multipart/" + filenames[i]+".txt";
				std::ifstream file(file_name.c_str(),std::ifstream::binary);
				if(!file) {
					std::cerr << "Failed to open file " << file_name << std::endl;
					return 1;
				}
				std::string content = getcontent(file);
				cppcms::impl::multipart_parser parser;
				TEST(parser.set_content_type(boundaries[i]));
				cppcms::impl::multipart_parser::files_type files;
				random_consumer c(block_size[i],parser,files);
				TEST(c(content.c_str(),content.size())==cppcms::impl::multipart_parser::eof);
				TEST(files.size()==3);
				TEST(files[0]->name()=="submit-name");
				TEST(files[0]->mime()=="" && files[0]->filename().empty());
				TEST(getcontent(files[0]->data())=="שלום");
				TEST(files[1]->name()=="file");
				TEST(files[1]->mime()=="text/plain");
				if(i==3) // IE
					TEST(files[1]->filename()=="z:\\tmp\\שלום.txt" || files[1]->filename()=="z:tmpשלום.txt");
				else
					TEST(files[1]->filename()=="שלום.txt");
				TEST(getcontent(files[1]->data())=="שלום עולם!\n");
				TEST(files[2]->name()=="submit");
				TEST(files[2]->mime().empty());
				TEST(files[2]->filename().empty());
				TEST(getcontent(files[2]->data())=="Send");
			}
		}
		
	}
	catch(std::exception const &e) {
		std::cerr << "Fail: " <<e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
