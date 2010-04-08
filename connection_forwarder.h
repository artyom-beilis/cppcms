#ifndef CPPCMS_CONNECTION_FORWARDER_H
#define CPPCMS_CONNECTION_FORWARDER_H

#include "application.h"

namespace cppcms {

	class CPPCMS_API connection_forwarder : public application {
	public:
		connection_forwarder(cppcms::service &srv,std::string const &ip,int port);
		~connection_forwarder();
		virtual void main(std::string);
	private:

		struct data;
		util::hold_ptr<data> d;
		std::string ip_;
		int port_;
	};
}

#endif
