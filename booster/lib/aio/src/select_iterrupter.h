#ifndef BOOSTER_AIO_SELECT_ITERRUPTER_H
#define BOOSTER_AIO_SELECT_ITERRUPTER_H

#include <booster/aio/types.h>
#include <booster/noncopyable.h>

namespace booster {
	namespace aio {
		namespace impl {
			class select_interrupter : public noncopyable {
			public:
				select_interrupter();
				~select_interrupter();
				
				bool open();
				void notify();
				void clean();
				native_type get_fd();
				void close();
			private:
				native_type read_,write_;
			};

		}
	}
}


#endif
