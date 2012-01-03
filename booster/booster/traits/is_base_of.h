//
// Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOSTER_IS_BASE_OF_H
#define BOOSTER_IS_BASE_OF_H

namespace booster{
	template<typename A, typename B>
	struct is_same {
		static bool const value = false;
	};

	template<typename A>
	struct is_same<A, A> {
		static bool const value = true;
	};


	template<typename B, typename D>
	struct is_base_of {
		static D * create_d(); 
		static char check(B *);
		static int check(...);
		static bool const value = 
			sizeof check(create_d()) == 1 
			&& !is_same<B volatile const,void volatile const>::value;
	};


} // booster

#endif


