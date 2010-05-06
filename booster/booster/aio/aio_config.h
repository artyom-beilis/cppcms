#ifndef BOOSTER_AIO_CONFIG_H
#define BOOSTER_AIO_CONFIG_H

#include <booster/config.h>

#if defined(BOOSTER_WIN32)
#  define BOOSTER_AIO_NO_PF_UNIX
#  define BOOSTER_AIO_NO_PF_INET6
#endif


#endif // AIO_CONFIG_H
