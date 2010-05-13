//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <cppcms/locale_formatting.h>
#include <cppcms/locale_format.h>
#include <cppcms/locale_generator.h>
#include "locale_src_test_locale.hpp"
#include "locale_src_test_locale_tools.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cppcms/encoding.h>

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
    
    if(loc.name()=="*" || loc.name()=="C")
        TEST_FP1(as::number,1200.1,"1200.1",double,1200.1);
    else
        TEST_FP1(as::number,1200.1,"1,200.1",double,1200.1);
    if(loc.name()!="*" && loc.name()!="C") {
        TEST_FMT(as::currency,10,"$10.00");
        TEST_FMT(as::currency << as::currency_national,10,"$10.00");
        TEST_FMT(as::currency << as::currency_iso,10,"USD  10.00");
    }

    time_t a_date = 3600*24*(31+4); // Feb 5th
    time_t a_time = 3600*15+60*33; // 15:33:05
    time_t a_timesec = 13;
    time_t a_datetime = a_date + a_time + a_timesec;

    std::time_put<char> const &put = std::use_facet<std::time_put<char> >(loc);
    std::ostringstream tmp;
    tmp.imbue(loc);
    put.put(tmp,tmp,' ',gmtime(&a_datetime),'x');
    TEST_FMT(as::date << as::gmt,a_datetime,tmp.str());
    tmp.str("");
    put.put(tmp,tmp,' ',gmtime(&a_datetime),'X');
    TEST_FMT(as::time << as::gmt,a_datetime,tmp.str());
    tmp.str("");
    put.put(tmp,tmp,' ',gmtime(&a_datetime),'c');
    TEST_FMT(as::datetime << as::gmt,a_datetime,tmp.str());
    tmp.str("");
    
    time_t now=time(0);
    char local_time_str[256];
    std::string format="%H:%M:%S";
    std::basic_string<CharType> format_string(format.begin(),format.end());
    strftime(local_time_str,sizeof(local_time_str),format.c_str(),localtime(&now));
    TEST_FMT(as::ftime(format_string),now,local_time_str);
    TEST_FMT(as::ftime(format_string)<<as::gmt<<as::local_time,now,local_time_str);

    std::string sample_f[]={
        "Now is %H o'clo''ck ' or not ' ",
        "'test %H'",
        "%H'",
        "'%H'" 
    };
    std::string expected_f[] = {
        "Now is 15 o'clo''ck ' or not ' ",
        "'test 15'",
        "15'",
        "'15'"
    };

    for(unsigned i=0;i<sizeof(sample_f)/sizeof(sample_f[0]);i++) {
        format_string.assign(sample_f[i].begin(),sample_f[i].end());
        TEST_FMT(as::ftime(format_string)<<as::gmt,a_datetime,expected_f[i]);
    }

}

#define FORMAT2(fstr,params,manips)    \
do { std::ostringstream tmp; tmp.imbue(loc); tmp << manips; FORMAT(fstr,params,tmp.str()); } while(0)

template<typename CharType>
void test_format(std::string charset="UTF-8")
{
    cppcms::locale::generator g;
    std::locale loc=g("en_US."+charset);

    FORMAT("{3} {1} {2}", 1 % 2 % 3,"3 1 2");
    FORMAT("{1} {2}", "hello" % 2,"hello 2");
    FORMAT2("{1}",120.1, 120.1);
    FORMAT2("Test {1,num}",120.1,"Test " << 120.1);
    FORMAT2("{{}} {1,number}",120.1,"{} " << 120.1);
    FORMAT2("{1,num=sci,p=3}",13.1,std::scientific << std::setprecision(3) << 13.1);
    FORMAT2("{1,num=scientific,p=3}",13.1,std::scientific << std::setprecision(3) << 13.1);
    FORMAT2("{1,num=fix,p=3}",13.1,std::fixed << std::setprecision(3) << 13.1);
    FORMAT2("{1,num=fixed,p=3}",13.1,std::fixed << std::setprecision(3) << 13.1);
    FORMAT2("{1,<,w=3,num}",-1,"-1 ");
    FORMAT2("{1,>,w=3,num}",1,"  1");
    if(loc.name()!="*" && loc.name()!="C") {
        FORMAT("{1,cur}",124,"$124.00");
        FORMAT("{1,currency}",124,"$124.00");
        FORMAT("{1,cur=nat}",124,"$124.00");
        FORMAT("{1,cur=national}",124,"$124.00");
        FORMAT("{1,cur=iso}",124,"USD  124.00");
    }

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
    std::time_put<char> const &put = std::use_facet<std::time_put<char> >(loc);
    
    std::ostringstream tmp;
    tmp.imbue(loc);
    put.put(tmp,tmp,' ',gmtime(&a_datetime),'x');
    tmp << ';';
    put.put(tmp,tmp,' ',gmtime(&a_datetime),'X');
    tmp << ';';
    put.put(tmp,tmp,' ',gmtime(&a_datetime),'c');
    tmp << ';';
    put.put(tmp,tmp,' ',gmtime(&a_datetime),'H');


    FORMAT("{1,date,gmt};{1,time,gmt};{1,datetime,gmt};{1,ftime='%H',gmt}",a_datetime,tmp.str());
    FORMAT("{1,ftime='%I',tz=GMT+01:00}",a_datetime,"04");
    FORMAT("{1,ftime='%I',tz=GMT-01:00}",a_datetime,"02");

    FORMAT("{1,gmt,ftime='%H'''}",a_datetime,"15'");
    FORMAT("{1,gmt,ftime='''%H'}",a_datetime,"'15");
    FORMAT("{1,gmt,ftime='%H o''clock'}",a_datetime,"15 o'clock");
}


void test_workaround()
{
    std::cout << "Testing UTF-8 workaround" << std::endl;
    std::locale ru;
    try {
        ru=std::locale("ru_RU.UTF-8");
    }
    catch(std::exception const &e)
    {
        std::cout << "Do not have ru_RU.UTF-8 locale, nothing to test" << std::endl;
    }
    std::ostringstream ss;
    ss.imbue(ru);
    ss<<13456.5;
    std::string tmp=ss.str();
    TEST(tmp.at(0)=='1');
    size_t count=0;
    if(cppcms::encoding::valid_utf8(tmp.c_str(),tmp.c_str()+tmp.size(),count)) {
        std::cout << "  Not needed" << std::endl;
    }
    ss.str("");
    ss.imbue(generator().get("ru_RU.UTF-8"));
    ss << 13456.5 << " " << as::currency << 12345.6;
    tmp=ss.str();
    TEST(tmp.at(0)=='1');
    TEST(cppcms::encoding::valid_utf8(tmp.c_str(),tmp.c_str()+tmp.size(),count));
}

int main()
{
    try {
        std::cout << "Testing UTF-8" << std::endl;
        test_manip<char>();
        test_format<char>();
        std::cout << "Testing ISO-8859-1" << std::endl;
        test_manip<char>("iso88591");
        test_format<char>("iso88591");
        test_workaround();
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
