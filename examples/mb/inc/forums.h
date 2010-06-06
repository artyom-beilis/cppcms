#ifndef FORUMS_H
#define FORUMS_H
#include <cppcms/application.h>

namespace apps {
using namespace std;
class mb;

class forums : public cppcms::application {
	mb &board;
	void display_forums(string page);
public:
	forums(mb &);
	string forums_url(int offset=0);
};

}

#endif
