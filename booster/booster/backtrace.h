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
#include <typeinfo>
#include <string>
#include <vector>
#include <iosfwd>

namespace booster {

    ///
    /// \brief Namespace that holds basic operations
    /// for implementing stack trace
    ///
    namespace stack_trace {
        ///
        /// \brief Record stack frame
        ///
        /// Records at most \a size stack frames returning the pointers to the running
        /// code into \a addresses vector that should have at least size places
        ///
        /// returns that number of actually recorded frames
        /// 
        BOOSTER_API int trace(void **addresses,int size);
        ///
        /// \brief Print stack trace
        ///
        /// Writes stack trace recorded \ref trace function of \a size size to the output stream
        ///
        BOOSTER_API void write_symbols(void *const *addresses,int size,std::ostream &);
        ///
        /// \brief Get stack trace information about a single address recorded
        ///
        BOOSTER_API std::string get_symbol(void *address);
        ///
        /// \brief Get stack trace information about multiple address recorded
        ///
        BOOSTER_API std::string get_symbols(void * const *address,int size);
    } // stack_trace

    ///
    /// \brief the class that records the stack trace when it is created,
    ///
    /// It is a base class for all exceptions that record stack trace
    ///

    class backtrace {
    public:
       
        ///
        /// The default number of recorded frames 
        ///
        static size_t const default_stack_size = 32;

        ///
        /// Create stack trace recording at most \a frames_no stack frames
        ///
        backtrace(size_t frames_no = default_stack_size) 
        {
            if(frames_no == 0)
                return;
            frames_.resize(frames_no,0);
            int size = stack_trace::trace(&frames_.front(),frames_no);
            frames_.resize(size);
        }

        virtual ~backtrace() throw()
        {
        }

        ///
        /// Get the actual number of recorded stack frames
        ///
        size_t stack_size() const
        {
            return frames_.size();
        }

        ///
        /// Get the returned address for the stack frame number \a frame_no 
        ///
        void *return_address(unsigned frame_no) const
        {
            if(frame_no < stack_size())
                return frames_[frame_no];
            return 0;
        }

        ///
        /// Print the stack trace frame for the frame \a frame_no to the stream \a out
        ///
        void trace_line(unsigned frame_no,std::ostream &out) const
        {
            if(frame_no < frames_.size())
                stack_trace::write_symbols(&frames_[frame_no],1,out);
        }

        ///
        /// Get a readable stack trace frame for the frame \a frame_no
        ///
        std::string trace_line(unsigned frame_no) const
        {
            if(frame_no < frames_.size())
                return stack_trace::get_symbol(frames_[frame_no]);
            return std::string();
        }

        ///
        /// Get full stack trace as a string
        ///
        std::string trace() const
        {
            if(frames_.empty())
                return std::string();
            return stack_trace::get_symbols(&frames_.front(),frames_.size());
        }

        ///
        /// Print full stack trace to a stream \a out
        ///
        void trace(std::ostream &out) const
        {
            if(frames_.empty())
                return;
            stack_trace::write_symbols(&frames_.front(),frames_.size(),out);
        }
    
    private:
        std::vector<void *> frames_;
    };

    ///
    /// \brief Same as std::exception but records stack trace
    ///
    class exception : public std::exception, public backtrace {
    public:
    };
    
    ///
    /// \brief Same as std::bad_cast but records stack trace
    ///
    class bad_cast : public std::bad_cast, public backtrace {
    public:
    };

    ///
    /// \brief Same as std::runtime_error but records stack trace
    ///
    class runtime_error: public std::runtime_error, public backtrace {
    public:
        explicit runtime_error(std::string const &s) : std::runtime_error(s) 
        {
        }
    };

    ///
    /// \brief Same as std::range_error but records stack trace
    ///
    class range_error: public std::range_error, public backtrace {
    public:
        explicit range_error(std::string const &s) : std::range_error(s) 
        {
        }
    };

    ///
    /// \brief Same as std::overflow_error but records stack trace
    ///
    class overflow_error: public std::overflow_error, public backtrace {
    public:
        explicit overflow_error(std::string const &s) : std::overflow_error(s) 
        {
        }
    };

    ///
    /// \brief Same as std::underflow_error but records stack trace
    ///
    class underflow_error: public std::underflow_error, public backtrace {
    public:
        explicit underflow_error(std::string const &s) : std::underflow_error(s) 
        {
        }
    };

    ///
    /// \brief Same as std::logic_error but records stack trace
    ///
    class logic_error: public std::logic_error, public backtrace {
    public:
        explicit logic_error(std::string const &s) : std::logic_error(s) 
        {
        }
    };

    ///
    /// \brief Same as std::domain_error but records stack trace
    ///
    class domain_error: public std::domain_error, public backtrace {
    public:
        explicit domain_error(std::string const &s) : std::domain_error(s) 
        {
        }
    };

    ///
    /// \brief Same as std::length_error but records stack trace
    ///
    class length_error: public std::length_error, public backtrace {
    public:
        explicit length_error(std::string const &s) : std::length_error(s) 
        {
        }
    };

    ///
    /// \brief Same as std::invalid_argument but records stack trace
    ///
    class invalid_argument : public std::invalid_argument, public backtrace {
    public:
        explicit invalid_argument(std::string const &s) : std::invalid_argument(s)
        {
        }
    };
    
    ///
    /// \brief Same as std::out_of_range but records stack trace
    ///
    class out_of_range : public std::out_of_range, public backtrace {
    public:
        explicit out_of_range(std::string const &s) : std::out_of_range(s)
        {
        }
    };

    /// \cond INTERNAL

    namespace details {
        class trace_manip {
        public:
            trace_manip(backtrace const *tr) :
                tr_(tr)
            {
            }
            std::ostream &write(std::ostream &out) const
            {
                if(tr_)
                    tr_->trace(out);
                return out;
            }
        private:
            backtrace const *tr_;
        };

        inline std::ostream &operator<<(std::ostream &out,details::trace_manip const &t)
        {
            return t.write(out);
        }
    }

    /// \endcond

    ///
    /// \brief manipulator that print stack trace for the exception \a e if it is derived from backtrace.
    ///
    /// For example:
    /// \code
    /// catch(std::exception const &e) {
    ///   std::cerr << e.what() << std::endl;
    ///   std::cerr << booster::trace(e);
    /// }
    ///
    template<typename E>
    details::trace_manip trace(E const &e)
    {
        backtrace const *tr = dynamic_cast<backtrace const *>(&e);
        return details::trace_manip(tr);
    }


} // booster

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

