//
//  Copyright (C) 2012 Artyom Beilis
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_UTIL_RECAPTCHA_H
#define CPPCMS_UTIL_RECAPTCHA_H

#include <cppcms/form.h>

namespace cppcms_util {
	///
	/// \brief Recaptcha Widget
	///
	/// This is a simple widgets that allows to embed recaptcha
	/// into CppCMS form.
	///
	/// It requires linking with libcurl
	///
	/// In order to use add it into the form, note, you need to
	/// setup both private and public keys before you use it.
	///
	/// Upon validation the recaptcha widget would try to acces
	/// google services to perform the validation
	///
	/// You can check if the recaptcha result is valid by checking
	/// that form validates and specifically calling recaptcha::valid()
	/// after validation 
	///
	class recaptcha : public cppcms::widgets::base_widget {
	public:
		
		recaptcha();
		virtual ~recaptcha();
		///
		/// Set Public Key - Required
		///
		void public_key(std::string const &key);
		///
		/// Set Private Key - Required
		///
		void private_key(std::string const &key);

		// Widget support functions
		virtual void render(cppcms::form_context &context);
		virtual void render_input(cppcms::form_context &context);
		virtual void load(cppcms::http::context &context);
		virtual bool validate();
	private:
		std::string public_;
		std::string private_;
		std::string response_;
		std::string challenge_;
		std::string remote_address_;
	};
}

#endif
