#include <cppcms/plugin.h>
#include <ctype.h>
#include "plugin_base.h"

namespace foo {

std::string lower(std::string f)
{
	for(size_t i=0;i<f.size();i++) {
		f[i]=tolower(f[i]);
	}
	return f;
}
CPPCMS_PLUGIN_ENTRY(foo,lower,std::string(std::string const &))

class bar : public bar_base {
public:
	bar(std::string const &m) : msg_(m) {}
	virtual	char const *msg() { return msg_.c_str(); };

	static bar *create(std::string const &m)
	{
		return new bar(m);
	}
private:
	std::string msg_;
};
CPPCMS_PLUGIN_ENTRY(foo,bar::create,bar_base *(std::string const &))


int my_counter()
{
	static int value;
	return ++value;
}
CPPCMS_NAMED_PLUGIN_ENTRY(foo,counter,my_counter,int());

} // namespace foo


