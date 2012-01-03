//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_AIO_SELECT_ITERRUPTER_H
#define BOOSTER_AIO_SELECT_ITERRUPTER_H

#include <booster/aio/types.h>
#include <booster/noncopyable.h>

namespace booster {
	namespace aio {
		namespace impl {
			//
			// Select interrupter, simple class that works like pipe or socket
			// pair that can be polled in select and notifid from one side
			//
			class select_interrupter : public noncopyable {
			public:
				//
				// Create new interrupter, file descriptors are not opened
				//
				select_interrupter();
				//
				// Destroy it and close fds
				//
				~select_interrupter();
				
				//
				// Returns true if the interrupter wasn't opened (i.e. no pipe was created)
				// if it is already opened returns false.
				//
				// Why not open in constructor? File descriptors are shared between processes, so 
				// in order to allow running io_service **after** fork we need to open it after
				// fork.
				//
				bool open();
				//
				// Write one byte to one side of the pair. Thread and signal safe.
				//
				void notify();
				//
				// Read some from the second side. Note, you may call this only after checking that
				// file descriptor is ready for read.
				//
				void clean();

				//
				// Get real descriptor for polling in select/poll etc.
				//
				native_type get_fd();
				//
				// Close interrupter's descriptors;
				//
				void close();
			private:
				void set_non_blocking(native_type fd);
				native_type read_,write_;
			};

		}
	}
}


#endif
