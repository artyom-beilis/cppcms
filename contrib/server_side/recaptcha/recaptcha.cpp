//
//  Copyright (C) 2012 Artyom Beilis
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "recaptcha.h"
#include <cppcms/http_context.h>
#include <cppcms/http_request.h>
#include <cppcms/steal_buf.h>
#include <curl/curl.h>
#include <curl/easy.h>

namespace cppcms_util {

namespace {
	class curl_init{
	public:
		curl_init() {curl_global_init(CURL_GLOBAL_ALL);};
	} init;
};

extern "C" {
	static size_t cppcms_util_widget_write_function(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		if(stream){
			std::ostream *out = static_cast<std::ostream *>(stream);
			out->write(static_cast<char const *>(ptr),size * nmemb);
			return size*nmemb;
		}
		return 0;
	}
}

recaptcha::~recaptcha()
{
}

recaptcha::recaptcha() 
{
}

void recaptcha::private_key(std::string const &key)
{
	private_ = key;
}

void recaptcha::public_key(std::string const &key)
{
	public_ = key;
}




void recaptcha::load(cppcms::http::context& context)
{
	response_ = context.request().post("recaptcha_response_field");
	challenge_ = context.request().post("recaptcha_challenge_field");
	remote_address_ = context.request().remote_addr();
}

bool recaptcha::validate()
{
	CURL *curl = 0;;
	CURLcode res=CURLE_OK;
	cppcms::util::stackbuf<> sb;
	std::ostream ss(&sb);

	char const *content_type="Content-Type: application/x-www-form-urlencoded; charset=UTF-8";
	std::string post_data;
	
	post_data.reserve(256);
	post_data += "privatekey=";
	post_data += private_;
	post_data += "&remoteip=";
	post_data += cppcms::util::urlencode(remote_address_);
	post_data += "&challenge=";
	post_data += cppcms::util::urlencode(challenge_);
	post_data += "&response=";
	post_data += cppcms::util::urlencode(response_);
	
	
	curl = curl_easy_init();
	if(!curl)
		throw booster::runtime_error("Failed to allocate CURL object");
	
	curl_slist *headerlist = curl_slist_append(NULL,content_type);
	
	/* what URL that receives this POST */
	curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com/recaptcha/api/verify");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,cppcms_util_widget_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA,static_cast<void *>(&ss));
	res = curl_easy_perform(curl);

	/* always cleanup */
	curl_easy_cleanup(curl);

	/* free slist */
	curl_slist_free_all (headerlist);
	if(res!=CURLE_OK || strncmp(sb.c_str(),"true\n",5) != 0) {
		valid(false);
		return false;
	}
	valid(true);
	return true;
}

void recaptcha::render_input(cppcms::form_context& context)
{
	if(context.widget_part()!=cppcms::form_context::first_part)
		return;
	context.out() << 
		"<script type='text/javascript' "
		"src='http://www.google.com/recaptcha/api/challenge?k=" << public_ << "'></script>\n"
		"<noscript>\n"
		"<iframe src='http://www.google.com/recaptcha/api/noscript?k=" << public_ << "' \n"
		"     height='300' width='500' frameborder='0'></iframe><br>\n"
		"<textarea name='recaptcha_challenge_field' rows='3' cols='40'></textarea>\n"
		"<input type='hidden' name='recaptcha_response_field' value='manual_challenge'>\n"
		"</noscript>\n";

}


void recaptcha::render(cppcms::form_context& context)
{
	auto_generate(&context);
	context.widget_part(first_part);
	render_input(context);
	context.widget_part(second_part);
	render_input(context);

}


} // namespace cppcms_util