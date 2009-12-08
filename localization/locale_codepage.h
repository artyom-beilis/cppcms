//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_CODEPAGE_HPP_INCLUDED
#define CPPCMS_LOCALE_CODEPAGE_HPP_INCLUDED

#include "defs.h"
#include "config.h"
#include "locale_info.h"
#include "cstdint.h"

namespace cppcms {
    namespace locale {
        namespace details {
            class CPPCMS_API converter {
            public:

                static const uint32_t illegal=0xFFFFFFFF;
                static const uint32_t incomplete=0xFFFFFFFE;
                
                // encoding
                converter(std::string const &encoding);
                ~converter();

                int max_len()
                {
                    if(is_utf8_)
                        return 4;
                    else
                        return max_len_;
                }

                uint32_t to_unicode(char const *&begin,char const *end)
                {
                    if(is_utf8_)
                        return from_utf8(begin,end);
                    return from_charset(begin,end);
                }

                uint32_t from_unicode(uint32_t u,char *begin,char const *end)
                {
                    if(is_utf8_)
                        return to_utf8(u,begin,end);
                    return to_charset(u,begin,end);
                }
            private:

                //
                // Non Copyable
                //
                converter(converter const &);
                void operator=(converter const &);

                uint32_t from_charset(char const *&begin,char const *end);
                uint32_t to_charset(uint32_t u,char *begin,char const *end);

                uint32_t to_utf8(uint32_t u,char *begin,char const *end)
                {
                    if(u>0x10ffff)
                        return illegal;
                    if(0xd800 <=u && u<= 0xdfff) // surrogates
                        return illegal;
                    ptrdiff_t d=end-begin;
                    if(u <=0x7F) { 
                        if(d>=1) {
                            *begin++=u;
                            return 1;
                        }
                        else
                            return incomplete;
                    }
                    else if(u <= 0x7FF) {
                        if(d>=2) {
                            *begin++=(u >> 6) | 0xC0;
                            *begin++=(u & 0x3F) | 0x80;
                            return 2;
                        }
                        else
                            return incomplete;
                    }
                    else if(u <= 0xFFFF) {
                        if(d>=3) {
                            *begin++=(u >> 12) | 0xE0;
                            *begin++=((u >> 6) & 0x3F) | 0x80;
                            *begin++=(u & 0x3F) | 0x80;
                            return 3;
                        }
                        else
                            return incomplete;
                    }
                    else {
                        if(d>=4) {
                            *begin++=(u >> 18) | 0xF0;
                            *begin++=((u >> 12) & 0x3F) | 0x80;
                            *begin++=((u >> 6) & 0x3F) | 0x80;
                            *begin++=(u & 0x3F) | 0x80;
                            return 4;
                        }
                        else
                            return incomplete;
                    }
                }
                uint32_t from_utf8(char const *&begin,char const *end)
                {
                    unsigned char const *p=reinterpret_cast<unsigned char const *>(begin);
                    unsigned char const *e=reinterpret_cast<unsigned char const *>(end);
                    if(p==e)
                        return incomplete;
                    unsigned char c=*p++;
                    unsigned char seq0,seq1=0,seq2=0,seq3=0;
                    seq0=c;
                    int len=1;
                    if((c & 0xC0) == 0xC0) {
                        if(p==e)
                            return incomplete;
                        seq1=*p++;
                        len=2;
                    }
                    if((c & 0xE0) == 0xE0) {
                        if(p==e)
                            return incomplete;
                        seq2=*p++;
                        len=3;
                    }
                    if((c & 0xF0) == 0xF0) {
                        if(p==e)
                            return incomplete;
                        seq3=*p++;
                        len=4;
                    }
                    switch(len) {
                    case 1:
                        break;
                    case 2: // non-overloading 2 bytes
                        if( 0xC2 <= seq0 && seq0 <= 0xDF
                            && 0x80 <= seq1 && seq1<= 0xBF)
                        {
                                break;
                        }
                        return illegal;
                    case 3: 
                        if(seq0==0xE0) { // exclude overloading
                            if(0xA0 <=seq1 && seq1<= 0xBF && 0x80 <=seq2 && seq2<=0xBF)
                                break;
                        }
                        else if( (0xE1 <= seq0 && seq0 <=0xEC) || seq0==0xEE || seq0==0xEF) { // stright 3 bytes
                            if(0x80 <=seq1 && seq1<=0xBF &&
                               0x80 <=seq2 && seq2<=0xBF)
                                break;
                        }
                        else if(seq0 == 0xED) { // exclude surrogates
                            if( 0x80 <=seq1 && seq1<=0x9F &&
                                0x80 <=seq2 && seq2<=0xBF)
                                break;
                        }
                        return illegal;
                    case 4:
                        switch(seq0) {
                        case 0xF0: // planes 1-3
                            if( 0x90 <=seq1 && seq1<=0xBF &&
                                0x80 <=seq2 && seq2<=0xBF &&
                                0x80 <=seq3 && seq3<=0xBF)
                                break;
                            return illegal;
                        case 0xF1: // planes 4-15
                        case 0xF2:
                        case 0xF3:
                            if( 0x80 <=seq1 && seq1<=0xBF &&
                                0x80 <=seq2 && seq2<=0xBF &&
                                0x80 <=seq3 && seq3<=0xBF)
                                break;
                            return illegal;
                        case 0xF4: // pane 16
                            if( 0x80 <=seq1 && seq1<=0x8F &&
                                0x80 <=seq2 && seq2<=0xBF &&
                                0x80 <=seq3 && seq3<=0xBF)
                                break;
                            return illegal;
                        default:
                            return illegal;
                        }
                    }
                    begin=reinterpret_cast<char const *>(p);
                    switch(len) {
                    case 1:
                        return seq0;
                    case 2:
                        return ((seq0 & 0x1F) << 6) | (seq1 & 0x3F);
                    case 3:
                        return ((seq0 & 0x0F) << 12) | ((seq1 & 0x3F) << 6) | (seq2 & 0x3F)  ;
					default: // can be only 4
                        return ((seq0 & 0x07) << 18) | ((seq1 & 0x3F) << 12) | ((seq2 & 0x3F) << 6) | (seq3 & 0x3F) ;
                    }
                }

                // Private data members
                bool is_utf8_;
                int max_len_;
                struct data;
                std::auto_ptr<data> d;
            };

            template<typename CharType,int n=sizeof(CharType)>
            struct uchar_traits;

            template<typename CharType>
            struct uchar_traits<CharType,2> {
                typedef uint16_t uint_type;
            };
            template<typename CharType>
            struct uchar_traits<CharType,4> {
                typedef uint32_t uint_type;
            };
        } // details


        ///
        /// \brief this class reimplements standard C++ codecvt facet. It is rarely used directly however it can be
        /// useful for code page conversions
        ///
        template<typename CharType>
        class code_converter : public std::codecvt<CharType,char,mbstate_t> 
        {
        public:
            static std::codecvt<CharType,char,mbstate_t> *create(info const &inf)
            {
                return new code_converter<CharType>(inf);
            }
        protected:
            typedef CharType uchar;
            code_converter(info const &inf,size_t refs=0) : 
                std::codecvt<CharType,char,mbstate_t>(refs),
                encoding_(inf.encoding())
            {
                details::converter cvt(encoding_);
                max_len_ = cvt.max_len(); 
            }
            virtual std::codecvt_base::result do_unshift(std::mbstate_t &s,char *from,char *to,char *&next) const
            {
                next=from;
                return std::codecvt_base::ok;
            }
            virtual int do_encoding() const throw()
            {
                // Always assume variable length
                return 0;
            }
            virtual int do_max_length() const throw()
            {
                return max_len_;
            }
            virtual bool do_always_noconv() const throw()
            {
                return false;
            }
            
            virtual std::codecvt_base::result 
            do_in(  std::mbstate_t &state,
                    char const *from,
                    char const *from_end,
                    char const *&from_next,
                    uchar *uto,
                    uchar *uto_end,
                    uchar *&uto_next) const
            {
                typedef typename details::uchar_traits<uchar>::uint_type uint_type;
                uint_type *to=reinterpret_cast<uint_type *>(uto);
                uint_type *to_end=reinterpret_cast<uint_type *>(uto_end);
                uint_type *&to_next=reinterpret_cast<uint_type *&>(uto_next);
                return do_real_in(state,from,from_end,from_next,to,to_end,to_next);
            }

            virtual std::codecvt_base::result 
            do_out( std::mbstate_t &state,
                    uchar const *ufrom,
                    uchar const *ufrom_end,
                    uchar const *&ufrom_next,
                    char *to,
                    char *to_end,
                    char *&to_next) const
            {
                typedef typename details::uchar_traits<uchar>::uint_type uint_type;
                uint_type const *from=reinterpret_cast<uint_type const *>(ufrom);
                uint_type const *from_end=reinterpret_cast<uint_type const *>(ufrom_end);
                uint_type const *&from_next=reinterpret_cast<uint_type const *&>(ufrom_next);
                return do_real_out(state,from,from_end,from_next,to,to_end,to_next);
            }
           

        private:

            //
            // Implementation for UTF-32
            //
            std::codecvt_base::result
            do_real_in( std::mbstate_t &state,
                        char const *from,
                        char const *from_end,
                        char const *&from_next,
                        uint32_t *to,
                        uint32_t *to_end,
                        uint32_t *to_next) const
            {
                details::converter cvt(encoding_);
                std::codecvt_base::result r=std::codecvt_base::ok;
                while(to < to_end && from < from_end)
                {
                    uint32_t ch=cvt.to_unicode(from,from_end);
                    if(ch==details::converter::illegal) {
                        r=std::codecvt_base::error;
                        break;
                    }
                    if(ch==details::converter::incomplete) {
                        r=std::codecvt_base::partial;
                        break;
                    }
                    *to++=ch;
                }
                from_next=from;
                to_next=to;
                if(r!=std::codecvt_base::ok)
                    return r;
                if(from!=from_end)
                    return std::codecvt_base::partial;
                return r;
            }

            //
            // Implementation for UTF-32
            //
            std::codecvt_base::result
            do_real_out(std::mbstate_t &state,
                        uint32_t const *from,
                        uint32_t const *from_end,
                        uint32_t const *&from_next,
                        char *to,
                        char *to_end,
                        char *&to_next) const
            {
                details::converter cvt(encoding_);
                std::codecvt_base::result r=std::codecvt_base::ok;
                while(to < to_end && from < from_end)
                {
                    uint32_t len=cvt.from_unicode(*from,to,to_end);
                    if(len==details::converter::illegal) {
                        r=std::codecvt_base::error;
                        break;
                    }
                    if(len==details::converter::incomplete) {
                        r=std::codecvt_base::partial;
                        break;
                    }
                    from++;
                    to+=len;
                }
                from_next=from;
                to_next=to;
                if(r!=std::codecvt_base::ok)
                    return r;
                if(from!=from_end)
                    return std::codecvt_base::partial;
                return r;
            }

			//
			// Can't handle full UTF-16, only UCS-2
			// because:
			//   1. codecvt facet must be able to work on single
			//      internal character ie if  do_in(s,from,from_end,x,y,z,t) returns ok
			//      then do_in(s,from,from+1) should return ok according to the standard papars
			//   2. I have absolutly NO information about mbstat_t -- I can't even know if its 0 
			//      or it is somehow initialized. So I can't store any state information
			//      about suragate pairs... So it works only for UCS-2
			//
			
			
            //
            // Implementation for UTF-16
            //
            std::codecvt_base::result
            do_real_in( std::mbstate_t &state,
                        char const *from,
                        char const *from_end,
                        char const *&from_next,
                        uint16_t *to,
                        uint16_t *to_end,
                        uint16_t *to_next) const
            {
                details::converter cvt(encoding_);
                std::codecvt_base::result r=std::codecvt_base::ok;
                while(to < to_end && from < from_end)
                {
                    char const *save_from=from;
                    uint32_t ch=cvt.to_unicode(from,from_end);
                    if(ch==details::converter::illegal) {
                        r=std::codecvt_base::error;
                        break;
                    }
                    if(ch==details::converter::incomplete) {
                        r=std::codecvt_base::partial;
                        break;
                    }
                    if(ch <= 0xFFFF) {
                        *to++=ch;
					}
                    else { /// can't handle surrogates
						r=std::codecvt_base::error;
                        break;
                    }
                }
                from_next=from;
                to_next=to;
                if(r!=std::codecvt_base::ok)
                    return r;
                if(from!=from_end)
                    return std::codecvt_base::partial;
                return r;
            }

            //
            // Implementation for UTF-16
            //
            std::codecvt_base::result
            do_real_out(std::mbstate_t &state,
                        uint16_t const *from,
                        uint16_t const *from_end,
                        uint16_t const *&from_next,
                        char *to,
                        char *to_end,
                        char *&to_next) const
            {
                details::converter cvt(encoding_);
                std::codecvt_base::result r=std::codecvt_base::ok;
                while(to < to_end && from < from_end)
                {
                    uint32_t ch=*from;
                    if(0xD800 <= ch && ch<=0xDFFF) {
                        r=std::codecvt_base::error;
						// Can't handle surragates
                        break;
                    }
                            
                    uint32_t len=cvt.from_unicode(ch,to,to_end);
                    if(len==details::converter::illegal) {
                        r=std::codecvt_base::error;
                        break;
                    }
                    if(len==details::converter::incomplete) {
                        r=std::codecvt_base::partial;
                        break;
                    }
                    to+=len;
					from++;
                }
                from_next=from;
                to_next=to;
                if(r!=std::codecvt_base::ok)
                    return r;
                if(from!=from_end)
                    return std::codecvt_base::partial;
                return r;
            }
            
            int max_len_;
            std::string encoding_;

        };
        
        template<>
        class code_converter<char>
		{
		public:
			static std::codecvt<char,char,mbstate_t> *create(info const &inf)
			{
				return new std::codecvt<char,char,mbstate_t>();
			}
        };
    } // locale
} // boost


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

