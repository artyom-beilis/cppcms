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
