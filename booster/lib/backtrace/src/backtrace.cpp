//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE

#include <booster/backtrace.h>

#if defined(__linux) || defined(__APPLE__) || defined(__sun)
#define BOOSTER_HAVE_EXECINFO
#define BOOSTER_HAVE_DLADDR
#endif

#if defined(__GNUC__)
#define BOOSTER_HAVE_ABI_CXA_DEMANGLE
#endif

#ifdef BOOSTER_HAVE_EXECINFO
#include <execinfo.h>
#endif

#ifdef BOOSTER_HAVE_ABI_CXA_DEMANGLE
#include <cxxabi.h>
#endif

#ifdef BOOSTER_HAVE_DLADDR
#include <dlfcn.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <ostream>
#include <sstream>


namespace booster {

    namespace {
        #ifdef BOOSTER_HAVE_EXECINFO
        int get_backtrace(void **array,size_t n)
        {
            return :: backtrace(array,n);
        }
        #else
        int get_backtrace(void **array,size_t n)
        {
            return 0;
        }
        #endif
        
        #ifdef BOOSTER_HAVE_DLADDR
        backtrace::frame get_frame_from_address(void *ptr)
        {
            backtrace::frame f;
            if(!ptr)
                return f;
            Dl_info info = {0};
            if(dladdr(ptr,&info) == 0)
                return f;
            f.name = info.dli_sname ? info.dli_sname : "???";
            f.file = info.dli_fname ? info.dli_fname : "???";
            f.offset = (char *)ptr - (char *)info.dli_saddr;
            return f;
        }
        #else
        backtrace::frame get_frame_from_address(void *ptr)
        {
            backtrace::frame f;
            if(!ptr)
                return f;
            std::ostringstream ss;
            ss << ptr;
            f.name = ss.str();
            f.file = "???";
            return f;
        }
        #endif

        #ifdef BOOSTER_HAVE_ABI_CXA_DEMANGLE
        std::string demangle(std::string const &in)
        {
            int status = 0;
            std::string demangled_name;
            char *demangled = abi::__cxa_demangle(in.c_str(),0,0,&status);
            if(demangled) {
                try {
                    demangled_name = demangled;
                }
                catch(...) { free(demangled); throw; }
                free(demangled);
            }
            else {
                demangled_name = in;
            }
            return demangled_name;
        }
        #else
        std::string demangle(std::string const &in)
        {
            return in;
        }
        #endif
    }

    backtrace::backtrace() :
        size_ (0)
    {
        memset(frames_,0,sizeof(frames_));
        size_ =  get_backtrace(frames_,max_stack_size);
    }
    backtrace::frame backtrace::get_frame(unsigned frame_no) const
    {
        backtrace::frame f = get_frame_from_address(address(frame_no));
        f.demangled_name = demangle(f.name);
        return f;
    }

    namespace details {
        std::ostream &trace_manip::write(std::ostream &out) const
        {
            if(!tr_) return out;
            size_t n = tr_->stack_size();
            for(size_t i=1;i<n;i++) {
                backtrace::frame f = tr_->get_frame(i);
                out <<'#' <<  i << ' ' << f.demangled_name << " in " << f.file << '\n';
            }
            out << std::flush;
            return out;
        }
    }

} // booster

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

