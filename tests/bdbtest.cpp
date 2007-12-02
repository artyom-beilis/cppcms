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

	int N=10;
	
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
	{
#if 1
		cursor<data,int> cur(db);
		bool res;
		for(res=cur.begin();res;res=cur++) {
			d=cur;
			cout<<d.val<<' '<<d.sq<<endl;
			d.sq++;
			cur=d;
		}
#else
		for(i=0;i<N;i++) {
			db.get(i,d);
			cout<<d.val<<' '<<d.sq<<endl;
			d.sq++;
			db.update(d);
		}
#endif
		cout<<d.sq<<endl;
	}
	db.close();
	e.close();
	return 0;
}
