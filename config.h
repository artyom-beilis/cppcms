#ifndef _CONFIG_H
#define _CONFIG_H

#define MAX_PASS_LEN 32
#define MAX_USERNAME_LEN 64
	
#define TS_BUFFER_SIZE		1024
#define INITAL_ALLOCATION	0x10000

#define MAX_ERROR_MESSAGE_LEN 256

#define CONF_MYSQL_MAX_PARAMETERS_LEN 128

struct All_Configuration {
	char mysql_host[CONF_MYSQL_MAX_PARAMETERS_LEN];
	char mysql_uname[CONF_MYSQL_MAX_PARAMETERS_LEN];
	char mysql_password[CONF_MYSQL_MAX_PARAMETERS_LEN];
	char mysql_db[CONF_MYSQL_MAX_PARAMETERS_LEN];
};

#endif /* _CONFIG_H */
