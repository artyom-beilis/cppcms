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
#include <iomanip>


namespace booster {

    namespace stack_trace {
        #if defined(BOOSTER_HAVE_EXECINFO)
        
        int trace(void **array,int n)
        {
            return :: backtrace(array,n);
        }
        
        #elif defined(BOOSTER_MSVC) && ( defined(BOOSTER_IS_X86) || defined(BOOSTER_IS_X86_64) )
        namespace {
            struct stack_frame {
                struct stack_frame* next;
                void* ret;
            };
        }

        int trace(void **array,int n)
        {
            stack_frame *start = 0;
            #ifdef BOOSTER_IS_X86
            __asm mov start,ebp;
            #elif defined(BOOSTER_IS_X86_64)
            __asm mov start,rbp;
            #endif
            int i=0;
            while(i<n && frame!=0) {
                array[i++]=frame->ret;
                frame = frame->next;
            }
            return i;
        }

        #else

        int trace(void ** /*array*/,int /*n*/)
        {
            return 0;
        }

        #endif
        
        #if defined(BOOSTER_HAVE_DLADDR) && defined(BOOSTER_HAVE_ABI_CXA_DEMANGLE)
        
        std::string get_symbol(void *ptr)
        {
            if(!ptr)
                return std::string();
            std::ostringstream res;
            res.imbue(std::locale::classic());
            res << ptr<<": ";
            Dl_info info = {0};
            if(dladdr(ptr,&info) == 0) {
                res << "???";
            }
            else {
                if(info.dli_sname) {
                    int status = 0;
                    char *demangled = abi::__cxa_demangle(info.dli_sname,0,0,&status);
                    if(demangled) {
                        res << demangled;
                        free(demangled);
                    }
                    else {
                        res << info.dli_sname;
                    }
                }
                else {
                    res << "???";
                }

                unsigned offset = (char *)ptr - (char *)info.dli_saddr;
                res << std::hex <<" + 0x" << offset ;

                if(info.dli_fname)
                    res << " in " << info.dli_fname;
            }
           return res.str();
        }

        std::string get_symbols(void *const *addresses,int size)
        {
            std::string res;
            for(int i=0;i<size;i++) {
                std::string tmp = get_symbol(addresses[i]);
                if(!tmp.empty()) {
                    res+=tmp;
                    res+='\n';
                }
            }
            return res;
        }
        void write_symbols(void *const *addresses,int size,std::ostream &out)
        {
            for(int i=0;i<size;i++) {
                std::string tmp = get_symbol(addresses[i]);
                if(!tmp.empty()) {
                    out << tmp << '\n';
                }
            }
            out << std::flush;
        }

        #elif defined(BOOSTER_HAVE_EXECINFO)
        std::string get_symbol(void *address)
        {
            char ** ptr = backtrace_symbols(&address,1);
            try {
                if(ptr == 0)
                    return std::string();
                std::string res = ptr[0];
                free(ptr);
                ptr = 0;
                return res;
            }
            catch(...) {
                free(ptr);
                throw;
            }
        }
        
        std::string get_symbols(void * const *address,int size)
        {
            char ** ptr = backtrace_symbols(address,size);
            try {
                if(ptr==0)
                    return std::string();
                std::string res;
                for(int i=0;i<size;i++) {
                    res+=ptr[i];
                    res+='\n';
                }
                free(ptr);
                ptr = 0;
                return res;
            }
            catch(...) {
                free(ptr);
                throw;
            }
        }

        
        void write_symbols(void *const *addresses,int size,std::ostream &out)
        {
            char ** ptr = backtrace_symbols(addresses,size);
            try {
                if(ptr==0)
                    return;
                for(int i=0;i<size;i++)
                    out << ptr[i] << '\n';
                free(ptr);
                ptr = 0;
                out << std::flush;
            }
            catch(...) {
                free(ptr);
                throw;
            }
        }

        #else

        std::string get_symbol(void *ptr)
        {
            if(!ptr)
                return std::string();
            std::ostringstream res;
            res.imbue(std::locale::classic());
            res << ptr;
            return res.str();
        }

        std::string get_symbols(void *const *ptrs,int size)
        {
            if(!ptrs)
                return std::string();
            std::ostringstream res;
            res.imbue(std::locale::classic());
            write_symbols(ptrs,size,res);
            return res.str();
        }

        void write_symbols(void *const *addresses,int size,std::ostream &out)
        {
            for(int i=0;i<size;i++) {
                if(addresses[i]!=0)
                    out << addresses[i]<<'\n';
            }
            out << std::flush;
        }

        #endif

    } // stack_trace

} // booster

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

