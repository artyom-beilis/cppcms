///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_TEST_H
#define CPPCMS_TEST_H


#include <stdexcept>
#include <sstream>


#define TEST(X) 								\
	do {									\
		if(X) break;							\
		std::ostringstream oss;						\
		oss << "Error " << __FILE__ << ":"<<__LINE__ << " "#X;		\
		throw std::runtime_error(oss.str());				\
	}while(0)	


#endif
