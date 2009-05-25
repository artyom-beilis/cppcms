#define CPPCMS_SOURCE
#include "regex.h"
#include <boost/regex.hpp>

namespace cppcms { namespace util {

	struct regex_result::data {
		boost::cmatch match;
	};
	regex_result::regex_result() : d(new data)
	{
	}
	regex_result::~regex_result()
	{
	}
	std::string regex_result::operator[](int n)
	{
		return d->match[n];
	}

	struct regex::data {
		boost::regex r;
		data(std::string const &e) : r(e) {}
	};

	regex::regex(std::string const &e) : d(new data(e))
	{
	}
	regex::~regex()
	{
	}
	bool regex::match(std::string const &str,regex_result &res) const
	{
		return boost::regex_match(str.c_str(),res.d->match,d->r);
	}
	bool regex::match(std::string const &str) const
	{
		return boost::regex_match(str.c_str(),d->r);
	}


}} // cppcms::util
