#include <cppcms/plugin.h>
#include <ctype.h>

struct foo {
	static std::string lower(std::string f)
	{
		for(size_t i=0;i<f.size();i++) {
			f[i]=tolower(f[i]);
		}
		return f;
	}
};

CPPCMS_PLUGIN_ENTRY(std::string(std::string const &),foo::lower)
