#ifndef BOOSTER_AIO_ENDPOINT_H
#define BOOSTER_AIO_ENDPOINT_H

#include <booster/config.h>
#include <booster/copy_ptr.h>
#include <booster/aio/types.h>
#include <string>

extern "C" {
	struct sockaddr;
};

namespace booster {
namespace aio {
	class BOOSTER_API endpoint {
	public:
		endpoint();
		endpoint(endpoint const &);
		endpoint const &operator = (endpoint const &);
		~endpoint();

		// Inet protocols 
		endpoint(std::string const &ip,int port);

		void ip(std::string const &);
		void port(int);

		std::string ip() const;
		int port() const;

#ifndef BOOSTER_WIN32
		// Unix domain protocols
		endpoint(std::string const &);
		void path(std::string const &);
		std::string path() const;
#endif

		family_type family() const;
		
		void raw(sockaddr const *p,int);
		typedef std::pair<sockaddr const *,int> native_address_type;
		native_address_type raw() const;

	private:
		void throw_invalid() const;
		struct data;
		copy_ptr<data> d;
	};
} // aio
} // booster



#endif
