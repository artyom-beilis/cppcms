#include <dbixx/dbixx.h>
#include <iostream>

using namespace dbixx;
using namespace std;

int main()
{
	try {
		session sql("sqlite3");
		sql.param("dbname","test.db");
		sql.param("sqlite3_dbdir","./");
		sql.connect();

		sql<<"DROP TABLE IF EXISTS users";
		sql.exec();
		sql<<"CREATE TABLE users ( "
			 " id integer primary key not null, "
			 " name varchar(128) not null "
			 ");";
		sql.exec();
		sql<<"INSERT INTO users(id,name) VALUES(?,?)",
			 1,"Moshe",exec();
		sql<<"INSERT INTO users(id,name) VALUES(?,?)",
			 2,"Yossi",exec();
		sql<<"SELECT name FROM users WHERE id=?",1;
		row r;
		if(sql.single(r)) {
			string name;
			r>>name;
			cout<<name<<endl;
		}
		else {
			cout<<"No user with id="<<1<<endl;
		}
		result res;
		sql<<"SELECT id,name FROM users";
		sql.fetch(res);
		cout<<"There are "<<res.rows()<<" users\n";
		while(res.next(r)) {
			int id;
			string name;
			r>>id>>name;
			cout<<id<<"\t"<<name<<endl;
		}
	}
	catch(std::exception const &e) {
		cerr<<e.what()<<endl;
		return 1;
	}
	return 0;
}
