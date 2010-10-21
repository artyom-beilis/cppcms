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

#if defined(BOOSTER_WIN32)
#include <windows.h>
#include <psapi.h>
#endif

#if defined(BOOSTER_MSVC)
#include <stdlib.h>
#include <dbghelp.h>
#endif



namespace booster {

    namespace stack_trace {
        #if defined(BOOSTER_HAVE_EXECINFO)
        
        int trace(void **array,int n)
        {
            return :: backtrace(array,n);
        }
        
        #elif defined(BOOSTER_MSVC)

        int trace(void **array,int n)
        {
            if(n>=63)
                n=62;
            return RtlCaptureStackBackTrace(0,n,array,0);
        }
        
        #elif defined(BOOSTER_WIN32)

        extern "C" {
            typedef unsigned short (*capture_func_type)(unsigned long, unsigned long ,void **,unsigned long *);
            static capture_func_type capture_stack_trace;
            static bool capture_stack_trace_loaded;
        }

        int trace(void **array,int n)
        {
            if(capture_stack_trace_loaded && !capture_stack_trace)
                return 0;
            if(!capture_stack_trace_loaded) {
                HMODULE h = GetModuleHandle("kernel32.dll");
                if(!h) {
                    capture_stack_trace_loaded = true;
                    return 0;
                }
                capture_stack_trace = (capture_func_type)GetProcAddress(h,"RtlCaptureStackBackTrace");
                capture_stack_trace_loaded = true;
            }
            
            if(!capture_stack_trace)
                return 0;
            
            if(n>=63)
                n=62;

            return capture_stack_trace(0,n,array,0);
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
        
        #elif defined(BOOSTER_MSVC)
        
        namespace {
            HANDLE hProcess = 0;
            bool syms_ready = false;
            
            void init()
            {
                if(hProcess == 0) {
                    hProcess = GetCurrentProcess();
                    SymSetOptions(SYMOPT_DEFERRED_LOADS);

                    if (SymInitialize(hProcess, NULL, TRUE))
                    {
                        syms_ready = true;
                    }
                }
            }
        }
        
        std::string get_symbol(void *ptr)
        {
            if(ptr==0)
                return std::string();
            init();
            std::ostringstream ss;
            ss << ptr;
            if(syms_ready) {
                DWORD64  dwDisplacement = 0;
                DWORD64  dwAddress = (DWORD64)ptr;

                std::vector<char> buffer(sizeof(SYMBOL_INFO) + MAX_SYM_NAME);
                PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)&buffer.front();

                pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                pSymbol->MaxNameLen = MAX_SYM_NAME;

                if (SymFromAddr(hProcess, dwAddress, &dwDisplacement, pSymbol))
                {
                    ss <<": " << pSymbol->Name << std::hex << " +0x" << dwDisplacement;
                }
                else
                {
                    ss << ": ???";
                }
            }
            return ss.str();
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

        #elif defined(BOOSTER_WIN32)
        namespace {
            struct module_info {
                void *ptr;
                void *mptr;
                std::string name;
            };

            void get_modules_info(void *const *addrs,int n,module_info *inf)
            {
                std::vector<HMODULE> mods(1024);
                
                HANDLE hProc = GetCurrentProcess();
                DWORD size=0;
                EnumProcessModules(hProc,&mods[0],sizeof(HMODULE)*mods.size(),&size);
                size /= sizeof(HMODULE);

                std::vector<MODULEINFO> info(size,MODULEINFO());
                std::vector<std::string> names(size);

                for(unsigned i=0;i<size;i++) {
                    GetModuleInformation(hProc,mods[i],&info[i],sizeof(MODULEINFO));
                }
                for(int i=0;i<n;i++) {
                    inf[i].ptr=addrs[i];
                    inf[i].mptr = 0;
                    for(unsigned j=0;j<size;j++) {
                        char *addr = (char*)(addrs[i]);
                        char *maddr = (char*)(info[j].lpBaseOfDll);
                        if(maddr <= addr && addr < maddr + info[j].SizeOfImage) {
                            inf[i].mptr = maddr;
                            if(names[j].empty()) {
                                char module_name[256];
                                GetModuleBaseNameA(hProc,mods[j],module_name,sizeof(module_name));
                                names[j]=module_name;
                            }
                            inf[i].name=names[j];
                            break;
                        }
                    }
                    if(inf[i].name.empty())
                        inf[i].name="???";
                }
            };
        } // anon
        
        std::string get_symbol(void *ptr)
        {
            module_info inf;
            get_modules_info(&ptr,1,&inf);
            std::ostringstream ss;
            ss<< inf.ptr<<" in "<<inf.name << " at " << inf.mptr;
            return ss.str();
        }
        
        std::string get_symbols(void *const *ptrs,int size)
        {
            std::ostringstream ss;
            write_symbols(ptrs,size,ss);
            ss.str();
        }
        
        void write_symbols(void *const *addresses,int size,std::ostream &out)
        {
            if(size==0)
                return;
            std::vector<module_info> inf(size);
            get_modules_info(addresses,size,&inf.front());
            for(int i=0;i<size;i++) {
                out << inf[i].ptr<<" in "<<inf[i].name << " at " << inf[i].mptr << '\n';
            }
            out << std::flush;
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

