#ifndef CPPCMS_SESSIOM_MEMORY_STORAGE_H
#define CPPCMS_SESSIOM_MEMORY_STORAGE_H

#include "defs.h"
#include "session_storage.h"
namespace cppcms {
namespace sessions {

	class CPPCMS_API session_memory_storage_factory : public session_storage_factory {
	public:
		session_memory_storage_factory();
		virtual intrusive_ptr<session_storage> get();
		virtual bool requires_gc();
		virtual void gc_job();
		virtual ~session_memory_storage_factory();
	private:
		intrusive_ptr<session_storage> storage_;
	};

} // sessions
} // cppcms 
#endif
