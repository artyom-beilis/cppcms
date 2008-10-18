#include "base_view.h"
#include <list>

class my_hello_world;
namespace view {

struct master : public cppcms::base_content {
	std::string title;
	bool ok;
};

struct data {
	std::string name;
	int val;
	data(char const *n="",int v=0) : name(n),val(v){}
};

struct hello : public master {
	std::string msg;
	std::list<int> numbers;
	std::list<data> lst;
};

};

