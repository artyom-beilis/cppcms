#include "transtext.h"
#include <stdio.h>
#include <cstdlib>

using namespace cppcms::transtext;


int main(int argc,char **argv)
{
	trans_factory tf;

	vector<string> domains;
	domains.push_back("test");
	vector<string> langs;
	langs.push_back("en");
	langs.push_back("he");
	tf.load("./locale",langs,"",domains,"");

	int i;
	for(i=0;i<15;i++) {
		printf(tf.get("he","").ngettext("passed one day","passed %d days",i),i);
		putchar('\n');
		printf(tf.get("en","").ngettext("passed one day","passed %d days",i),i);
		putchar('\n');
	}

	return 0;
}
