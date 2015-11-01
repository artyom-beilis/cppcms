///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/service.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/http_context.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/mount_point.h>
#include <cppcms/json.h>
#include <cppcms/copy_filter.h>
#include "internal_file_server.h"
#include <iostream>
#include "client.h"
#include "test.h"
#ifdef CPPCMS_WIN_NATIVE
#include <direct.h>
#else
#include <unistd.h>
#endif

int test(std::string const &left,std::string const &expected)
{
	std::string tmp=left;
	cppcms::impl::file_server::normalize_path(tmp);
	if(tmp!=expected) {
		std::cout << "[" << left << "] -> expected [" << expected << "] got [" << tmp << "]\n";
		return 1;
	}
	return 0;
}
int test_normalize()
{
	std::cout << "- Tesing path normalization "<< std::endl;
	int r = 0
	+ test("/foo/bar","/foo/bar")
	+ test("/foo/bar/","/foo/bar")
	+ test("/foo////bar","/foo/bar")
	+ test("///foo/./bar","/foo/bar")
	+ test("///foo/./bar///","/foo/bar")
	+ test("///foo/./bar/","/foo/bar")
	+ test("/../bar","/bar")
	+ test("/..","/")
	+ test("../test/./","/test")
	+ test("/var/www/../../../xx","/xx")
	+ test("/var/www/../../xx","/xx")
	+ test("../test/./x","/test/x")
	+ test("/test/..","/")
	+ test("/test/xx/../","/test")
	+ test("/test/xx/..","/test")
	;
	return r;
}
int main(int argc,char **argv)
{
	if(test_normalize()!=0)
		return 1;
	if(argc<5 || argv[argc-2]!=std::string("-U"))  {
		std::cerr <<"Usage -c config -U dir";
		return 1;
	}
	if(chdir(argv[argc-1])!=0) {
		std::cerr << "Failed to chdir to " << argv[argc-1] << std::endl;
		return EXIT_FAILURE;
	}
	try {
		cppcms::service srv(argc,argv);
		srv.after_fork(submitter(srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	if(!run_ok ) {
		std::cerr << "Python script failed" << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Ok" << std::endl;
	return EXIT_SUCCESS;
}
