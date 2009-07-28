#ifndef CPPCMS_GLOBAL_CONFIG_H
#define CPPCMS_GLOBAL_CONFIG_H

#include <string>
#include <vector>
#include "defs.h"
#include "copy_ptr.h"

namespace cppcms {


class cppcms_config_impl;

class CPPCMS_API cppcms_config {
public:
	int ival(std::string m) const;
	int ival(std::string m,int def) const;
	double dval(std::string m) const;
	double dval(std::string m,double def) const;
	std::string sval(std::string m) const;
	std::string sval(std::string m,std::string def) const;
	std::vector<int> const &ilist(std::string m) const;
	std::vector<double> const &dlist(std::string m) const;
	std::vector<std::string> const &slist(std::string m) const;

	void load(std::string const &filename);
	void load(int argc,char *argv[],char const *def=NULL);

	cppcms_config();
	~cppcms_config();
	cppcms_config(cppcms_config const &other);
	cppcms_config const &operator=(cppcms_config const &other);
private:
	util::copy_ptr<cppcms_config_impl> pimpl_;
};



} // namespace cppcms


#endif /* _GLOBAL_CONFIG_H */
