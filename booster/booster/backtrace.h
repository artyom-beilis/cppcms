//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_BACKTRACE_H
#define BOOSTER_BACKTRACE_H

#include <booster/config.h>
#include <stdexcept>
#include <iosfwd>

namespace booster {

    class BOOSTER_API backtrace {
    public:
        struct frame {
            std::string name;
            std::string demangled_name;
            std::string file;
            size_t offset;
            frame() : offset(0) {}
        };

        backtrace();
        frame get_frame(unsigned i) const;

        virtual ~backtrace() throw()
        {
        }

        size_t stack_size() const
        {
            return size_;
        }
        void *address(unsigned frame_no) const
        {
            if(frame_no < size_)
                return frames_[frame_no];
            return 0;
        }


        std::string name(unsigned frame_no) const
        {
            return get_frame(frame_no).name;
        }
        std::string demangled_name(unsigned frame_no) const
        {
            return get_frame(frame_no).demangled_name;
        }
        std::string file(unsigned frame_no) const
        {
            return get_frame(frame_no).file;
        }
        size_t offset(unsigned frame_no) const
        {
            return get_frame(frame_no).offset;
        }

        static size_t const max_stack_size = 32;

    private:
        void *frames_[max_stack_size];
        size_t size_;
    };

    class exception : public std::exception, public backtrace {
    public:
    };

    class runtime_error: public std::runtime_error, public backtrace {
    public:
        runtime_error(std::string const &s) : std::runtime_error(s) 
        {
        }
    };

    class logic_error: public std::logic_error, public backtrace {
    public:
        logic_error(std::string const &s) : std::logic_error(s) 
        {
        }
    };
    class invalid_argument : public std::invalid_argument, public backtrace {
    public:
        invalid_argument(std::string const &s) : std::invalid_argument(s)
        {
        }
    };

    namespace details {
        class BOOSTER_API trace_manip {
        public:
            trace_manip(backtrace const *tr) :
                tr_(tr)
            {
            }
            std::ostream &write(std::ostream &out) const;
        private:
            backtrace const *tr_;
        };
        inline std::ostream &operator<<(std::ostream &out,details::trace_manip const &t)
        {
            return t.write(out);
        }
    }

    template<typename E>
    details::trace_manip trace(E const &e)
    {
        backtrace const *tr = dynamic_cast<backtrace const *>(&e);
        return details::trace_manip(tr);
    }


} // booster

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

