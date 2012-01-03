///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_URANDOM_H
#define CPPCMS_URANDOM_H

#include <cppcms/defs.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>

namespace cppcms {

	///
	/// \brief High entropy random number generator
	///
	/// This is cryptographic random number generator that uses /dev/urandom on POSIX platforms
	/// and CryptoAPI's CryptGenRandom function under MS Windows
	///
	class CPPCMS_API urandom_device : public booster::noncopyable {
	public:

		///
		/// Create a new random number generator
		///
		urandom_device();

		///
		/// Destory it
		///
		~urandom_device();
		
		///
		/// Fill a buffer pointer by \a ptr of \a n bytes with random numbers
		///
		void generate(void *ptr,unsigned n);

	private:
		struct _data;
		booster::hold_ptr<_data> d;
		

	};
}


#endif
