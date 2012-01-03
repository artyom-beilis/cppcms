//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_SYSTEM_ERROR_H
#define BOOSTER_SYSTEM_ERROR_H

#include <string>
#include <booster/backtrace.h>
#include <functional>

#include <booster/config.h>

namespace booster {

///
/// \brief this namespace includes partial implementation of std::tr1's/boost's system_error, error_code
/// classes 
///
namespace system {

	///
	/// \brief this class represents a category of errors.
	///
	/// Technically it allows to convert a numerical error representation to 
	/// human readable error message.
	///
	/// This class is just an interface that should be implemented for each
	/// specific category
	///
	class error_category {
	public:
		virtual ~error_category()
		{
		}
		///
		/// The name of the category
		///
		virtual char const *name() const = 0;
		///
		/// Convert the error code representation to the human readable text
		///
		virtual std::string message(int ev) const = 0;

		bool operator==(error_category const &other) const
		{
			return this==&other;
		}
		bool operator!=(error_category const &other) const
		{
			return this!=&other;
		}
		bool operator<(error_category const &other) const
		{
			return std::less<error_category const *>()(this,&other);
		}

	};

	BOOSTER_API error_category const &get_system_category();
	static const error_category &system_category = get_system_category();
	
	#ifdef BOOSTER_WIN32
	BOOSTER_API error_category const &get_windows_category();
	static const error_category &windows_category = get_system_category();
	#endif

	#ifdef BOOSTER_POSIX
	BOOSTER_API error_category const &get_posix_category();
	static const error_category &posix_category = get_system_category();
	#endif


	///
	/// \brief The lightweight object that carries a error code information and its
	/// category.
	///
	/// It is a pair: an integer code and a reference to the error_category object.
	///
	class error_code {
	public:
		///
		/// Create an empty error object - no error
		///
		error_code() : 
			value_(0),
			category_(&system_category)
		{
		}
		///
		/// Create a error object withing specific category \a cat with code \a val
		///
		error_code(int val,error_category const &cat) :
			value_(val),
			category_(&cat)
		{
		}
		///
		/// Get the numeric code of the error
		///
		int value() const
		{
			return value_;
		}
		///
		/// Get the reference to the specific category
		///
		const error_category &category() const
		{
			return *category_;
		}
		///
		/// Convert the error code to the human readable string
		///
		std::string message() const
		{
			return std::string(category_->name()) + ": " + category_->message(value_);
		}
		///
		/// Convert to bool - returns true of it is a error and false if it is not (value()==0)
		///
		operator bool () const
		{
			return value() != 0;
		}
	private:
		int value_;
		error_category const *category_;
	};

	///
	/// Compare two error code for equality
	///
	inline bool operator==(error_code const &left,error_code const &right)
	{
		return left.value() == right.value() && left.category() == right.category(); 
	}
	///
	/// Compare two error code for inequality
	///
	inline bool operator!=(error_code const &left,error_code const &right)
	{
		return !(left==right);
	}

	///
	/// \brief This is the object that should be thrown in case of the error.
	///
	/// It consists
	/// of two parts: the error_code object and the optional error message that would
	/// be added to the error code itself.
	///
	class system_error : public booster::runtime_error {
	public:
		///
		/// Create a system error from error_code \a e 
		///
		system_error(error_code const &e) :
			booster::runtime_error(e.message()),
			error_(e)
		{
		}
		///
		/// Create a system error from error_code \a e and additional message
		///
		system_error(error_code const &e,std::string const &message) :
			booster::runtime_error(e.message()+": " + message),
			error_(e)
		{
		}
		///
		/// Create a system error from error_code \a e and an additional message \a message
		///
		system_error(error_code const &e,char const *message) :
			booster::runtime_error(e.message()+": " + message),
			error_(e)
		{
		}
		///
		/// Create a system error from error_code defined by integer code \a ev, a error category
		/// \a category and an additional message \a message
		///
		system_error(int ev,error_category const &category,char const *message) :
			booster::runtime_error(
				std::string(category.name()) 
				+ ": " 	+ category.message(ev) 
				+ ": "  + message
				),
			error_(ev,category)
		{
		}
		///
		/// Create a system error from error_code defined by integer code \a ev, a error category
		/// \a category and an additional message \a message
		///
		system_error(int ev,error_category const &category,std::string const &message) :
			booster::runtime_error(
				std::string(category.name()) 
				+ ": " 	+ category.message(ev) 
				+ ": "  + message
				),
			error_(ev,category)
		{
		}
		///
		/// Create a system error from error_code defined by integer code \a ev, a error category
		/// \a category
		///
		system_error(int ev,error_category const &category) :
			booster::runtime_error(
				std::string(category.name()) 
				+ ": " 	+ category.message(ev) 
				),
			error_(ev,category)
		{
		}
		/// 
		/// Get the error code for the thrown error
		///
		error_code const &code() const
		{
			return error_;
		}
	private:
		error_code error_;
	};

} // system
} // booster


#endif
