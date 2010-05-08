#ifndef BOOSTER_AIO_SOCKET_H
#define BOOSTER_AIO_SOCKET_H

#include <booster/aio/types.h>
#include <booster/function.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>
#include <booster/aio/endpoint.h>

namespace booster {
namespace aio {
	class mutable_buffer;
	class const_buffer;
	class io_service;
	class endpoint;

	class BOOSTER_API socket : public noncopyable {
	public:
		typedef enum {
			shut_rd,shut_wr,shut_rdwr
		} how_type;

		socket();
		socket(io_service &srv);
		~socket();

		bool has_io_service();
		io_service &get_io_service();
		void set_io_service(io_service &srv);
		void reset_io_service();

		#ifndef BOOSTER_WIN32
		void set_process_shared();
		#endif
		
		void open(family_type d,socket_type t);
		void open(family_type d,socket_type t,system::error_code &e);
		void close();
		void close(system::error_code &e);
		void shutdown(how_type h);
		void shutdown(how_type h,system::error_code &e);

		native_type native();

		void attach(native_type fd);
		void assign(native_type fd);
		native_type release();

		void accept(socket &);
		void accept(socket &,system::error_code &e);

		void connect(endpoint const &);
		void connect(endpoint const &,system::error_code &e);

		void bind(endpoint const &);
		void bind(endpoint const &,system::error_code &e);

		void listen(int backlog);
		void listen(int backlog,system::error_code &e);

		size_t read_some(mutable_buffer const &buffer);
		size_t read_some(mutable_buffer const &buffer,system::error_code &e);
		
		size_t write_some(const_buffer const &buffer);
		size_t write_some(const_buffer const &buffer,system::error_code &e);

		size_t read(mutable_buffer const &buffer);
		size_t write(const_buffer const &buffer);
		
		size_t read(mutable_buffer const &buffer,system::error_code &e);
		size_t write(const_buffer const &buffer,system::error_code &e);


		void on_readable(event_handler const &r);
		void on_writeable(event_handler const &r);
		void cancel();

		void async_read_some(mutable_buffer const &buffer,io_handler const &h);
		void async_write_some(const_buffer const &buffer,io_handler const &h);
		void async_accept(socket &,event_handler const &h);
		void async_connect(endpoint const &,event_handler const &h);

		void async_read(mutable_buffer const &buffer,io_handler const &h);
		void async_write(const_buffer const &buffer,io_handler const &h);

		endpoint local_endpoint(system::error_code &e);
		endpoint local_endpoint();
		endpoint remote_endpoint(system::error_code &e);
		endpoint remote_endpoint();

		typedef enum {
			tcp_no_delay,
			keep_alive,
			reuse_address
		} boolean_option_type;

		bool get_option(boolean_option_type opt,system::error_code &e);
		bool get_option(boolean_option_type opt);
		void set_option(boolean_option_type opt,bool v,system::error_code &e);
		void set_option(boolean_option_type opt,bool v);
		
		void set_non_blocking(bool nonblocking);
		void set_non_blocking(bool nonblocking,system::error_code &e);

		static bool would_block(system::error_code const &e);

	private:
		bool dont_block(event_handler const &);
		bool dont_block(io_handler const &);
		
		int writev(const_buffer const &b);
		int readv(mutable_buffer const &b);

		struct data;
		hold_ptr<data> d;
		native_type fd_;
		bool owner_;
		bool nonblocking_was_set_;
		bool process_shared_;
		io_service *srv_;
	};


	///
	/// Create a connected pair of sockets, under UNIX creates unix-domain-sockets
	/// under windows AF_INET sockets
	///
	void socket_pair(socket_type t,socket &s1,socket &s2,system::error_code &e);
	void socket_pair(socket_type t,socket &s1,socket &s2);

} // aio
} // booster

#endif
