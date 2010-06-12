#include <booster/nowide/fstream.h>
#include <iostream>

int main()
{
	{
		booster::nowide::ofstream f("test.txt",std::ios_base::trunc | std::ios_base::out);
		f<< "Hello World" << std::endl;;
		f.close();
	}
	{
		booster::nowide::fstream f("test.txt");
		std::string s;
		f >> s;
		std::cout << s << std::endl;
		f.seekp(0,std::ios_base::end);
		f<<s;
		f.close();
	}
	{
		booster::nowide::ifstream f("test.txt");
		char buf[16];
		while(!f.eof()) {
			f.read(buf,16);
			std::cout.write(buf,f.gcount());
		}
		std::cout << std::flush;
	}
	{
		booster::nowide::ifstream f;
		f.open("test.txt");
		char buf[16];
		while(!f.eof()) {
			f.read(buf,16);
			std::cout.write(buf,f.gcount());
		}
		std::cout << std::flush;
	}
}
