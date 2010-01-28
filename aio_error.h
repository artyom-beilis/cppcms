#ifndef CPPCMS_AIO_ERROR_H
#define CPPCMS_AIO_ERROR_H

#include "defs.h"

namespace cppcms {
	namespace aio {
		class CPPCMS_API error_code {
		public:
			typedef enum {
				ok 	 	= 0,
				canceled 	= 1,
				system_error	= 2
			} status_type;
			
			error_code();
			error_code(status_type st,int code);	
			error_code(error_code const &other);
			error_code const &operator=(error_code const &other);
			~error_code();

			status_type status() const;
			status(status_type s);

			int code() const;
			void code(int code);

			std::string message() const;
		private:
			status_type status_;
		};

		class CPPCMS_API aio_error : public cppcms_error {
		public:
			aio_error(error_code const &code);
			error_code error() const;
		private:
			error_code error_;
		};
	} // AIO
} // CPPCMS


#endif
