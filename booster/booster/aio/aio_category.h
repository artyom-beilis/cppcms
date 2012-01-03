//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_ERROR_CATEGORY_H
#define BOOSTER_ERROR_CATEGORY_H


#include <booster/system_error.h>

namespace booster {
namespace aio {

	///
	/// \brief This namespace includes Booster.Aio specific error codes
	///
	namespace aio_error {
		enum {
			ok,			///< No error
			canceled,		///< Operation was canceled
			select_failed,		///< It was impossible to perform select operation on the file descriptor
			eof,			///< End of file occured
			invalid_endpoint,	///< The provided endpoint (address) is not valid
			no_service_provided,	///< The io_service was not assigned
			prefork_not_enabled	///< Prefork acceptor support is not enabled
		};

		///
		/// Error category for booster::aio objects
		///
		class BOOSTER_API category : public system::error_category {
		public:
			/// Get category name
			virtual char const *name() const;
			/// Get message from code
			virtual std::string message(int cat) const;
		};
		
		///
		/// get aio category object
		///
		BOOSTER_API aio_error::category const &get_category();
	}

	///
	/// aio category object reference
	///
	static aio_error::category const &aio_error_cat = aio_error::get_category();
}
} // booster


#endif
