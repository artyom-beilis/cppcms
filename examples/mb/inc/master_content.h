#ifndef MASTER_CONTENT_H
#define MASTER_CONTENT_H

#include <cppcms/view.h>
using namespace cppcms;
namespace content {

struct master : public cppcms::base_content {
	std::string main_page;
	std::string media;
};


}

#endif

