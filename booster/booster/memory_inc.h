//
//  Copyright (C) 2009-2020 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_MEMORY_INC_H
#define BOOSTER_MEMORY_INC_H
#include <memory>
namespace booster {
    using std::enable_shared_from_this;
    using std::weak_ptr;
    using std::shared_ptr;
    using std::bad_weak_ptr;
    using std::static_pointer_cast;
    using std::dynamic_pointer_cast;
    using std::const_pointer_cast;
}
#endif
