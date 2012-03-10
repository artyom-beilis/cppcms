#include "todec.h"
#include "test.h"
#include <iostream>
#include <sstream>
#include <string>

template<typename T>
void test_single()
{
	using cppcms::impl::cint;
	std::ostringstream a,b;
	a 	<< cint(std::numeric_limits<T>::min()) << " " 
		<< cint(std::numeric_limits<T>::max()) << " " 
		<< cint(T(1)) << " "
		<< cint(T(0)) << " "
		<< cint(T(-1)) << " "
		<< cint(T(10)) << " "
		<< cint(T(-10)) << " "
		<< cint(T(12345)) << " "
		<< cint(T(-12345)) << " ";
	b	<< (std::numeric_limits<T>::min()) << " " 
		<< (std::numeric_limits<T>::max()) << " " 
		<< (T(1)) << " "
		<< (T(0)) << " "
		<< (T(-1)) << " "
		<< (T(10)) << " "
		<< (T(-10)) << " "
		<< (T(12345)) << " "
		<< (T(-12345)) << " ";
	if(a.str()!=b.str()) {
		std::cerr << "Error!!! \n   " << a.str() << "\n   " << b.str() << std::endl;
		TEST(a.str() == b.str());
	}
	else {
		std::cout << "Ok       \n   " << a.str() << "\n   " << b.str() << std::endl;
	}
}

int main()
{
	try {
		test_single<unsigned short>();
		test_single<unsigned int>();
		test_single<unsigned long int>();
		test_single<unsigned long long int>();
		test_single<short>();
		test_single<int>();
		test_single<long int>();
		test_single<long long int>();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok\n";
	return 0;
}
