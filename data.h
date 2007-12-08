//
// C++ Interface: data
//
// Description:
//
//
// Author: artik <artik@artyom-linux>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef DATA_H
#define DATA_H

#include "easy_bdb.h"

using namespace ebdb;

typedef varchar<32> username_t;
typedef varchar<16> password_t;

struct user_t {
	int id;
	username_t	username;
	password_t	password;
};

struct message_t {
	int id;
	int user_id;
	long text_id;
};

class Users {
public:
	typedef Index_Auto_Increment<user_t,int,&user_t::id> id_idx;
	typedef id_idx::cursor_t id_c;
	id_idx id;

	typedef Index_Var<user_t,username_t,&user_t::username> username_idx;
	typedef username_idx::cursor_t username_c;
	username_idx username;
	Users(Environment &env) :
			id(env,"users_id.db",DB_BTREE),
			username(env,"users_username.db",DB_BTREE,&id)
	{};
	void open() { id.open(); username.open();};
	void create() { id.create(); username.create();};
	void close() { id.close(); username.close();};
};

class Messages {
public:
	typedef Index_Auto_Increment<message_t,int,&message_t::id> id_idx;
	typedef id_idx::cursor_t id_c;
	id_idx id;

	Messages(Environment &env) :
			id(env,"messages_id.db",DB_BTREE)
	{};
	void open() { id.open();};
	void create() { id.create(); };
	void close() { id.close();};
};




extern auto_ptr<Users> users;
extern auto_ptr<Messages> all_messages;
extern auto_ptr<Texts_Collector> texts;




#endif
