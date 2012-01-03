///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/base_content.h>
#include <cppcms/cppcms_error.h>

namespace cppcms {

struct base_content::_data {};

base_content::base_content() : 
	app_(0)
{
}

base_content::base_content(base_content const &other) :
	d(other.d),
	app_(other.app_)
{
}

base_content::~base_content()
{
}

base_content const &base_content::operator=(base_content const &other)
{
	d = other.d; 
	app_ = other.app_;
	return *this;
}

void base_content::app(application &app)
{
	app_ = &app;
}

bool base_content::has_app()
{
	return app_;
}

application &base_content::app()
{
	if(!app_) {
		throw cppcms_error("cppcms::base_content: an attempt to access content's application that wasn't assigned");
	}
	return *app_;
}

void base_content::reset_app()
{
	app_ = 0;
}


}// cppcms
