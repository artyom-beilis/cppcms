#include <stdio.h>
#include <stdlib.h>

int main(int argv,char **argc)
{
	char const *file_name="/dev/random";
	if(argv>=2) {
		if(argv>2 || strcmp(argc[1],"-h")==0 || strcmp(argc[1],"--help")==0) {
			printf(	"Usage: cppcms_make_key [ random-device ] [>>config.txt] \n"
				"       default random device is /dev/random\n"
				"       you may specify other, for example:\n"
				"\n"
				"       cppcms_make_key /dev/urandom >>config.txt\n"
				"\n");
			return 1;
		}
		file_name=argc[1];
	}
	FILE *f=fopen(file_name,"r");
	if(!f) {
		perror("fopen");
		return 1;
	}
	unsigned char buf[16];
	if(fread(buf,1,16,f)!=16) {
		fprintf(stderr,"Failed to read 16 bytes from file %s\n",file_name);
		fclose(f);
		return 1;
	}
	fclose(f);
	int i;
	printf(	"\n"
		"# This is your PRIVATE KEY --- keep it in secret!\n"
		"# Put this line into your configuration file\n"
		"\n"
		"session.cookies_key = \"" );
	for(i=0;i<sizeof(buf);i++) {
		printf("%02x",buf[i]);
	}
	printf("\"\n\n");
	return 0;
}
