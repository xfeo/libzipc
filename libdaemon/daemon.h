#ifndef __LIB_DAEMON__
#define __LIB_DAEMON__

#define LIBDAEMON_TCPPORT 5555
#define LIBNAME "libdaemon.so"

int startServer ();
int getServerPid (int *pid);

#endif
