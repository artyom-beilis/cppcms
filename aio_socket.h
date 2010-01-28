#ifndef CPPCMS_AIO_SOCKET_H
#define CPPCMS_AIO_SOCKET_H

#include "defs.h"
#include "noncopyable.h"
#include "callback1.h"
#include "callback2.h"
#include "hold_ptr.h"


namespace cppcms {
	class service;
	namespace aio {

		struct buffer {
			char *buffer;
			size_t size;
			buffer(std::vector<char> &b) : 
				buffer(&b.front()),
				size(b.size())
			{
			}
			buffer(void *p,size_t s) :
				buffer(p),
				size(s)
			{
			}
		};

		struct const_buffer {
			char const *buffer;
			size_t size;
			buffer(std::vector<char> const &b) : 
				buffer(&b.front()),
				size(b.size())
			{
			}
			buffer(void const *p,size_t s) : 
				buffer(p),
				size(s)
			{
			}
			buffer(std::string const &b) :
				buffer(b.data()),
				size(b.size())
			{
			}
		};

		namespace protocol {
			typedef enum {
				ip_v4 	= 0,
				ip_v6 	= 1,
				unix 	= 2,
			} family_type;

			typedef enum {
				stream 	= 0,
				datagram= 1
			} communication_type;
		} // protocol

		class acceptor : public util::noncopyable {
		public:
			acceptor(service &srv);
			~acceptor();
			
			void open(protocol::family_type family,protocol::communication_type type); 
			
			void bind(std::string ip,int port);
			#ifndef CPPCMS_WIN32
			///
			/// Bind to Unix Domain socket
			///
			void bind(std::string path);
			#endif
			
			void cancel();
			void close();
			
			void listen(int backlog);
			#ifdef CPPCMS_WIN32
			void assign(unsigned);
			unsigned native();
			#else
			int native();
			void assign(int );
			#endif
			
			void async_accept(socket &sock);
			
		};

		class socket : public util::noncopyable {
		public:
			socket(service &);
			~socket();

			typedef util::callback1<error_code> handler;
			typedef util::callback2<error_code,size_t> io_handler;

			void open(protocol::family_type family,protocol::communication_type type);
			void close();

			void async_connect(std::string ip,int port,handler const &h);
			#ifndef CPPCMS_WIN32 
			void async_connect(std::string socket,handler const &h);
			#endif

			void async_read_some(buffer data,io_handler const &h);
			void async_read_some(buffer const *data,unsigned n,io_handler const &h);
			void async_write_some(const_buffer data,io_handler const &h);
			void async_write_some(const_buffer const *data,unsigned n,io_handler const &h);

			void async_send(const_buffer data,io_handler const &h);
			void async_send_to(const_buffer data,std::string ip,int port,io_handler const &h);
			void async_send(const_buffer const *data,unsigned no,io_handler const &h);
			void async_send_to(const_buffer const &data,unsigned no,std::string ip,int port,io_handler const &h);
			
			void async_receive(buffer data,io_handler const &h);
			void async_receive(buffer const *data,size_t n,io_handler const &h);
			void async_receive_from(buffer data,std::string &ip,int &port,io_handler const &h);
			void async_receive_from(buffer const *data,size_t n,std::string &ip,int &port,io_handler const &h);
		private:
			util::hold_ptr<socket_impl> pimpl_;

		}
	}
}


#endif
