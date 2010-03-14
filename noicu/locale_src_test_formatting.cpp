//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "locale_formatting.h"
#include "locale_format.h"
#include "locale_generator.h"
#include "locale_src_test_locale.hpp"
#include "locale_src_test_locale_tools.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace cppcms::locale;

//#define TEST_DEBUG

#ifdef TEST_DEBUG
#define CPPCMS_NO_STD_WSTRING
#undef CPPCMS_HAS_CHAR16_T
#undef CPPCMS_HAS_CHAR32_T
#define TESTEQ(x,y) do { std::cerr << "["<<x << "]!=\n[" << y <<"]"<< std::endl; TEST((x)==(y)); } while(0)
#else
#define TESTEQ(x,y) TEST((x)==(y))
#endif

#define TEST_FMT(manip,value,expected) \
do{ \
    std::basic_ostringstream<CharType> ss; \
    ss.imbue(loc); \
    ss << manip << value; \
    TESTEQ(ss.str(),to_correct_string<CharType>(expected,loc)); \
}while(0)

#define TEST_NOPAR(manip,actual,type)                           \
do{                                                             \
    type v;                                                     \
    std::basic_string<CharType> act=                            \
        to_correct_string<CharType>(actual,loc);                \
    {                                                           \
        std::basic_istringstream<CharType> ss;                  \
        ss.imbue(loc);                                          \
        ss.str(act);                                            \
        ss >> manip >> v ;                                      \
        TEST(ss.fail());                                        \
    }                                                           \
    {                                                           \
        std::basic_istringstream<CharType> ss;                  \
        ss.imbue(loc);                                          \
        ss.str(act);                                            \
        ss.exceptions(std::ios_base::failbit);                  \
        ss >> manip;                                            \
        TEST_THROWS(ss >> v,std::ios_base::failure);            \
    }                                                           \
}while(0)
 
#define TEST_PAR(manip,type,actual,expected) \
do{ \
    type v; \
    {std::basic_istringstream<CharType> ss; \
    ss.imbue(loc); \
    ss.str(to_correct_string<CharType>(actual,loc)); \
    ss >> manip >> v >> std::ws; \
    TESTEQ(v,expected); \
    TEST(ss.eof()); }\
    {std::basic_istringstream<CharType> ss; \
    ss.imbue(loc); \
    ss.str(to_correct_string<CharType>(std::string(actual)+"@",loc)); \
    CharType tmp_c; \
    ss >> manip >> v >> std::skipws >> tmp_c; \
    TESTEQ(v,expected); \
    TEST(tmp_c=='@'); } \
}while(0)

#define TEST_FP1(manip,value_in,str,type,value_out) \
do { \
    TEST_FMT(manip,value_in,str); \
    TEST_PAR(manip,type,str,value_out); \
}while(0)

#define TEST_FP2(m1,m2,value_in,str,type,value_out) \
do { \
    TEST_FMT(m1<<m2,value_in,str); \
    TEST_PAR(m1>>m2,type,str,value_out);  \
}while(0)

#define TEST_FP3(m1,m2,m3,value_in,str,type,value_out) \
do { \
    TEST_FMT(m1<<m2<<m3,value_in,str); \
    TEST_PAR(m1>>m2>>m3,type,str,value_out); \
}while(0)

#define TEST_FP4(m1,m2,m3,m4,value_in,str,type,value_out) \
do { \
    TEST_FMT(m1<<m2<<m3<<m4,value_in,str); \
    TEST_PAR(m1>>m2>>m3>>m4,type,str,value_out); \
}while(0)


#define FORMAT(f,v,exp) \
    do{\
        std::basic_ostringstream<CharType> ss; \
        ss.imbue(loc);  \
        std::basic_string<CharType> fmt = to_correct_string<CharType>(f,loc); \
        ss << cppcms::locale::basic_format<CharType>(fmt) % v; \
        TESTEQ(ss.str(),to_correct_string<CharType>(exp,loc)); \
        ss.str(to_correct_string<CharType>("",loc)); \
        ss << cppcms::locale::basic_format<CharType>(cppcms::locale::translate(f)) % v; \
        TESTEQ(ss.str(),to_correct_string<CharType>(exp,loc)); \
    } while(0)


template<typename CharType>
void test_manip(std::string e_charset="UTF-8")
{
    cppcms::locale::generator g;
    std::locale loc=g("en_US."+e_charset);
    
    TEST_FP1(as::posix,1200.1,"1200.1",double,1200.1);
    TEST_FMT(as::currency,10,"$10.00");
    TEST_FMT(as::currency<< as::currency_national,10,"$10.00");
    TEST_FMT(as::currency<< as::currency_iso,10,"USD10.34");

    time_t a_date = 3600*24*(31+4); // Feb 5th
    time_t a_time = 3600*15+60*33; // 15:33:05
    time_t a_timesec = 13;
    time_t a_datetime = a_date + a_time + a_timesec;

    TEST_FMT(as::date << as::gmt,a_datetime,"February 5, 1970");
    TEST_FMT(as::time << as::gmt,a_datetime,"3:33:13 PM");
    TEST_FMT(as::datetime << as::gmt,a_datetime,"February 5, 1970 3:33:13 PM");
    
    time_t now=time(0);
    char local_time_str[256];
    std::string format="%H:%M:%S";
    std::basic_string<CharType> format_string(format.begin(),format.end());
    strftime(local_time_str,sizeof(local_time_str),format.c_str(),localtime(&now));
    TEST_FMT(as::ftime(format_string),now,local_time_str);
    TEST_FMT(as::ftime(format_string)<<as::gmt<<as::local_time,now,local_time_str);

    std::string sample_f[]={
        "Now is %A, %H o'clo''ck ' or not ' ",
        "'test %H'",
        "%H'",
        "'%H'" 
    };
    std::string expected_f[] = {
        "Now is Thursday, 15 o'clo''ck ' or not ' ",
        "'test 15'",
        "15'",
        "'15'"
    };

    for(unsigned i=0;i<sizeof(sample_f)/sizeof(sample_f[0]);i++) {
        format_string.assign(sample_f[i].begin(),sample_f[i].end());
        TEST_FMT(as::ftime(format_string)<<as::gmt,a_datetime,expected_f[i]);
    }

}

template<typename CharType>
void test_format(std::string charset="UTF-8")
{
    cppcms::locale::generator g;
    std::locale loc=g("en_US."+charset);
    
    FORMAT("{3} {1} {2}", 1 % 2 % 3,"3 1 2");
    FORMAT("{1} {2}", "hello" % 2,"hello 2");
    FORMAT("{1}",1200.1,"1200.1");
    FORMAT("Test {1,num}",1200.1,"Test 1,200.1");
    FORMAT("{{}} {1,number}",1200.1,"{} 1,200.1");
    FORMAT("{1,num=sci,p=3}",13.1,"1.310E1");
    FORMAT("{1,num=scientific,p=3}",13.1,"1.310E1");
    FORMAT("{1,num=fix,p=3}",13.1,"13.100");
    FORMAT("{1,num=fixed,p=3}",13.1,"13.100");
    FORMAT("{1,<,w=3,num}",-1,"-1 ");
    FORMAT("{1,>,w=3,num}",1,"  1");
    FORMAT("{per,1}",0.1,"10%");
    FORMAT("{percent,1}",0.1,"10%");
    FORMAT("{1,cur}",1234,"$1,234.00");
    FORMAT("{1,currency}",1234,"$1,234.00");
    FORMAT("{1,cur=nat}",1234,"$1,234.00");
    FORMAT("{1,cur=national}",1234,"$1,234.00");
    FORMAT("{1,cur=iso}",1234,"USD1,234.00");

    time_t now=time(0);
    char local_time_str[256];
    std::string format="'%H:%M:%S'";
    std::basic_string<CharType> format_string(format.begin(),format.end());
    strftime(local_time_str,sizeof(local_time_str),format.c_str(),localtime(&now));

    FORMAT("{1,ftime='''%H:%M:%S'''}",now,local_time_str);
    FORMAT("{1,local,ftime='''%H:%M:%S'''}",now,local_time_str);
    FORMAT("{1,ftime='''%H:%M:%S'''}",now,local_time_str);

    time_t a_date = 3600*24*(31+4); // Feb 5th
    time_t a_time = 3600*15+60*33; // 15:33:05
    time_t a_timesec = 13;
    time_t a_datetime = a_date + a_time + a_timesec;
    FORMAT("{1,date,gmt};{1,time,gmt};{1,datetime,gmt};{1,dt,gmt}",a_datetime,
            "Feb 5, 1970;3:33:13 PM;Feb 5, 1970 3:33:13 PM;Feb 5, 1970 3:33:13 PM");
    FORMAT("{1,time=short,gmt};{1,time=medium,gmt};{1,time=long,gmt};{1,date=full,gmt}",a_datetime,
            "3:33 PM;3:33:13 PM;3:33:13 PM GMT+00:00;Thursday, February 5, 1970");
    FORMAT("{1,time=s,gmt};{1,time=m,gmt};{1,time=l,gmt};{1,date=f,gmt}",a_datetime,
            "3:33 PM;3:33:13 PM;3:33:13 PM GMT+00:00;Thursday, February 5, 1970");
    FORMAT("{1,time=s,tz=GMT+01:00}",a_datetime,"4:33 PM");
    FORMAT("{1,time=s,timezone=GMT+01:00}",a_datetime,"4:33 PM");

    FORMAT("{1,gmt,ftime='%H'''}",a_datetime,"15'");
    FORMAT("{1,gmt,ftime='''%H'}",a_datetime,"'15");
    FORMAT("{1,gmt,ftime='%H o''clock'}",a_datetime,"15 o'clock");
}


int main()
{
    try {
        std::cout << "Testing char, UTF-8" << std::endl;
        test_manip<char>();
        test_format<char>();
        std::cout << "Testing char, ISO-8859-1" << std::endl;
        test_manip<char>("ISO-8859-1");
        test_format<char>("ISO-8859-1");

    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
