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
#include <iostream>
#include "client.h"
#include "test.h"
#ifdef CPPCMS_WIN_NATIVE
#include <direct.h>
#else
#include <unistd.h>
#endif


int main(int argc,char **argv)
{
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
