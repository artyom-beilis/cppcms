#ifndef BOOSTER_HOLD_PTR_H
#define BOOSTER_HOLD_PTR_H

namespace booster { 

	///
	/// \brief a smart pointer similar to std::auto_ptr but it is non-copyable and
	/// underlying object has same constness as the pointer itself (not like in ordinary pointer).
	///
	template<typename T>
	class hold_ptr {
		T *ptr_;
		hold_ptr(hold_ptr const &other); // non copyable 
		hold_ptr const &operator=(hold_ptr const &other); // non assignable
	public:
		hold_ptr() : ptr_(0) {}
		hold_ptr(T *v) : ptr_(v) {}
		~hold_ptr() 
		{
			if(ptr_) delete ptr_;
		}

		T const *get() const { return ptr_; }
		T *get() { return ptr_; }

		T const &operator *() const { return *ptr_; }
		T &operator *() { return *ptr_; }
		T const *operator->() const { return ptr_; }
		T *operator->() { return ptr_; }
		T *release() { T *tmp=ptr_; ptr_=0; return tmp; }
		void reset(T *p=0)
		{
			if(ptr_) delete ptr_;
			ptr_=p;
		}
		void swap(hold_ptr &other)
		{
			T *tmp=other.ptr_;
			other.ptr_=ptr_;
			ptr_=tmp;
		}
	};
} // booster

#endif
