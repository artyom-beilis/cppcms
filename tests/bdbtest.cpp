#include "easy_bdb.h"
#include <iostream>

using namespace std;
using namespace ebdb;

struct data {
	int val;
	int sq;
};


int main(int argc,char **argv)
{
	Environment e("./db/");
	Index_Var<data,int,&data::val> db(e,"test.db",DB_BTREE);
	
	data d;
	int i;

	int N=100;
	
	if(argc==2 && argv[1][0]=='c') {
		e.create();
		db.create();
		for(i=0;i<N;i++) {
			d.val=i;
			d.sq=i*i;
			db.insert(d);
		}
	}
	else {
		e.open();
		db.open();
	}

	cout<<"Sleep\n";
//	sleep(5);
	cout<<"Reading\n";
	try{
#if 1
		cursor<data,int> cur(db);
		for(cur>=5;cur;cur++) {
			d=cur;
			cout<<d.val<<endl;
			if(d.val<10) {
				d.sq++;
				cur=d;
			}
			else {
				cur.del();
			}
		}
#else
		for(i=0;i<N;i++) {
			db.get(i,d);
	//		cout<<d.val<<' '<<d.sq<<endl;
			d.sq++;
			db.update(d);
		}
#endif
		cout<<d.sq<<endl;
	}
	catch(DbException &e) {
		cerr<<e.what()<<endl;
	}
	db.close();
	e.close();
	return 0;
}
