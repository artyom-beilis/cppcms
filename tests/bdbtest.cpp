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
	Index_Auto_Increment<data,int,&data::val> db(e,"test.db",DB_BTREE);

	data d;
	int i;

	int N=10;
        try{
        	if(argc==2 && argv[1][0]=='c') {
        		e.create();
        		db.create();
        	}
        	else {
        		e.open();
        		db.open();
        	}

                cursor<data,int> cur(db);

        	if(argc==2 && argv[1][0]=='c') {
                        for(i=0;i<N;i++) {
                                d.val=i;
                                d.sq=i*i;
                                if(db.insert(d)) {
                                        cout<<"Inserted: "<<d.val<<endl;
                                }
                                else {
                                        cout<<"Not Inserted: "<<d.val<<endl;
                                }
                        }
        	}

#if 1
		for(cur.begin();cur;cur++) {
			d=cur;
			cout<<d.val<<' '<<d.sq<<endl;
			/*if(d.val<10) {
				d.sq++;
				cur=d;
			}
			else {
				cur.del();
			}*/
		}
#elif 0
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
