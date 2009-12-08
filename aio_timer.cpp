#define CPPCMS_SOURCE
#include "asio_config.h"
#include "aio_timer.h"
#include "service.h"
#include "service_impl.h"

#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#   include <boost/date_time/posix_time/posix_time.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
#   include <cppcms_boost/date_time/posix_time/posix_time.hpp>
    namespace boost = cppcms_boost;
#endif

namespace cppcms { namespace aio {

	struct timer::data {
		boost::asio::deadline_timer timer;
		data(boost::asio::io_service  &srv) : timer(srv) {}
	};

	timer::timer(service &srv) : d(new timer::data(srv.impl().get_io_service())) {}
	timer::~timer() {}
	
	namespace {
		void adapter(boost::system::error_code const &e,timer::handler const &h)
		{
			h(e);
		}
	}
	void timer::async_wait(handler const &h)
	{
		d->timer.async_wait(boost::bind(adapter,boost::asio::placeholders::error,h));
	}
	void timer::expires_from_now(int seconds)
	{
		d->timer.expires_from_now(boost::posix_time::seconds(seconds));
	}
	void timer::expires_from_now(int seconds,int mili)
	{
		d->timer.expires_from_now(boost::posix_time::seconds(seconds) + boost::posix_time::milliseconds(mili));
	}
	void timer::expires_at(time_t t)
	{
		d->timer.expires_at(boost::posix_time::from_time_t(t));
	}
	void timer::cancel()
	{
		d->timer.cancel();
	}
	

}} // cppcms::aio

