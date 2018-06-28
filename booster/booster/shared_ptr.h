#ifndef BOOSTER_SMART_PTR_SHARED_PTR_HPP_INCLUDED
#define BOOSTER_SMART_PTR_SHARED_PTR_HPP_INCLUDED


#include <memory>

namespace booster
{

	template <typename T>
	using shared_ptr = std::shared_ptr<T>;

	using std::dynamic_pointer_cast;
	using std::static_pointer_cast;
	
} // namespace booster

#endif  // #ifndef BOOST_SMART_PTR_SHARED_PTR_HPP_INCLUDED
