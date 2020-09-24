#ifndef BOOSTER_SMART_PTR_WEAK_PTR_HPP_INCLUDED
#define BOOSTER_SMART_PTR_WEAK_PTR_HPP_INCLUDED

#include <memory>
#include <booster/bad_weak_ptr.h>

namespace booster {

template <typename T>
using weak_ptr = std::weak_ptr<T>;

} // namespace booster

#endif  // #ifndef BOOST_SMART_PTR_WEAK_PTR_HPP_INCLUDED
