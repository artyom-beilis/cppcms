#ifndef BOOSTER_NONCOPYABLE_H
#define BOOSTER_NONCOPYABLE_H

namespace booster { 
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
