#ifndef CPPCMS_SESSION_INTERFACE_H
#define CPPCMS_SESSION_INTERFACE_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"
#include "intrusive_ptr.h"
#include <string>
#include <map>
#include <memory>
#include <sstream>
#include <typeinfo>

namespace cppcms {
namespace http {
	class context;
	class request;
	class response;
}

class session_api;

class CPPCMS_API session_interface : private util::noncopyable {
public:

	session_interface(http::context &);
	~session_interface();

	bool is_set(std::string const &key);
	void erase(std::string const &key);
	void clear();

	bool is_exposed(std::string const &key);
	void expose(std::string const &key,bool val=true);
	void hide(std::string const &key);

	std::string &operator[](std::string const &);

	std::string get(std::string const &key);
	void set(std::string const &key,std::string const &v);
	
	template<typename T>
	T get(std::string const &key)
	{
		std::istringstream ss(get(key));
		ss.imbue(std::locale::classic());
		T value;
		ss>>value;
		if(ss.fail() || !ss.eof())
			throw std::bad_cast();
		return value;
	}

	template<typename T>
	void set(std::string const &key,T const &value)
	{
		std::ostringstream ss;
		ss.imbue(std::locale::classic());
		ss<<value;
		set(key,value);
	}
	

	enum { fixed, renew, browser };

	int age();
	void age(int t);
	void default_age();

	int expiration();
	void expiration(int h);
	void default_expiration();

	void on_server(bool srv);
	bool on_server();


	// Following interface should be used only by user
	// implementations of session_api

	void set_session_cookie(std::string const &data);
	void clear_session_cookie();
	std::string get_session_cookie();

private:
	friend class http::response;
	friend class http::request;

	bool load();
	void save();


	struct entry;

	typedef std::map<std::string,entry> data_type;
	data_type data_,data_copy_;
	http::context *context_;

	// Cached defaults
	int timeout_val_def_;
	int how_def_;

	// User Values
	int timeout_val_;
	int how_;

	// Information from session data
	time_t timeout_in_;
	bool new_session_;
	bool saved_;
	bool on_server_;

	std::string temp_cookie_;

	// storage itself
	
	intrusive_ptr<session_api> storage_;
	struct data;
	util::hold_ptr<data> d; // for future use

	int cookie_age();
	time_t   session_age();

	void check();
	void update_exposed(bool); 


	void set_session_cookie(int64_t age,std::string const &data,std::string const &key=std::string());

	void save_data(std::map<std::string,entry> const &data,std::string &s);
	void load_data(std::map<std::string,entry> &data,std::string const &s);
};


} // cppcms


#endif
