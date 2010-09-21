#include <booster/nowide/convert.h>
#include <booster/nowide/fstream.h>
#include <booster/nowide/cstdio.h>
#include <iostream>

#include <iostream>
#include "test.h"

#define TEST_THROWS(X,type) 			\
	do { try { X; throw 1; } catch(type const &x) {} catch(...) { throw std::runtime_error("Failed " #X); } }while(0)

int main()
{
	try {
		namespace nw=booster::nowide;
		wchar_t shalom[5] = { 0x05e9,0x05dc,0x05d5,0x05dd, 0};
		#ifdef BOOSTER_WIN_NATIVE
		std::cout << "Testing converting" << std::endl;
		TEST(nw::convert(shalom)=="שלום");
		TEST(nw::convert("שלום")==shalom);
		#endif
		std::cout << "Testing cstdio" << std::endl;
		{
			nw::remove("שלום.txt");
			nw::remove("עולם.txt");
			FILE *f=nw::fopen("שלום.txt","w");
			TEST(f);
			fprintf(f,"test");
			fclose(f);
			f=0;
			TEST(nw::rename("שלום.txt","עולם.txt")==0);
			f=nw::fopen("עולם.txt","r");
			TEST(f);
			char str[16]={0};
			fgets(str,16,f);
			TEST(str==std::string("test"));
			fclose(f);
			TEST(nw::remove("עולם.txt")==0);
			TEST(nw::remove("עולם.txt")<0);
		}
		std::cout << "Testing fstream" << std::endl;
		{
			nw::ofstream fo;
			fo.open("שלום.txt");
			TEST(fo);
			fo<<"test"<<std::endl;
			fo.close();
			#ifdef BOOSTER_WIN_NATIVE
			{
				FILE *tmp=_wfopen((std::wstring(shalom)+L".txt").c_str(),L"r");
				TEST(tmp);
				TEST(fgetc(tmp)=='t');
				TEST(fgetc(tmp)=='e');
				TEST(fgetc(tmp)=='s');
				TEST(fgetc(tmp)=='t');
				TEST(fgetc(tmp)=='\n');
				TEST(fgetc(tmp)==EOF);
				fclose(tmp);
			}
			#endif
			{
				nw::ifstream fi;
				fi.open("שלום.txt");
				TEST(fi);
				std::string tmp;
				fi  >> tmp;
				TEST(tmp=="test");
				fi.close();
			}
			{
				nw::ifstream fi("שלום.txt");
				TEST(fi);
				std::string tmp;
				fi  >> tmp;
				TEST(tmp=="test");
				fi.close();
			}
			{
				nw::ifstream fi("שלום.txt",std::ios::binary);
				TEST(fi);
				std::string tmp;
				fi  >> tmp;
				TEST(tmp=="test");
				fi.close();
			}

			{
				nw::ifstream fi;
				nw::remove("שלום.txt");
				fi.open("שלום.txt");
				TEST(!fi);
			}
			{
				nw::fstream f("שלום.txt",nw::fstream::in | nw::fstream::out | nw::fstream::trunc | nw::fstream::binary);
				TEST(f);
				f << "test2" ;
				std::string tmp;
				f.seekg(0);
				f>> tmp;
				TEST(tmp=="test2");
				f.close();
			}
			nw::remove("שלום.txt");
		}
			
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;

}
