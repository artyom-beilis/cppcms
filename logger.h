#ifndef CPPCMS_LOGGER_H
#define CPPCMS_LOGGER_H

namespace cppcms {
	class CPPCMS_API logger {
	public:
		class CPPCMS_API ostream_proxy {
		public:
			ostream_proxy(level_type level,char const *module);
			~ostream_proxy()
			ostream_proxy(ostream_proxy const &other);
			ostream_proxy const &operator=(ostream_proxy const &other);

			template<typename T>
			ostream_proxy &operator<<(T const &v) const
			{
				output() << v;
			}
		private:
			struct data; 
			intrusive_ptr<data> d;
		};
		typedef enum {
			none 	= 0,
			fatal	= 10,
			critical= 20,
			error	= 30,
			warning	= 40,
			message	= 50,
			info	= 60,
			debug	= 70,
			all	= 100
		} level_type;

		bool level(level_type l,char const *module);
		ostream_proxy proxy(level_type l,char const *module);
	};

	
	#define CPPCMS_LOG(_l,_m)						\
	::cppcms::logger::instance().level(::cppcms::logger::_l,_m) 		\
		&& ::cppcms::logger::instance().proxy(::cppcms::logger::_l,_m)

	#define CPPCMS_FATAL(_m)	CPPCMS_LOG(fatal,_m)
	#define CPPCMS_CRITICAL(_m)	CPPCMS_LOG(critical,_m)
	#define CPPCMS_ERROR(_m)	CPPCMS_LOG(error,_m)
	#define CPPCMS_WARNING(_m)	CPPCMS_LOG(warning,_m)
	#define CPPCMS_MESSAGE(_m)	CPPCMS_LOG(message,_m)
	#define CPPCMS_INFO(_m)		CPPCMS_LOG(info,_m)
	#define CPPCMS_DEBUG(_m)	CPPCMS_LOG(debug,_m)

}

#endif
