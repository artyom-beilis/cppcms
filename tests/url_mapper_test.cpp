///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_response.h>
#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/json.h>
#include <cppcms/cppcms_error.h>
#include <booster/locale/info.h>
#include <sstream>
#include "test.h"
#include "dummy_api.h"
#include "dummy_http_context.h"


struct point {
	int x,y;
};

std::istream &operator>>(std::istream &in,point &d)
{
	char c;
	in >> d.x >> c >> d.y;
	if(c!=',')
		in.setstate(std::ios::failbit);
	return in;
}

namespace stuff {
struct foo { 
	int l;
};

bool parse_url_parameter(cppcms::util::const_char_istream &parameter,foo &f)
{
	f.l = parameter.end() - parameter.begin();
	return true;
}

} // stuff
std::string value(std::ostringstream &s)
{
	std::string res = s.str();
	s.str("");
	return res;
}

class disp : public cppcms::application {
public:
	void hg(booster::cmatch const &m)
	{ v_ = "m:" + std::string(m[0]) +':'+std::string(m[1]); }
	void h0() { v_ = "-"; }
	void h1(std::string s1)	
	{ v_ = ':'+s1; }
	void h2(std::string s1,std::string s2)
	{ v_ = ':'+s1+':'+s2; }
	void h3(std::string s1,std::string s2,std::string s3)
	{ v_ = ':'+s1+':'+s2+':'+s3; }
	void h4(std::string s1,std::string s2,std::string s3,std::string s4)
	{ v_ = ':'+s1+':'+s2+':'+s3+':'+s4; }
	void h5(std::string s1,std::string s2,std::string s3,std::string s4,std::string s5)
	{ v_ = ':'+s1+':'+s2+':'+s3+':'+s4+':'+s5; }
	void h6(std::string s1,std::string s2,std::string s3,std::string s4,std::string s5,std::string s6)
	{ v_ = ':'+s1+':'+s2+':'+s3+':'+s4+':'+s5+':'+s6; }


	template<typename T>
	static std::string to_string(T v)
	{
		std::ostringstream ss;
		ss<<v;
		return "to_str(" + ss.str() + ")";
	}
	template<typename T>
	static std::string ts(T v)
	{
		std::ostringstream ss;
		ss<<':'<<v;
		return ss.str();
	}

	void h1_i(int param)
	{
		v_= "h1_i:"+to_string(param);
	}
	void h1_d(double param)
	{
		v_= "h1_d:"+to_string(param);
	}
	void h1_s(std::string param)
	{
		v_= "h1_s:"+param;
	}
	void h1_s_cr(std::string const &param)
	{
		v_= "h1_s_cr:"+param;
	}
	void h1_s_r(std::string &param)
	{
		v_= "h1_s_r:"+param;
	}
	void h1_s_c(std::string const param)
	{
		v_= "h1_s_c:"+param;
	}

	void get_0() { v_="get_0"; }
	void get_1(int a) { v_="get_1" + ts(a); }
	void get_2(int a,int b) { v_="get_2" + ts(a)+ts(b); }
	void get_3(int a,int b,int c) { v_="get_3" + ts(a)+ts(b)+ts(c); }
	void get_4(int a,int b,int c,int d) { v_="get_4" + ts(a)+ts(b)+ts(c)+ts(d); }
	void get_5(int a,int b,int c,int d,int e) { v_="get_5" + ts(a)+ts(b)+ts(c)+ts(d)+ts(e); }
	void get_6(int a,int b,int c,int d,int e,int f) { v_="get_6" + ts(a)+ts(b)+ts(c)+ts(d)+ts(e)+ts(f); }
	void get_7(int a,int b,int c,int d,int e,int f,int g) { v_="get_7" + ts(a)+ts(b)+ts(c)+ts(d)+ts(e)+ts(f)+ts(g); }
	void get_8(int a,int b,int c,int d,int e,int f,int g,int h) { v_="get_8" + ts(a)+ts(b)+ts(c)+ts(d)+ts(e)+ts(f)+ts(g)+ts(h); }
	
	void p_0() { v_="p_0"; }
	void p_1(int a) { v_="p_1" + ts(a); }
	void p_2(int a,int b) { v_="p_2" + ts(a)+ts(b); }
	void p_3(int a,int b,int c) { v_="p_3" + ts(a)+ts(b)+ts(c); }
	void p_4(int a,int b,int c,int d) { v_="p_4" + ts(a)+ts(b)+ts(c)+ts(d); }
	void p_5(int a,int b,int c,int d,int e) { v_="p_5" + ts(a)+ts(b)+ts(c)+ts(d)+ts(e); }
	void p_6(int a,int b,int c,int d,int e,int f) { v_="p_6" + ts(a)+ts(b)+ts(c)+ts(d)+ts(e)+ts(f); }
	void p_7(int a,int b,int c,int d,int e,int f,int g) { v_="p_7" + ts(a)+ts(b)+ts(c)+ts(d)+ts(e)+ts(f)+ts(g); }
	void p_8(int a,int b,int c,int d,int e,int f,int g,int h) { v_="p_8" + ts(a)+ts(b)+ts(c)+ts(d)+ts(e)+ts(f)+ts(g)+ts(h); }

	void point_m(point const &p) { v_="point:" + to_string(p.x) + "X" + to_string(p.y); }
	void foo_m(stuff::foo const &p) { v_="foo:" + to_string(p.l); }



	disp(cppcms::service &s) : cppcms::application(s) 
	{
		dispatcher().assign_generic("hg/(.*)",&disp::hg,this);
		dispatcher().assign("h0",&disp::h0,this);
		dispatcher().assign("h1/(a(\\d+))",&disp::h1,this,2);
		dispatcher().assign("h2/(a(\\d+))/(a(\\d+))",&disp::h2,this,2,4);
		dispatcher().assign("h3/(a(\\d+))/(a(\\d+))/(a(\\d+))",&disp::h3,this,2,4,6);
		dispatcher().assign("h4/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",&disp::h4,this,2,4,6,8);
		dispatcher().assign("h5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",&disp::h5,this,2,4,6,8,10);
		dispatcher().assign("h6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",&disp::h6,this,2,4,6,8,10,12);
		
		dispatcher().map("/h0",&disp::h0,this);
		dispatcher().map(booster::regex("/rh0",booster::regex::icase),&disp::h0,this);
		dispatcher().map("/h1_num/(.*)",&disp::h1_i,this,1);
		dispatcher().map("/h1_num/(.*)",&disp::h1_d,this,1);

		dispatcher().map("/h1_s/(\\d+)",&disp::h1_s,this,1);
		dispatcher().map(booster::regex("/rh1_s/(\\d+)",booster::regex::icase),&disp::h1_s,this,1);
		dispatcher().map("/h1_s_cr/((\\d+))",&disp::h1_s_cr,this,2);
		dispatcher().map("/h1_s_r/(\\d+)",&disp::h1_s_r,this,1);
		dispatcher().map("/h1_s_c/(\\d+)",&disp::h1_s_c,this,1);

		dispatcher().map("/name/(.*)",&disp::h1_s,this,1);

		dispatcher().map("/point/(.*)",&disp::point_m,this,1);
		dispatcher().map("/foo/(.*)",&disp::foo_m,this,1);
		
		dispatcher().map("GET",
						 "/res_1",
						 &disp::get_0,this);
		dispatcher().map("(PUT|POST)",
						 "/res_1",
						 &disp::p_0,this);
		dispatcher().map("GET",
						 "/res_1/(a(\\d+))",
						 &disp::get_1,this,2);
		dispatcher().map("(PUT|POST)",
						 "/res_1/(a(\\d+))",
						 &disp::p_1,this,2);
		dispatcher().map("GET",
						 "/res_1/(a(\\d+))/(a(\\d+))",
						 &disp::get_2,this,2,4);
		dispatcher().map("(PUT|POST)",
						 "/res_1/(a(\\d+))/(a(\\d+))",
						 &disp::p_2,this,2,4);
		dispatcher().map("GET",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_3,this,2,4,6);
		dispatcher().map("(PUT|POST)",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_3,this,2,4,6);
		dispatcher().map("GET",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_4,this,2,4,6,8);
		dispatcher().map("(PUT|POST)",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_4,this,2,4,6,8);
		dispatcher().map("GET",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_5,this,2,4,6,8,10);
		dispatcher().map("(PUT|POST)",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_5,this,2,4,6,8,10);
		dispatcher().map("GET",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_6,this,2,4,6,8,10,12);
		dispatcher().map("(PUT|POST)",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_6,this,2,4,6,8,10,12);
		dispatcher().map("GET",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_7,this,2,4,6,8,10,12,14);
		dispatcher().map("(PUT|POST)",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_7,this,2,4,6,8,10,12,14);
		dispatcher().map("GET",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_8,this,2,4,6,8,10,12,14,16);
		dispatcher().map("(PUT|POST)",
						 "/res_1/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_8,this,2,4,6,8,10,12,14,16);

		dispatcher().map("/res_2",
						 &disp::get_0,this);
		dispatcher().map("/res_2/(a(\\d+))",
						 &disp::get_1,this,2);
		dispatcher().map("/res_2/(a(\\d+))/(a(\\d+))",
						 &disp::get_2,this,2,4);
		dispatcher().map("/res_2/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_3,this,2,4,6);
		dispatcher().map("/res_2/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_4,this,2,4,6,8);
		dispatcher().map("/res_2/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_5,this,2,4,6,8,10);
		dispatcher().map("/res_2/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_6,this,2,4,6,8,10,12);
		dispatcher().map("/res_2/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_7,this,2,4,6,8,10,12,14);
		dispatcher().map("/res_2/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_8,this,2,4,6,8,10,12,14,16);

		dispatcher().map("GET",
						 booster::regex("/RES_3",booster::regex::icase),
						 &disp::get_0,this);
		dispatcher().map("(PUT|POST)",
						 booster::regex("/RES_3",booster::regex::icase),
						 &disp::p_0,this);
		dispatcher().map("GET",
						 booster::regex("/RES_3/(a(\\d+))",booster::regex::icase),
						 &disp::get_1,this,2);
		dispatcher().map("(PUT|POST)",
						 booster::regex("/RES_3/(a(\\d+))",booster::regex::icase),
						 &disp::p_1,this,2);
		dispatcher().map("GET",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_2,this,2,4);
		dispatcher().map("(PUT|POST)",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_2,this,2,4);
		dispatcher().map("GET",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_3,this,2,4,6);
		dispatcher().map("(PUT|POST)",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_3,this,2,4,6);
		dispatcher().map("GET",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_4,this,2,4,6,8);
		dispatcher().map("(PUT|POST)",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_4,this,2,4,6,8);
		dispatcher().map("GET",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_5,this,2,4,6,8,10);
		dispatcher().map("(PUT|POST)",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_5,this,2,4,6,8,10);
		dispatcher().map("GET",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_6,this,2,4,6,8,10,12);
		dispatcher().map("(PUT|POST)",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_6,this,2,4,6,8,10,12);
		dispatcher().map("GET",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
		  				 &disp::get_7,this,2,4,6,8,10,12,14);
		dispatcher().map("(PUT|POST)",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
		  				 &disp::p_7,this,2,4,6,8,10,12,14);
		dispatcher().map("GET",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_8,this,2,4,6,8,10,12,14,16);
		dispatcher().map("(PUT|POST)",
						 booster::regex("/RES_3/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_8,this,2,4,6,8,10,12,14,16);

		dispatcher().map(booster::regex("/RES_4",booster::regex::icase),
						 &disp::get_0,this);
		dispatcher().map(booster::regex("/RES_4/(a(\\d+))",booster::regex::icase),
						 &disp::get_1,this,2);
		dispatcher().map(booster::regex("/RES_4/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_2,this,2,4);
		dispatcher().map(booster::regex("/RES_4/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_3,this,2,4,6);
		dispatcher().map(booster::regex("/RES_4/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_4,this,2,4,6,8);
		dispatcher().map(booster::regex("/RES_4/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_5,this,2,4,6,8,10);
		dispatcher().map(booster::regex("/RES_4/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_6,this,2,4,6,8,10,12);
		dispatcher().map(booster::regex("/RES_4/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_7,this,2,4,6,8,10,12,14);
		dispatcher().map(booster::regex("/RES_4/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_8,this,2,4,6,8,10,12,14,16);
		
		dispatcher().map("GET",
						 "text/html",
						 "/res_5",
						 &disp::get_0,this);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "/res_5",
						 &disp::p_0,this);
		dispatcher().map("GET",
						 "text/html",
						 "/res_5/(a(\\d+))",
						 &disp::get_1,this,2);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "/res_5/(a(\\d+))",
						 &disp::p_1,this,2);
		dispatcher().map("GET",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))",
						 &disp::get_2,this,2,4);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))",
						 &disp::p_2,this,2,4);
		dispatcher().map("GET",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_3,this,2,4,6);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_3,this,2,4,6);
		dispatcher().map("GET",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_4,this,2,4,6,8);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_4,this,2,4,6,8);
		dispatcher().map("GET",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_5,this,2,4,6,8,10);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_5,this,2,4,6,8,10);
		dispatcher().map("GET",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_6,this,2,4,6,8,10,12);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_6,this,2,4,6,8,10,12);
		dispatcher().map("GET",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_7,this,2,4,6,8,10,12,14);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_7,this,2,4,6,8,10,12,14);
		dispatcher().map("GET",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::get_8,this,2,4,6,8,10,12,14,16);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "/res_5/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_8,this,2,4,6,8,10,12,14,16);
		
		dispatcher().map("GET",
						 "text/html",
						 booster::regex("/RES_6",booster::regex::icase),
						 &disp::get_0,this);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 booster::regex("/RES_6",booster::regex::icase),
						 &disp::p_0,this);
		dispatcher().map("GET",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))",booster::regex::icase),
						 &disp::get_1,this,2);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))",booster::regex::icase),
						 &disp::p_1,this,2);
		dispatcher().map("GET",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_2,this,2,4);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_2,this,2,4);
		dispatcher().map("GET",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_3,this,2,4,6);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_3,this,2,4,6);
		dispatcher().map("GET",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_4,this,2,4,6,8);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_4,this,2,4,6,8);
		dispatcher().map("GET",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_5,this,2,4,6,8,10);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_5,this,2,4,6,8,10);
		dispatcher().map("GET",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_6,this,2,4,6,8,10,12);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_6,this,2,4,6,8,10,12);
		dispatcher().map("GET",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::get_7,this,2,4,6,8,10,12,14);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_7,this,2,4,6,8,10,12,14);
		dispatcher().map("GET",
						 "text/html",
		  				 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
		  				 &disp::get_8,this,2,4,6,8,10,12,14,16);
		dispatcher().map("(PUT|POST)",
						 "text/html",
		  				 booster::regex("/RES_6/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
		  				 &disp::p_8,this,2,4,6,8,10,12,14,16);
		
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 "/res_7",
						 &disp::p_0,this);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 "/res_7/(a(\\d+))",
						 &disp::p_1,this,2);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 "/res_7/(a(\\d+))/(a(\\d+))",
						 &disp::p_2,this,2,4);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 "/res_7/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_3,this,2,4,6);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 "/res_7/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_4,this,2,4,6,8);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 "/res_7/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_5,this,2,4,6,8,10);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 "/res_7/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_6,this,2,4,6,8,10,12);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 "/res_7/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_7,this,2,4,6,8,10,12,14);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 "/res_7/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_7,this,2,4,6,8,10,12,14);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 "/res_7/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",
						 &disp::p_8,this,2,4,6,8,10,12,14,16);
		
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 booster::regex("/RES_8",booster::regex::icase),
						 &disp::p_0,this);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 booster::regex("/RES_8/(a(\\d+))",booster::regex::icase),
						 &disp::p_1,this,2);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 booster::regex("/RES_8/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_2,this,2,4);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 booster::regex("/RES_8/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_3,this,2,4,6);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 booster::regex("/RES_8/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_4,this,2,4,6,8);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 booster::regex("/RES_8/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_5,this,2,4,6,8,10);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 booster::regex("/RES_8/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_6,this,2,4,6,8,10,12);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
						 booster::regex("/RES_8/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
						 &disp::p_7,this,2,4,6,8,10,12,14);
		dispatcher().map("(PUT|POST)",
						 "text/html",
						 "application/x-www-form-urlencoded",
		  				 booster::regex("/RES_8/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))/(a(\\d+))",booster::regex::icase),
		  				 &disp::p_8,this,2,4,6,8,10,12,14,16);
	}
#define TESTD(x,y) do { main(x); TEST(v_==y); } while(0)

#define TESTM(m,x,y) do { v_="none"; set_context((m)); main((x)); release_context() ; if(v_!=(y))  std::cerr << "v=" <<v_ << " exp="<< (y) <<std::endl; TEST(v_==(y)); } while(0)

#define TESTMA(m,a,x,y) do { v_="none"; set_context((m),(a)); main((x)); release_context() ; if(v_!=(y))  std::cerr << "v=" <<v_ << " exp="<< (y) <<std::endl; TEST(v_==(y)); } while(0)

#define TESTMACT(m,a,ct,x,y) do { v_="none"; set_context((m),(a),(ct)); main((x)); release_context() ; if(v_!=(y))  std::cerr << "v=" <<v_ << " exp="<< (y) <<std::endl; TEST(v_==(y)); } while(0)

	void test()
	{
		TESTD("hg/x","m:hg/x:x");
		TESTD("h0","-");
		TESTD("h1/a1",":1");
		TESTD("h2/a1/a2",":1:2");
		TESTD("h3/a1/a2/a3",":1:2:3");
		TESTD("h4/a1/a2/a3/a4",":1:2:3:4");
		TESTD("h5/a1/a2/a3/a4/a5",":1:2:3:4:5");
		TESTD("h6/a1/a2/a3/a4/a5/a6",":1:2:3:4:5:6");

		TESTM("GET","/h0","-");
		TESTM("GET","/rh0","-");
		TESTM("GET","/RH0","-");
		TESTM("GET","/h1_num/123.3","h1_d:to_str(123.3)");
		TESTM("GET","/h1_num/123","h1_i:to_str(123)");
		TESTM("GET","/h1_num/123 23","none");
		TESTM("GET","/h1_s/1111","h1_s:1111");
		TESTM("GET","/RH1_S/1111","h1_s:1111");
		TESTM("GET","/h1_s_c/1111","h1_s_c:1111");
		TESTM("GET","/h1_s_r/1111","h1_s_r:1111");
		TESTM("GET","/h1_s_cr/1111","h1_s_cr:1111");
		TESTM("GET","/h1_s_cr/1111\xFF","none");
		TESTM("GET","/h1_s_cr/1111\xFF","none");
		
		TESTM("GET","/name/\xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D\x20\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E",
			   "h1_s:\xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D\x20\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E");
		TESTM("GET","/name/\xFF","none");

		TESTM("GET","/point/123,23","point:to_str(123)Xto_str(23)");
		TESTM("GET","/point/123","none");
		TESTM("GET","/point/123,23.2","none");

		TESTM("GET","/foo/abcd","foo:to_str(4)");
		TESTM("GET","/foo/abcdefg","foo:to_str(7)");
		
		TESTM("GET","/res_1","get_0");
		TESTMA("GET","application/json","/res_1","get_0");
		TESTMACT("GET","application/json","application/json; utf-8","/res_1","get_0");
		TESTM("PUT","/res_1","p_0");
		TESTMA("PUT","application/json","/res_1","p_0");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_1","p_0");
		TESTM("POST","/res_1","p_0");
		TESTMA("POST","application/json","/res_1","p_0");
		TESTMACT("POST","application/json","application/json; utf-8","/res_1","p_0");
		TESTM("DELETE","/res_1","none");
		TESTMA("DELETE","application/json","/res_1","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_1","none");
		TESTM("GET","/res_1/a1","get_1:1");
		TESTMA("GET","application/json","/res_1/a1","get_1:1");
		TESTMACT("GET","application/json","application/json; utf-8","/res_1/a1","get_1:1");
		TESTM("PUT","/res_1/a1","p_1:1");
		TESTMA("PUT","application/json","/res_1/a1","p_1:1");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_1/a1","p_1:1");
		TESTM("POST","/res_1/a1","p_1:1");
		TESTMA("POST","application/json","/res_1/a1","p_1:1");
		TESTMACT("POST","application/json","application/json; utf-8","/res_1/a1","p_1:1");
		TESTM("DELETE","/res_1/a1","none");
		TESTMA("DELETE","application/json","/res_1/a1","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_1/a1","none");
		TESTM("GET","/res_1/a1/a2","get_2:1:2");
		TESTMA("GET","application/json","/res_1/a1/a2","get_2:1:2");
		TESTMACT("GET","application/json","application/json; utf-8","/res_1/a1/a2","get_2:1:2");
		TESTM("PUT","/res_1/a1/a2","p_2:1:2");
		TESTMA("PUT","application/json","/res_1/a1/a2","p_2:1:2");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_1/a1/a2","p_2:1:2");
		TESTM("POST","/res_1/a1/a2","p_2:1:2");
		TESTMA("POST","application/json","/res_1/a1/a2","p_2:1:2");
		TESTMACT("POST","application/json","application/json; utf-8","/res_1/a1/a2","p_2:1:2");
		TESTM("DELETE","/res_1/a1/a2","none");
		TESTMA("DELETE","application/json","/res_1/a1/a2","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_1/a1/a2","none");
		TESTM("GET","/res_1/a1/a2/a3","get_3:1:2:3");
		TESTMA("GET","application/json","/res_1/a1/a2/a3","get_3:1:2:3");
		TESTMACT("GET","application/json","application/json; utf-8","/res_1/a1/a2/a3","get_3:1:2:3");
		TESTM("PUT","/res_1/a1/a2/a3","p_3:1:2:3");
		TESTMA("PUT","application/json","/res_1/a1/a2/a3","p_3:1:2:3");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_1/a1/a2/a3","p_3:1:2:3");
		TESTM("POST","/res_1/a1/a2/a3","p_3:1:2:3");
		TESTMA("POST","application/json","/res_1/a1/a2/a3","p_3:1:2:3");
		TESTMACT("POST","application/json","application/json; utf-8","/res_1/a1/a2/a3","p_3:1:2:3");
		TESTM("DELETE","/res_1/a1/a2/a3","none");
		TESTMA("DELETE","application/json","/res_1/a1/a2/a3","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_1/a1/a2/a3","none");
		TESTM("GET","/res_1/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("GET","application/json","/res_1/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMACT("GET","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("PUT","/res_1/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("PUT","application/json","/res_1/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTM("POST","/res_1/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("POST","application/json","/res_1/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMACT("POST","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTM("DELETE","/res_1/a1/a2/a3/a4","none");
		TESTMA("DELETE","application/json","/res_1/a1/a2/a3/a4","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4","none");
		TESTM("GET","/res_1/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("GET","application/json","/res_1/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMACT("GET","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("PUT","/res_1/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("PUT","application/json","/res_1/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTM("POST","/res_1/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("POST","application/json","/res_1/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMACT("POST","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTM("DELETE","/res_1/a1/a2/a3/a4/a5","none");
		TESTMA("DELETE","application/json","/res_1/a1/a2/a3/a4/a5","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5","none");
		TESTM("GET","/res_1/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("GET","application/json","/res_1/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMACT("GET","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("PUT","/res_1/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("PUT","application/json","/res_1/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTM("POST","/res_1/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("POST","application/json","/res_1/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMACT("POST","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTM("DELETE","/res_1/a1/a2/a3/a4/a5/a6","none");
		TESTMA("DELETE","application/json","/res_1/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6","none");
		TESTM("GET","/res_1/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("GET","application/json","/res_1/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMACT("GET","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("PUT","/res_1/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("PUT","application/json","/res_1/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTM("POST","/res_1/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("POST","application/json","/res_1/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMACT("POST","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTM("DELETE","/res_1/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("DELETE","application/json","/res_1/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("GET","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMA("GET","application/json","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMACT("GET","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTM("PUT","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("PUT","application/json","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTM("POST","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("POST","application/json","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMACT("POST","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTM("DELETE","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("DELETE","application/json","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_1/a1/a2/a3/a4/a5/a6/a7/a8","none");

		TESTM("GET","/res_2","get_0");
		TESTMA("GET","application/json","/res_2","get_0");
		TESTMACT("GET","application/json","application/json; utf-8","/res_2","get_0");
		TESTM("PUT","/res_2","get_0");
		TESTMA("PUT","application/json","/res_2","get_0");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_2","get_0");
		TESTM("POST","/res_2","get_0");
		TESTMA("POST","application/json","/res_2","get_0");
		TESTMACT("POST","application/json","application/json; utf-8","/res_2","get_0");
		TESTM("DELETE","/res_2","get_0");
		TESTMA("DELETE","application/json","/res_2","get_0");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_2","get_0");
		TESTM("GET","/res_2/a1","get_1:1");
		TESTMA("GET","application/json","/res_2/a1","get_1:1");
		TESTMACT("GET","application/json","application/json; utf-8","/res_2/a1","get_1:1");
		TESTM("PUT","/res_2/a1","get_1:1");
		TESTMA("PUT","application/json","/res_2/a1","get_1:1");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_2/a1","get_1:1");
		TESTM("POST","/res_2/a1","get_1:1");
		TESTMA("POST","application/json","/res_2/a1","get_1:1");
		TESTMACT("POST","application/json","application/json; utf-8","/res_2/a1","get_1:1");
		TESTM("DELETE","/res_2/a1","get_1:1");
		TESTMA("DELETE","application/json","/res_2/a1","get_1:1");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_2/a1","get_1:1");
		TESTM("GET","/res_2/a1/a2","get_2:1:2");
		TESTMA("GET","application/json","/res_2/a1/a2","get_2:1:2");
		TESTMACT("GET","application/json","application/json; utf-8","/res_2/a1/a2","get_2:1:2");
		TESTM("PUT","/res_2/a1/a2","get_2:1:2");
		TESTMA("PUT","application/json","/res_2/a1/a2","get_2:1:2");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_2/a1/a2","get_2:1:2");
		TESTM("POST","/res_2/a1/a2","get_2:1:2");
		TESTMA("POST","application/json","/res_2/a1/a2","get_2:1:2");
		TESTMACT("POST","application/json","application/json; utf-8","/res_2/a1/a2","get_2:1:2");
		TESTM("DELETE","/res_2/a1/a2","get_2:1:2");
		TESTMA("DELETE","application/json","/res_2/a1/a2","get_2:1:2");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_2/a1/a2","get_2:1:2");
		TESTM("GET","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTMA("GET","application/json","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTMACT("GET","application/json","application/json; utf-8","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTM("PUT","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTMA("PUT","application/json","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTM("POST","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTMA("POST","application/json","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTMACT("POST","application/json","application/json; utf-8","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTM("DELETE","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTMA("DELETE","application/json","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_2/a1/a2/a3","get_3:1:2:3");
		TESTM("GET","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("GET","application/json","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMACT("GET","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("PUT","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("PUT","application/json","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("POST","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("POST","application/json","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMACT("POST","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("DELETE","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("DELETE","application/json","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("GET","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("GET","application/json","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMACT("GET","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("PUT","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("PUT","application/json","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("POST","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("POST","application/json","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMACT("POST","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("DELETE","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("DELETE","application/json","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("PUT","/res_2/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("PUT","application/json","/res_2/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("POST","/res_2/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("POST","application/json","/res_2/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMACT("POST","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("DELETE","/res_2/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("DELETE","application/json","/res_2/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("GET","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("GET","application/json","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMACT("GET","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("PUT","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("PUT","application/json","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("POST","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("POST","application/json","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMACT("POST","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("DELETE","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("DELETE","application/json","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_2/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");

		TESTM("GET","/res_3","get_0");
		TESTMA("GET","application/json","/res_3","get_0");
		TESTMACT("GET","application/json","application/json; utf-8","/res_3","get_0");
		TESTM("PUT","/res_3","p_0");
		TESTMA("PUT","application/json","/res_3","p_0");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_3","p_0");
		TESTM("POST","/res_3","p_0");
		TESTMA("POST","application/json","/res_3","p_0");
		TESTMACT("POST","application/json","application/json; utf-8","/res_3","p_0");
		TESTM("DELETE","/res_3","none");
		TESTMA("DELETE","application/json","/res_3","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_3","none");
		TESTM("GET","/res_3/a1","get_1:1");
		TESTMA("GET","application/json","/res_3/a1","get_1:1");
		TESTMACT("GET","application/json","application/json; utf-8","/res_3/a1","get_1:1");
		TESTM("PUT","/res_3/a1","p_1:1");
		TESTMA("PUT","application/json","/res_3/a1","p_1:1");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_3/a1","p_1:1");
		TESTM("POST","/res_3/a1","p_1:1");
		TESTMA("POST","application/json","/res_3/a1","p_1:1");
		TESTMACT("POST","application/json","application/json; utf-8","/res_3/a1","p_1:1");
		TESTM("DELETE","/res_3/a1","none");
		TESTMA("DELETE","application/json","/res_3/a1","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_3/a1","none");
		TESTM("GET","/res_3/a1/a2","get_2:1:2");
		TESTMA("GET","application/json","/res_3/a1/a2","get_2:1:2");
		TESTMACT("GET","application/json","application/json; utf-8","/res_3/a1/a2","get_2:1:2");
		TESTM("PUT","/res_3/a1/a2","p_2:1:2");
		TESTMA("PUT","application/json","/res_3/a1/a2","p_2:1:2");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_3/a1/a2","p_2:1:2");
		TESTM("POST","/res_3/a1/a2","p_2:1:2");
		TESTMA("POST","application/json","/res_3/a1/a2","p_2:1:2");
		TESTMACT("POST","application/json","application/json; utf-8","/res_3/a1/a2","p_2:1:2");
		TESTM("DELETE","/res_3/a1/a2","none");
		TESTMA("DELETE","application/json","/res_3/a1/a2","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_3/a1/a2","none");
		TESTM("GET","/res_3/a1/a2/a3","get_3:1:2:3");
		TESTMA("GET","application/json","/res_3/a1/a2/a3","get_3:1:2:3");
		TESTMACT("GET","application/json","application/json; utf-8","/res_3/a1/a2/a3","get_3:1:2:3");
		TESTM("PUT","/res_3/a1/a2/a3","p_3:1:2:3");
		TESTMA("PUT","application/json","/res_3/a1/a2/a3","p_3:1:2:3");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_3/a1/a2/a3","p_3:1:2:3");
		TESTM("POST","/res_3/a1/a2/a3","p_3:1:2:3");
		TESTMA("POST","application/json","/res_3/a1/a2/a3","p_3:1:2:3");
		TESTMACT("POST","application/json","application/json; utf-8","/res_3/a1/a2/a3","p_3:1:2:3");
		TESTM("DELETE","/res_3/a1/a2/a3","none");
		TESTMA("DELETE","application/json","/res_3/a1/a2/a3","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_3/a1/a2/a3","none");
		TESTM("GET","/res_3/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("GET","application/json","/res_3/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMACT("GET","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("PUT","/res_3/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("PUT","application/json","/res_3/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTM("POST","/res_3/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("POST","application/json","/res_3/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMACT("POST","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTM("DELETE","/res_3/a1/a2/a3/a4","none");
		TESTMA("DELETE","application/json","/res_3/a1/a2/a3/a4","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4","none");
		TESTM("GET","/res_3/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("GET","application/json","/res_3/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMACT("GET","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("PUT","/res_3/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("PUT","application/json","/res_3/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTM("POST","/res_3/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("POST","application/json","/res_3/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMACT("POST","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTM("DELETE","/res_3/a1/a2/a3/a4/a5","none");
		TESTMA("DELETE","application/json","/res_3/a1/a2/a3/a4/a5","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5","none");
		TESTM("GET","/res_3/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("GET","application/json","/res_3/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMACT("GET","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("PUT","/res_3/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("PUT","application/json","/res_3/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTM("POST","/res_3/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("POST","application/json","/res_3/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMACT("POST","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTM("DELETE","/res_3/a1/a2/a3/a4/a5/a6","none");
		TESTMA("DELETE","application/json","/res_3/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6","none");
		TESTM("GET","/res_3/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("GET","application/json","/res_3/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMACT("GET","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("PUT","/res_3/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("PUT","application/json","/res_3/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTM("POST","/res_3/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("POST","application/json","/res_3/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMACT("POST","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTM("DELETE","/res_3/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("DELETE","application/json","/res_3/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("GET","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMA("GET","application/json","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMACT("GET","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTM("PUT","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("PUT","application/json","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTM("POST","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("POST","application/json","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMACT("POST","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTM("DELETE","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("DELETE","application/json","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_3/a1/a2/a3/a4/a5/a6/a7/a8","none");

		TESTM("GET","/res_4","get_0");
		TESTMA("GET","application/json","/res_4","get_0");
		TESTMACT("GET","application/json","application/json; utf-8","/res_4","get_0");
		TESTM("PUT","/res_4","get_0");
		TESTMA("PUT","application/json","/res_4","get_0");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_4","get_0");
		TESTM("POST","/res_4","get_0");
		TESTMA("POST","application/json","/res_4","get_0");
		TESTMACT("POST","application/json","application/json; utf-8","/res_4","get_0");
		TESTM("DELETE","/res_4","get_0");
		TESTMA("DELETE","application/json","/res_4","get_0");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_4","get_0");
		TESTM("GET","/res_4/a1","get_1:1");
		TESTMA("GET","application/json","/res_4/a1","get_1:1");
		TESTMACT("GET","application/json","application/json; utf-8","/res_4/a1","get_1:1");
		TESTM("PUT","/res_4/a1","get_1:1");
		TESTMA("PUT","application/json","/res_4/a1","get_1:1");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_4/a1","get_1:1");
		TESTM("POST","/res_4/a1","get_1:1");
		TESTMA("POST","application/json","/res_4/a1","get_1:1");
		TESTMACT("POST","application/json","application/json; utf-8","/res_4/a1","get_1:1");
		TESTM("DELETE","/res_4/a1","get_1:1");
		TESTMA("DELETE","application/json","/res_4/a1","get_1:1");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_4/a1","get_1:1");
		TESTM("GET","/res_4/a1/a2","get_2:1:2");
		TESTMA("GET","application/json","/res_4/a1/a2","get_2:1:2");
		TESTMACT("GET","application/json","application/json; utf-8","/res_4/a1/a2","get_2:1:2");
		TESTM("PUT","/res_4/a1/a2","get_2:1:2");
		TESTMA("PUT","application/json","/res_4/a1/a2","get_2:1:2");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_4/a1/a2","get_2:1:2");
		TESTM("POST","/res_4/a1/a2","get_2:1:2");
		TESTMA("POST","application/json","/res_4/a1/a2","get_2:1:2");
		TESTMACT("POST","application/json","application/json; utf-8","/res_4/a1/a2","get_2:1:2");
		TESTM("DELETE","/res_4/a1/a2","get_2:1:2");
		TESTMA("DELETE","application/json","/res_4/a1/a2","get_2:1:2");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_4/a1/a2","get_2:1:2");
		TESTM("GET","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTMA("GET","application/json","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTMACT("GET","application/json","application/json; utf-8","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTM("PUT","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTMA("PUT","application/json","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTM("POST","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTMA("POST","application/json","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTMACT("POST","application/json","application/json; utf-8","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTM("DELETE","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTMA("DELETE","application/json","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_4/a1/a2/a3","get_3:1:2:3");
		TESTM("GET","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("GET","application/json","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMACT("GET","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("PUT","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("PUT","application/json","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("POST","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("POST","application/json","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMACT("POST","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("DELETE","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("DELETE","application/json","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("GET","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("GET","application/json","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMACT("GET","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("PUT","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("PUT","application/json","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("POST","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("POST","application/json","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMACT("POST","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("DELETE","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("DELETE","application/json","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("GET","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("GET","application/json","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMACT("GET","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("PUT","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("PUT","application/json","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("POST","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("POST","application/json","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMACT("POST","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("DELETE","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("DELETE","application/json","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("GET","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("GET","application/json","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMACT("GET","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("PUT","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("PUT","application/json","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("POST","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("POST","application/json","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMACT("POST","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("DELETE","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("DELETE","application/json","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("GET","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMA("GET","application/json","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMACT("GET","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTM("PUT","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMA("PUT","application/json","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMACT("PUT","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTM("POST","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMA("POST","application/json","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMACT("POST","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTM("DELETE","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMA("DELETE","application/json","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMACT("DELETE","application/json","application/json; utf-8","/res_4/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");

		TESTM("GET","/res_5","get_0");
		TESTMA("GET","text/html,text/xhtml","/res_5","get_0");
		TESTMA("GET","application/json","/res_5","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5","get_0");
		TESTM("PUT","/res_5","p_0");
		TESTMA("PUT","text/html,text/xhtml","/res_5","p_0");
		TESTMA("PUT","application/json","/res_5","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5","p_0");
		TESTM("POST","/res_5","p_0");
		TESTMA("POST","text/html,text/xhtml","/res_5","p_0");
		TESTMA("POST","application/json","/res_5","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5","p_0");
		TESTM("DELETE","/res_5","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_5","none");
		TESTMA("DELETE","application/json","/res_5","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5","none");
		TESTM("GET","/res_5/a1","get_1:1");
		TESTMA("GET","text/html,text/xhtml","/res_5/a1","get_1:1");
		TESTMA("GET","application/json","/res_5/a1","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1","get_1:1");
		TESTM("PUT","/res_5/a1","p_1:1");
		TESTMA("PUT","text/html,text/xhtml","/res_5/a1","p_1:1");
		TESTMA("PUT","application/json","/res_5/a1","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1","p_1:1");
		TESTM("POST","/res_5/a1","p_1:1");
		TESTMA("POST","text/html,text/xhtml","/res_5/a1","p_1:1");
		TESTMA("POST","application/json","/res_5/a1","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1","p_1:1");
		TESTM("DELETE","/res_5/a1","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_5/a1","none");
		TESTMA("DELETE","application/json","/res_5/a1","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1","none");
		TESTM("GET","/res_5/a1/a2","get_2:1:2");
		TESTMA("GET","text/html,text/xhtml","/res_5/a1/a2","get_2:1:2");
		TESTMA("GET","application/json","/res_5/a1/a2","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2","get_2:1:2");
		TESTM("PUT","/res_5/a1/a2","p_2:1:2");
		TESTMA("PUT","text/html,text/xhtml","/res_5/a1/a2","p_2:1:2");
		TESTMA("PUT","application/json","/res_5/a1/a2","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2","p_2:1:2");
		TESTM("POST","/res_5/a1/a2","p_2:1:2");
		TESTMA("POST","text/html,text/xhtml","/res_5/a1/a2","p_2:1:2");
		TESTMA("POST","application/json","/res_5/a1/a2","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2","p_2:1:2");
		TESTM("DELETE","/res_5/a1/a2","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_5/a1/a2","none");
		TESTMA("DELETE","application/json","/res_5/a1/a2","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2","none");
		TESTM("GET","/res_5/a1/a2/a3","get_3:1:2:3");
		TESTMA("GET","text/html,text/xhtml","/res_5/a1/a2/a3","get_3:1:2:3");
		TESTMA("GET","application/json","/res_5/a1/a2/a3","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3","get_3:1:2:3");
		TESTM("PUT","/res_5/a1/a2/a3","p_3:1:2:3");
		TESTMA("PUT","text/html,text/xhtml","/res_5/a1/a2/a3","p_3:1:2:3");
		TESTMA("PUT","application/json","/res_5/a1/a2/a3","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3","p_3:1:2:3");
		TESTM("POST","/res_5/a1/a2/a3","p_3:1:2:3");
		TESTMA("POST","text/html,text/xhtml","/res_5/a1/a2/a3","p_3:1:2:3");
		TESTMA("POST","application/json","/res_5/a1/a2/a3","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3","p_3:1:2:3");
		TESTM("DELETE","/res_5/a1/a2/a3","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_5/a1/a2/a3","none");
		TESTMA("DELETE","application/json","/res_5/a1/a2/a3","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3","none");
		TESTM("GET","/res_5/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("GET","text/html,text/xhtml","/res_5/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("GET","application/json","/res_5/a1/a2/a3/a4","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("PUT","/res_5/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("PUT","text/html,text/xhtml","/res_5/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("PUT","application/json","/res_5/a1/a2/a3/a4","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTM("POST","/res_5/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("POST","text/html,text/xhtml","/res_5/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("POST","application/json","/res_5/a1/a2/a3/a4","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTM("DELETE","/res_5/a1/a2/a3/a4","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_5/a1/a2/a3/a4","none");
		TESTMA("DELETE","application/json","/res_5/a1/a2/a3/a4","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4","none");
		TESTM("GET","/res_5/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("GET","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("GET","application/json","/res_5/a1/a2/a3/a4/a5","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("PUT","/res_5/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("PUT","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("PUT","application/json","/res_5/a1/a2/a3/a4/a5","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTM("POST","/res_5/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("POST","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("POST","application/json","/res_5/a1/a2/a3/a4/a5","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTM("DELETE","/res_5/a1/a2/a3/a4/a5","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5","none");
		TESTMA("DELETE","application/json","/res_5/a1/a2/a3/a4/a5","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5","none");
		TESTM("GET","/res_5/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("GET","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("GET","application/json","/res_5/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("PUT","/res_5/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("PUT","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("PUT","application/json","/res_5/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTM("POST","/res_5/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("POST","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("POST","application/json","/res_5/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTM("DELETE","/res_5/a1/a2/a3/a4/a5/a6","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6","none");
		TESTMA("DELETE","application/json","/res_5/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6","none");
		TESTM("GET","/res_5/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("GET","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("GET","application/json","/res_5/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("PUT","/res_5/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("PUT","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("PUT","application/json","/res_5/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTM("POST","/res_5/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("POST","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("POST","application/json","/res_5/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTM("DELETE","/res_5/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("DELETE","application/json","/res_5/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("GET","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMA("GET","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMA("GET","application/json","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTM("PUT","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("PUT","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("PUT","application/json","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTM("POST","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("POST","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("POST","application/json","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTM("DELETE","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("DELETE","application/json","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_5/a1/a2/a3/a4/a5/a6/a7/a8","none");

		TESTM("GET","/res_6","get_0");
		TESTMA("GET","text/xhtml,text/html","/res_6","get_0");
		TESTMA("GET","application/json","/res_6","none");
		TESTMACT("GET","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6","get_0");
		TESTM("PUT","/res_6","p_0");
		TESTMA("PUT","text/xhtml,text/html","/res_6","p_0");
		TESTMA("PUT","application/json","/res_6","none");
		TESTMACT("PUT","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6","p_0");
		TESTM("POST","/res_6","p_0");
		TESTMA("POST","text/xhtml,text/html","/res_6","p_0");
		TESTMA("POST","application/json","/res_6","none");
		TESTMACT("POST","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6","p_0");
		TESTM("DELETE","/res_6","none");
		TESTMA("DELETE","text/xhtml,text/html","/res_6","none");
		TESTMA("DELETE","application/json","/res_6","none");
		TESTMACT("DELETE","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6","none");
		TESTM("GET","/res_6/a1","get_1:1");
		TESTMA("GET","text/xhtml,text/html","/res_6/a1","get_1:1");
		TESTMA("GET","application/json","/res_6/a1","none");
		TESTMACT("GET","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1","get_1:1");
		TESTM("PUT","/res_6/a1","p_1:1");
		TESTMA("PUT","text/xhtml,text/html","/res_6/a1","p_1:1");
		TESTMA("PUT","application/json","/res_6/a1","none");
		TESTMACT("PUT","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1","p_1:1");
		TESTM("POST","/res_6/a1","p_1:1");
		TESTMA("POST","text/xhtml,text/html","/res_6/a1","p_1:1");
		TESTMA("POST","application/json","/res_6/a1","none");
		TESTMACT("POST","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1","p_1:1");
		TESTM("DELETE","/res_6/a1","none");
		TESTMA("DELETE","text/xhtml,text/html","/res_6/a1","none");
		TESTMA("DELETE","application/json","/res_6/a1","none");
		TESTMACT("DELETE","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1","none");
		TESTM("GET","/res_6/a1/a2","get_2:1:2");
		TESTMA("GET","text/xhtml,text/html","/res_6/a1/a2","get_2:1:2");
		TESTMA("GET","application/json","/res_6/a1/a2","none");
		TESTMACT("GET","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2","get_2:1:2");
		TESTM("PUT","/res_6/a1/a2","p_2:1:2");
		TESTMA("PUT","text/xhtml,text/html","/res_6/a1/a2","p_2:1:2");
		TESTMA("PUT","application/json","/res_6/a1/a2","none");
		TESTMACT("PUT","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2","p_2:1:2");
		TESTM("POST","/res_6/a1/a2","p_2:1:2");
		TESTMA("POST","text/xhtml,text/html","/res_6/a1/a2","p_2:1:2");
		TESTMA("POST","application/json","/res_6/a1/a2","none");
		TESTMACT("POST","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2","p_2:1:2");
		TESTM("DELETE","/res_6/a1/a2","none");
		TESTMA("DELETE","text/xhtml,text/html","/res_6/a1/a2","none");
		TESTMA("DELETE","application/json","/res_6/a1/a2","none");
		TESTMACT("DELETE","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2","none");
		TESTM("GET","/res_6/a1/a2/a3","get_3:1:2:3");
		TESTMA("GET","text/xhtml,text/html","/res_6/a1/a2/a3","get_3:1:2:3");
		TESTMA("GET","application/json","/res_6/a1/a2/a3","none");
		TESTMACT("GET","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3","get_3:1:2:3");
		TESTM("PUT","/res_6/a1/a2/a3","p_3:1:2:3");
		TESTMA("PUT","text/xhtml,text/html","/res_6/a1/a2/a3","p_3:1:2:3");
		TESTMA("PUT","application/json","/res_6/a1/a2/a3","none");
		TESTMACT("PUT","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3","p_3:1:2:3");
		TESTM("POST","/res_6/a1/a2/a3","p_3:1:2:3");
		TESTMA("POST","text/xhtml,text/html","/res_6/a1/a2/a3","p_3:1:2:3");
		TESTMA("POST","application/json","/res_6/a1/a2/a3","none");
		TESTMACT("POST","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3","p_3:1:2:3");
		TESTM("DELETE","/res_6/a1/a2/a3","none");
		TESTMA("DELETE","text/xhtml,text/html","/res_6/a1/a2/a3","none");
		TESTMA("DELETE","application/json","/res_6/a1/a2/a3","none");
		TESTMACT("DELETE","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3","none");
		TESTM("GET","/res_6/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("GET","text/xhtml,text/html","/res_6/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTMA("GET","application/json","/res_6/a1/a2/a3/a4","none");
		TESTMACT("GET","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4","get_4:1:2:3:4");
		TESTM("PUT","/res_6/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("PUT","text/xhtml,text/html","/res_6/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("PUT","application/json","/res_6/a1/a2/a3/a4","none");
		TESTMACT("PUT","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTM("POST","/res_6/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("POST","text/xhtml,text/html","/res_6/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMA("POST","application/json","/res_6/a1/a2/a3/a4","none");
		TESTMACT("POST","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTM("DELETE","/res_6/a1/a2/a3/a4","none");
		TESTMA("DELETE","text/xhtml,text/html","/res_6/a1/a2/a3/a4","none");
		TESTMA("DELETE","application/json","/res_6/a1/a2/a3/a4","none");
		TESTMACT("DELETE","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4","none");
		TESTM("GET","/res_6/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("GET","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTMA("GET","application/json","/res_6/a1/a2/a3/a4/a5","none");
		TESTMACT("GET","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5","get_5:1:2:3:4:5");
		TESTM("PUT","/res_6/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("PUT","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("PUT","application/json","/res_6/a1/a2/a3/a4/a5","none");
		TESTMACT("PUT","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTM("POST","/res_6/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("POST","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMA("POST","application/json","/res_6/a1/a2/a3/a4/a5","none");
		TESTMACT("POST","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTM("DELETE","/res_6/a1/a2/a3/a4/a5","none");
		TESTMA("DELETE","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5","none");
		TESTMA("DELETE","application/json","/res_6/a1/a2/a3/a4/a5","none");
		TESTMACT("DELETE","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5","none");
		TESTM("GET","/res_6/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("GET","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTMA("GET","application/json","/res_6/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("GET","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6","get_6:1:2:3:4:5:6");
		TESTM("PUT","/res_6/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("PUT","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("PUT","application/json","/res_6/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("PUT","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTM("POST","/res_6/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("POST","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMA("POST","application/json","/res_6/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("POST","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTM("DELETE","/res_6/a1/a2/a3/a4/a5/a6","none");
		TESTMA("DELETE","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6","none");
		TESTMA("DELETE","application/json","/res_6/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("DELETE","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6","none");
		TESTM("GET","/res_6/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("GET","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTMA("GET","application/json","/res_6/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("GET","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6/a7","get_7:1:2:3:4:5:6:7");
		TESTM("PUT","/res_6/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("PUT","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("PUT","application/json","/res_6/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("PUT","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTM("POST","/res_6/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("POST","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMA("POST","application/json","/res_6/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("POST","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTM("DELETE","/res_6/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("DELETE","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("DELETE","application/json","/res_6/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("DELETE","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("GET","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMA("GET","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTMA("GET","application/json","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("GET","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","get_8:1:2:3:4:5:6:7:8");
		TESTM("PUT","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("PUT","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("PUT","application/json","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("PUT","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTM("POST","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("POST","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMA("POST","application/json","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("POST","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTM("DELETE","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("DELETE","text/xhtml,text/html","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("DELETE","application/json","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("DELETE","text/xhtml,text/html","application/x-www-form-urlencoded; charset=utf-8","/res_6/a1/a2/a3/a4/a5/a6/a7/a8","none");

		TESTM("GET","/res_7","none");
		TESTMA("GET","text/html,text/xhtml","/res_7","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_7","none");
		TESTMA("GET","application/json","/res_7","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_7","none");
		TESTM("PUT","/res_7","none");
		TESTMA("PUT","text/html,text/xhtml","/res_7","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7","p_0");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_7","none");
		TESTMA("PUT","application/json","/res_7","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_7","none");
		TESTM("POST","/res_7","none");
		TESTMA("POST","text/html,text/xhtml","/res_7","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7","p_0");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_7","none");
		TESTMA("POST","application/json","/res_7","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_7","none");
		TESTM("DELETE","/res_7","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_7","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_7","none");
		TESTMA("DELETE","application/json","/res_7","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_7","none");
		TESTM("GET","/res_7/a1","none");
		TESTMA("GET","text/html,text/xhtml","/res_7/a1","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1","none");
		TESTMA("GET","application/json","/res_7/a1","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_7/a1","none");
		TESTM("PUT","/res_7/a1","none");
		TESTMA("PUT","text/html,text/xhtml","/res_7/a1","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1","p_1:1");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1","none");
		TESTMA("PUT","application/json","/res_7/a1","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_7/a1","none");
		TESTM("POST","/res_7/a1","none");
		TESTMA("POST","text/html,text/xhtml","/res_7/a1","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1","p_1:1");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1","none");
		TESTMA("POST","application/json","/res_7/a1","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_7/a1","none");
		TESTM("DELETE","/res_7/a1","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_7/a1","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1","none");
		TESTMA("DELETE","application/json","/res_7/a1","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_7/a1","none");
		TESTM("GET","/res_7/a1/a2","none");
		TESTMA("GET","text/html,text/xhtml","/res_7/a1/a2","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2","none");
		TESTMA("GET","application/json","/res_7/a1/a2","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_7/a1/a2","none");
		TESTM("PUT","/res_7/a1/a2","none");
		TESTMA("PUT","text/html,text/xhtml","/res_7/a1/a2","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2","p_2:1:2");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2","none");
		TESTMA("PUT","application/json","/res_7/a1/a2","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_7/a1/a2","none");
		TESTM("POST","/res_7/a1/a2","none");
		TESTMA("POST","text/html,text/xhtml","/res_7/a1/a2","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2","p_2:1:2");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2","none");
		TESTMA("POST","application/json","/res_7/a1/a2","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_7/a1/a2","none");
		TESTM("DELETE","/res_7/a1/a2","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_7/a1/a2","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2","none");
		TESTMA("DELETE","application/json","/res_7/a1/a2","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_7/a1/a2","none");
		TESTM("GET","/res_7/a1/a2/a3","none");
		TESTMA("GET","text/html,text/xhtml","/res_7/a1/a2/a3","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3","none");
		TESTMA("GET","application/json","/res_7/a1/a2/a3","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3","none");
		TESTM("PUT","/res_7/a1/a2/a3","none");
		TESTMA("PUT","text/html,text/xhtml","/res_7/a1/a2/a3","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3","p_3:1:2:3");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3","none");
		TESTMA("PUT","application/json","/res_7/a1/a2/a3","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3","none");
		TESTM("POST","/res_7/a1/a2/a3","none");
		TESTMA("POST","text/html,text/xhtml","/res_7/a1/a2/a3","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3","p_3:1:2:3");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3","none");
		TESTMA("POST","application/json","/res_7/a1/a2/a3","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3","none");
		TESTM("DELETE","/res_7/a1/a2/a3","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_7/a1/a2/a3","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3","none");
		TESTMA("DELETE","application/json","/res_7/a1/a2/a3","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3","none");
		TESTM("GET","/res_7/a1/a2/a3/a4","none");
		TESTMA("GET","text/html,text/xhtml","/res_7/a1/a2/a3/a4","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4","none");
		TESTMA("GET","application/json","/res_7/a1/a2/a3/a4","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4","none");
		TESTM("PUT","/res_7/a1/a2/a3/a4","none");
		TESTMA("PUT","text/html,text/xhtml","/res_7/a1/a2/a3/a4","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4","none");
		TESTMA("PUT","application/json","/res_7/a1/a2/a3/a4","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4","none");
		TESTM("POST","/res_7/a1/a2/a3/a4","none");
		TESTMA("POST","text/html,text/xhtml","/res_7/a1/a2/a3/a4","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4","none");
		TESTMA("POST","application/json","/res_7/a1/a2/a3/a4","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4","none");
		TESTM("DELETE","/res_7/a1/a2/a3/a4","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_7/a1/a2/a3/a4","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4","none");
		TESTMA("DELETE","application/json","/res_7/a1/a2/a3/a4","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4","none");
		TESTM("GET","/res_7/a1/a2/a3/a4/a5","none");
		TESTMA("GET","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5","none");
		TESTMA("GET","application/json","/res_7/a1/a2/a3/a4/a5","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5","none");
		TESTM("PUT","/res_7/a1/a2/a3/a4/a5","none");
		TESTMA("PUT","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5","none");
		TESTMA("PUT","application/json","/res_7/a1/a2/a3/a4/a5","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5","none");
		TESTM("POST","/res_7/a1/a2/a3/a4/a5","none");
		TESTMA("POST","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5","none");
		TESTMA("POST","application/json","/res_7/a1/a2/a3/a4/a5","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5","none");
		TESTM("DELETE","/res_7/a1/a2/a3/a4/a5","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5","none");
		TESTMA("DELETE","application/json","/res_7/a1/a2/a3/a4/a5","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5","none");
		TESTM("GET","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMA("GET","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMA("GET","application/json","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTM("PUT","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMA("PUT","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMA("PUT","application/json","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTM("POST","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMA("POST","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMA("POST","application/json","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTM("DELETE","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMA("DELETE","application/json","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6","none");
		TESTM("GET","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("GET","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("GET","application/json","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("PUT","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("PUT","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("PUT","application/json","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("POST","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("POST","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("POST","application/json","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("DELETE","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("DELETE","application/json","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("GET","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("GET","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("GET","application/json","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTM("PUT","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("PUT","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("PUT","application/json","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTM("POST","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("POST","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("POST","application/json","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTM("DELETE","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("DELETE","application/json","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_7/a1/a2/a3/a4/a5/a6/a7/a8","none");

		TESTM("GET","/res_8","none");
		TESTMA("GET","text/html,text/xhtml","/res_8","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_8","none");
		TESTMA("GET","application/json","/res_8","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_8","none");
		TESTM("PUT","/res_8","none");
		TESTMA("PUT","text/html,text/xhtml","/res_8","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8","p_0");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_8","none");
		TESTMA("PUT","application/json","/res_8","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_8","none");
		TESTM("POST","/res_8","none");
		TESTMA("POST","text/html,text/xhtml","/res_8","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8","p_0");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_8","none");
		TESTMA("POST","application/json","/res_8","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_8","none");
		TESTM("DELETE","/res_8","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_8","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_8","none");
		TESTMA("DELETE","application/json","/res_8","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_8","none");
		TESTM("GET","/res_8/a1","none");
		TESTMA("GET","text/html,text/xhtml","/res_8/a1","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1","none");
		TESTMA("GET","application/json","/res_8/a1","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_8/a1","none");
		TESTM("PUT","/res_8/a1","none");
		TESTMA("PUT","text/html,text/xhtml","/res_8/a1","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1","p_1:1");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1","none");
		TESTMA("PUT","application/json","/res_8/a1","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_8/a1","none");
		TESTM("POST","/res_8/a1","none");
		TESTMA("POST","text/html,text/xhtml","/res_8/a1","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1","p_1:1");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1","none");
		TESTMA("POST","application/json","/res_8/a1","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_8/a1","none");
		TESTM("DELETE","/res_8/a1","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_8/a1","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1","none");
		TESTMA("DELETE","application/json","/res_8/a1","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_8/a1","none");
		TESTM("GET","/res_8/a1/a2","none");
		TESTMA("GET","text/html,text/xhtml","/res_8/a1/a2","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2","none");
		TESTMA("GET","application/json","/res_8/a1/a2","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_8/a1/a2","none");
		TESTM("PUT","/res_8/a1/a2","none");
		TESTMA("PUT","text/html,text/xhtml","/res_8/a1/a2","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2","p_2:1:2");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2","none");
		TESTMA("PUT","application/json","/res_8/a1/a2","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_8/a1/a2","none");
		TESTM("POST","/res_8/a1/a2","none");
		TESTMA("POST","text/html,text/xhtml","/res_8/a1/a2","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2","p_2:1:2");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2","none");
		TESTMA("POST","application/json","/res_8/a1/a2","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_8/a1/a2","none");
		TESTM("DELETE","/res_8/a1/a2","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_8/a1/a2","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2","none");
		TESTMA("DELETE","application/json","/res_8/a1/a2","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_8/a1/a2","none");
		TESTM("GET","/res_8/a1/a2/a3","none");
		TESTMA("GET","text/html,text/xhtml","/res_8/a1/a2/a3","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3","none");
		TESTMA("GET","application/json","/res_8/a1/a2/a3","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3","none");
		TESTM("PUT","/res_8/a1/a2/a3","none");
		TESTMA("PUT","text/html,text/xhtml","/res_8/a1/a2/a3","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3","p_3:1:2:3");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3","none");
		TESTMA("PUT","application/json","/res_8/a1/a2/a3","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3","none");
		TESTM("POST","/res_8/a1/a2/a3","none");
		TESTMA("POST","text/html,text/xhtml","/res_8/a1/a2/a3","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3","p_3:1:2:3");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3","none");
		TESTMA("POST","application/json","/res_8/a1/a2/a3","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3","none");
		TESTM("DELETE","/res_8/a1/a2/a3","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_8/a1/a2/a3","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3","none");
		TESTMA("DELETE","application/json","/res_8/a1/a2/a3","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3","none");
		TESTM("GET","/res_8/a1/a2/a3/a4","none");
		TESTMA("GET","text/html,text/xhtml","/res_8/a1/a2/a3/a4","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4","none");
		TESTMA("GET","application/json","/res_8/a1/a2/a3/a4","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4","none");
		TESTM("PUT","/res_8/a1/a2/a3/a4","none");
		TESTMA("PUT","text/html,text/xhtml","/res_8/a1/a2/a3/a4","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4","none");
		TESTMA("PUT","application/json","/res_8/a1/a2/a3/a4","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4","none");
		TESTM("POST","/res_8/a1/a2/a3/a4","none");
		TESTMA("POST","text/html,text/xhtml","/res_8/a1/a2/a3/a4","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4","p_4:1:2:3:4");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4","none");
		TESTMA("POST","application/json","/res_8/a1/a2/a3/a4","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4","none");
		TESTM("DELETE","/res_8/a1/a2/a3/a4","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_8/a1/a2/a3/a4","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4","none");
		TESTMA("DELETE","application/json","/res_8/a1/a2/a3/a4","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4","none");
		TESTM("GET","/res_8/a1/a2/a3/a4/a5","none");
		TESTMA("GET","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5","none");
		TESTMA("GET","application/json","/res_8/a1/a2/a3/a4/a5","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5","none");
		TESTM("PUT","/res_8/a1/a2/a3/a4/a5","none");
		TESTMA("PUT","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5","none");
		TESTMA("PUT","application/json","/res_8/a1/a2/a3/a4/a5","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5","none");
		TESTM("POST","/res_8/a1/a2/a3/a4/a5","none");
		TESTMA("POST","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5","p_5:1:2:3:4:5");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5","none");
		TESTMA("POST","application/json","/res_8/a1/a2/a3/a4/a5","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5","none");
		TESTM("DELETE","/res_8/a1/a2/a3/a4/a5","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5","none");
		TESTMA("DELETE","application/json","/res_8/a1/a2/a3/a4/a5","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5","none");
		TESTM("GET","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMA("GET","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMA("GET","application/json","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTM("PUT","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMA("PUT","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMA("PUT","application/json","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTM("POST","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMA("POST","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","p_6:1:2:3:4:5:6");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMA("POST","application/json","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTM("DELETE","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMA("DELETE","application/json","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6","none");
		TESTM("GET","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("GET","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("GET","application/json","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("PUT","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("PUT","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("PUT","application/json","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("POST","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("POST","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","p_7:1:2:3:4:5:6:7");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("POST","application/json","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("DELETE","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMA("DELETE","application/json","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7","none");
		TESTM("GET","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("GET","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("GET","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("GET","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("GET","application/json","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("GET","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTM("PUT","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("PUT","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("PUT","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMACT("PUT","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("PUT","application/json","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("PUT","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTM("POST","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("POST","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("POST","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","p_8:1:2:3:4:5:6:7:8");
		TESTMACT("POST","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("POST","application/json","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("POST","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTM("DELETE","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("DELETE","text/html,text/xhtml","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/x-www-form-urlencoded; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("DELETE","text/html,text/xhtml","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMA("DELETE","application/json","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
		TESTMACT("DELETE","application/json","application/json; charset=utf-8","/res_8/a1/a2/a3/a4/a5/a6/a7/a8","none");
	}
	void set_context(std::string const &method="GET", std::string const &accept="*/*", std::string const &content_type="")
	{
		std::map<std::string,std::string> env;
		env["HTTP_HOST"]="www.example.com";
		env["SCRIPT_NAME"]="/foo";
		env["PATH_INFO"]="/bar";
		env["REQUEST_METHOD"]=method;
		if(!accept.empty())
			env["HTTP_ACCEPT"]=accept;
		if(!content_type.empty())
			env["CONTENT_TYPE"]=content_type;
		env["HTTP_ACCEPT_ENCODING"]="gzip";
		booster::shared_ptr<dummy_api> api(new dummy_api(service(),env,output_));
		booster::shared_ptr<dummy_http_context> cnt(new dummy_http_context(api));
		assign_context(cnt);
		response().io_mode(cppcms::http::response::normal);
		output_.clear();
	}
	
	std::string v_;
	std::string output_;
};

void dispatcher_test(cppcms::service &srv)
{
	disp d(srv);
	d.test();
}

void basic_test(cppcms::service &srv,bool throws)
{
	cppcms::application app(srv);
	cppcms::url_mapper m(&app);

	std::cout << "-- Basic Mapping" << std::endl;

	m.assign("foo","/foo");
	m.assign("foo","/foo/{1}");
	m.assign("foo","/foo/{1}/{2}");
	m.assign("foo","/foo/{1}/{2}/{3}");
	m.assign("foo","/foo/{1}/{2}/{3}/{4}");
	m.assign("foo","/foo/{1}/{2}/{3}/{4}/{5}");
	m.assign("foo","/foo/{1}/{2}/{3}/{4}/{5}/{6}");
	m.assign("bar","/bar/{lang}/{1}");
	m.assign("bar","/bar/{2}/{1}");
	m.assign("test1","{1}x");
	m.assign("test2","x{1}");
	app.mapper().set_value("lang","en");
	m.root("test.com");
	TEST(m.root()=="test.com");

	std::ostringstream ss;
	m.map(ss,"foo");
	TEST(value(ss) == "test.com/foo");


	m.map(ss,"foo",1);
	TEST(value(ss) == "test.com/foo/1");
	m.map(ss,"foo",1,2);
	TEST(value(ss) == "test.com/foo/1/2");
	m.map(ss,"foo",1,2,3);
	TEST(value(ss) == "test.com/foo/1/2/3");
	m.map(ss,"foo",1,2,3,4);
	TEST(value(ss) == "test.com/foo/1/2/3/4");
	m.map(ss,"foo",1,2,3,4,5);
	TEST(value(ss) == "test.com/foo/1/2/3/4/5");
	m.map(ss,"foo",1,2,3,4,5,6);
	TEST(value(ss) == "test.com/foo/1/2/3/4/5/6");

	m.map(ss,"foo",1,"a","b",5);
	TEST(value(ss) == "test.com/foo/1/a/b/5");
	
	m.map(ss,"bar",1);
	TEST(value(ss) == "test.com/bar/en/1");
	m.map(ss,"bar",1,"ru");
	TEST(value(ss) == "test.com/bar/ru/1");
	m.root("");
	m.map(ss,"test1",10);
	TEST(value(ss) == "10x");
	m.map(ss,"test2",10);
	TEST(value(ss) == "x10");

	std::cout << "-- Testing throwing at invalid params" << std::endl;
	try {
		m.assign("x","a{");
		TEST(0);
	}
	catch(cppcms::cppcms_error const &e) {}
	catch(...) { TEST(0); }

	try {
		m.assign("x","a}");
		TEST(0);
	}
	catch(cppcms::cppcms_error const &e) {}
	catch(...) { TEST(0); }
	
	try {
		m.assign("x","a{0}");
		TEST(0);
	}
	catch(cppcms::cppcms_error const &e) {}
	catch(...) { TEST(0); }

	if(throws) {
		try {
			m.map(ss,"undefined");
			TEST(!"Should not be there");
		}
		catch(cppcms::cppcms_error const &e) {}
		
		try {
			m.map(ss,"undefined");
			TEST(!"Should not be there");
		}
		catch(cppcms::cppcms_error const &e) {}
		
		try {
			m.map(ss,"test1",1,2);
			TEST(!"Should not be there");
		}
		catch(cppcms::cppcms_error const &e) {}
	}
	else {

		m.map(ss,"undefined");
		TEST(value(ss)=="/this_is_an_invalid_url_generated_by_url_mapper");

		m.map(ss,"undefined");
		TEST(value(ss)=="/this_is_an_invalid_url_generated_by_url_mapper");

		m.map(ss,"test1",1,2);
		TEST(value(ss)=="/this_is_an_invalid_url_generated_by_url_mapper");
	}
}


struct test1 : public cppcms::application {
	test1(cppcms::service &s) : cppcms::application(s)
	{
		mapper().assign("/default");
		mapper().assign("somepath","/");
		mapper().assign("page","/{1}");
		mapper().assign("preview","/{1}/preview");
		mapper().assign("bylang","/{lang}");
		mapper().assign("byloc","/{lang}_{terr}");
		mapper().assign("byloc","/{lang}_{terr}/{1}");
	}
};

struct test2 : public cppcms::application {
	test1 bee;
	test2(cppcms::service &s) : 
		cppcms::application(s),
		bee(s)
	{
		mapper().assign("somepath","/");
		mapper().assign("page","/{1}");
		mapper().assign("preview","/{1}/preview");
		mapper().mount("bee","/bee{1}",bee);
	}
};

struct test_app : public cppcms::application {
	test1 foo,bar;
	test2 bee;

	test_app(cppcms::service &s) : 
		cppcms::application(s),
		foo(s),
		bar(s),
		bee(s)
	{ 
		add(foo);
		add(bar);
		add(bee);
		mapper().mount("foo","/foo{1}",foo);
		mapper().mount("bar","/bar{1}",bar);
		mapper().mount("foobar","/foobar{1}",bee);
		mapper().assign("somepath","/test");
		mapper().root("xx");

		bee.bee.mapper().set_value("lang","en");
		mapper().set_value("terr","US");
	}
	void test_hierarchy()
	{
		std::ostringstream ss;
		mapper().map(ss,"somepath");
		TEST(value(ss) == "xx/test");

		foo.mapper().map(ss,"somepath");
		TEST(value(ss) == "xx/foo/");
		foo.mapper().map(ss,"page",1);
		TEST(value(ss) == "xx/foo/1");
		foo.mapper().map(ss,"preview",1);
		TEST(value(ss) == "xx/foo/1/preview");

		bar.mapper().map(ss,"somepath");
		TEST(value(ss) == "xx/bar/");
		bar.mapper().map(ss,"page",1);
		TEST(value(ss) == "xx/bar/1");
		bar.mapper().map(ss,"preview",1);
		TEST(value(ss) == "xx/bar/1/preview");


		bee.mapper().map(ss,"somepath");
		TEST(value(ss) == "xx/foobar/");
		bee.mapper().map(ss,"page",1);
		TEST(value(ss) == "xx/foobar/1");
		bee.mapper().map(ss,"preview",1);
		TEST(value(ss) == "xx/foobar/1/preview");

		bee.bee.mapper().map(ss,"somepath");
		TEST(value(ss) == "xx/foobar/bee/");
		bee.bee.mapper().map(ss,"page",1);
		TEST(value(ss) == "xx/foobar/bee/1");
		bee.bee.mapper().map(ss,"preview",1);
		TEST(value(ss) == "xx/foobar/bee/1/preview");
		
		bee.bee.mapper().map(ss,"bylang");
		TEST(value(ss) == "xx/foobar/bee/en");

	}
	std::string u(cppcms::application &a,std::string const &param)
	{
		std::ostringstream ss;
		a.mapper().map(ss,param);
		return ss.str();
	}
	void test_mapping()
	{
		TEST(u(*this,"/somepath")=="xx/test");
		TEST(u(*this,"/foo/somepath")=="xx/foo/");
		TEST(u(*this,"/foo/")=="xx/foo/default");
		TEST(u(*this,"/foo")=="xx/foo/default");
		TEST(u(*this,"/foobar/bee/")=="xx/foobar/bee/default");
		TEST(u(*this,"/foobar/bee")=="xx/foobar/bee/default");
		TEST(u(*this,"/foobar/bee/bylang")=="xx/foobar/bee/en");
		
		TEST(u(*this,"somepath")=="xx/test");
		TEST(u(*this,"foo/somepath")=="xx/foo/");
		TEST(u(*this,"foobar/bee/somepath")=="xx/foobar/bee/");
		TEST(u(*this,"foobar/bee/bylang")=="xx/foobar/bee/en");
		
		TEST(u(bar,"/somepath")=="xx/test");
		TEST(u(bar,"/foo/somepath")=="xx/foo/");
		TEST(u(bar,"/foobar/bee/somepath")=="xx/foobar/bee/");
		TEST(u(bar,"/foobar/bee/")=="xx/foobar/bee/default");
		TEST(u(bar,"/foobar/bee")=="xx/foobar/bee/default");
		TEST(u(bar,"/foobar/bee/bylang")=="xx/foobar/bee/en");
		
		TEST(u(bee.bee,"/somepath")=="xx/test");
		TEST(u(bee.bee,"/foo/somepath")=="xx/foo/");
		TEST(u(bee.bee,"/foobar/bee/somepath")=="xx/foobar/bee/");
		TEST(u(bee.bee,"/foobar/bee/bylang")=="xx/foobar/bee/en");
		
		TEST(u(bee.bee,"somepath")=="xx/foobar/bee/");
		TEST(u(bee.bee,"bylang")=="xx/foobar/bee/en");
		
		TEST(u(bee.bee,"../somepath")=="xx/foobar/");
		TEST(u(bee.bee,"../../somepath")=="xx/test");
		TEST(u(bee.bee,"bylang")=="xx/foobar/bee/en");
		TEST(u(foo,"../foobar/bee/bylang")=="xx/foobar/bee/en");
	}
	void test_keywords()
	{
		std::ostringstream ss;
		mapper().map(ss,"/foobar/bee/bylang");
		TEST(value(ss)=="xx/foobar/bee/en");
		mapper().map(ss,"/foobar/bee/bylang;lang","ru");
		TEST(value(ss)=="xx/foobar/bee/ru");
		mapper().map(ss,"/foobar/bee/byloc;lang","ru");
		TEST(value(ss)=="xx/foobar/bee/ru_US");
		mapper().map(ss,"/foobar/bee/byloc;lang,terr","ru","RU");
		TEST(value(ss)=="xx/foobar/bee/ru_RU");
		mapper().map(ss,"/foobar/bee/byloc;terr","RU");
		TEST(value(ss)=="xx/foobar/bee/en_RU");
		mapper().map(ss,"/foobar/bee/byloc;terr,lang","UA","ru");
		TEST(value(ss)=="xx/foobar/bee/ru_UA");
		mapper().map(ss,"/foobar/bee/byloc;lang","ru","foo");
		TEST(value(ss)=="xx/foobar/bee/ru_US/foo");
		mapper().map(ss,"/foobar/bee/byloc;lang,terr","ru","RU","foo");
		TEST(value(ss)=="xx/foobar/bee/ru_RU/foo");
		mapper().map(ss,"/foobar/bee/byloc;terr","RU","foo");
		TEST(value(ss)=="xx/foobar/bee/en_RU/foo");
		mapper().map(ss,"/foobar/bee/byloc;terr,lang","UA","ru","foo");
		TEST(value(ss)=="xx/foobar/bee/ru_UA/foo");
	}
};


int main()
{
	try {


		std::cout << "- Basics no throw" << std::endl;

		cppcms::json::value cfg;
		cfg["localization"]["locales"][0]="en_US.UTF-8";
		{
			cppcms::service srv(cfg);
			basic_test(srv,false);
		}

		cfg["misc"]["invalid_url_throws"]=true;
		std::cout << "- Basics throw" << std::endl;

		{
			cppcms::service srv(cfg);
			basic_test(srv,true);
		}

		cppcms::service srv(cfg);
		
		std::cout << "- Hierarchy " << std::endl;
		test_app app(srv);

		app.test_hierarchy();

		std::cout << "- Mappting" << std::endl;
		app.test_mapping();
		std::cout << "- Keyword substitution" << std::endl;
		app.test_keywords();

		std::cout << "- Dispatcher Test" << std::endl;
		dispatcher_test(srv);

	}
	catch(std::exception const &e) {
		std::cerr << "Fail: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "ok" << std::endl;
	return 0;
}

