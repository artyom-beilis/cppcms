//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_NONCOPYABLE_H
#define BOOSTER_NONCOPYABLE_H

namespace booster { 
	///
	/// \brief This class makes impossible to copy any class derived from this one.
	///
	class noncopyable {
	private:
		noncopyable(noncopyable const &);
		noncopyable const &operator=(noncopyable const &);
	protected:
		noncopyable(){}
		~noncopyable(){}
	};
}

#endif
