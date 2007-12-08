#include <iostream>
#include <memory>
#include "main_thread.h"
#include "thread_pool.h"
#include "global_config.h"
#include "url.h"

#include "templates.h"
#include "data.h"

using namespace std;

Templates_Set templates;


auto_ptr<Users> users;
auto_ptr<Messages> all_messages;
auto_ptr<Texts_Collector> texts;

void setup()
{
	user_t user;
	user.username="artik";
	user.password="artik";
	cerr<<"Setting up: "<<users->id.add(user)<<endl;
	user.username="maas";
	user.password="maas";
	users->id.add(user);
}

int main(int argc,char **argv)
{
	try{
		global_config.load(argc,argv);

		templates.load();

		Environment env(global_config.sval( "bdb.path" ).c_str());
		users=auto_ptr<Users>(new Users(env));
		all_messages=auto_ptr<Messages>(new Messages(env));
		texts=auto_ptr<Texts_Collector>(new Texts_Collector(env,"texts.db"));

		env.create();

		users->create();
		all_messages->create();
		texts->create();


		if(argc>2 && strcmp(argv[1],"setup")==0) {
			setup();
		}

		Run_Application<Main_Thread>(argc,argv);

		users->close();
		all_messages->close();
		texts->close();
		env.close();

		cout<<"Exiting\n";
	}
	catch(HTTP_Error &s) {
		cerr<<s.get()<<endl;
		return 1;
	}
	catch(DbException &e) {
		cerr<<e.what()<<endl;
	}
	return 0;
}
