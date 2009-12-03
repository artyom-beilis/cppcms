#define CPPCMS_SOURCE
#include "base_view.h"
#include "util.h"
#include "cppcms_error.h"

#include <vector>
#include <boost/format.hpp>

namespace cppcms {

struct base_view::data {
	std::ostream *out;
};

base_view::base_view(std::ostream &out) :
	d(new data)
{
	d->out=&out;
}

std::ostream &base_view::out()
{
	return *d->out;
}

base_view::~base_view()
{
}

void base_view::render()
{
}


}// cppcms
