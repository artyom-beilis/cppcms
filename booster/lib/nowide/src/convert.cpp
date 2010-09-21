#define BOOSTER_SOURCE
#include <booster/nowide/convert.h>

#ifdef BOOSTER_WIN_NATIVE

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <vector>

namespace booster {
namespace nowide {

bad_utf::bad_utf() : std::runtime_error("Bad utf-8 or utf-16 sequence")
{
}


std::string convert(wchar_t const *s)
{
	std::string res;
	std::vector<char> buf(wcslen(s)*4+1);
	int n=WideCharToMultiByte(CP_UTF8,0,s,-1,&buf.front(),buf.size(),0,0);
	if(n==0) {
		throw bad_utf();
	}
	res.assign(&buf.front(),n-1);
	return res;
}

std::wstring convert(char const *s)
{
	std::wstring res;
	std::vector<wchar_t> buf(strlen(s)+1);
	int n=MultiByteToWideChar(CP_UTF8,0,s,-1,&buf.front(),buf.size());
	if(n==0) {
		throw bad_utf();
	}
	res.assign(&buf.front(),n-1);
	return res;
}

} // nowide
} // booster

#endif 
