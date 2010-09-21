//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_TEST_H
#define BOOSTER_TEST_H


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
