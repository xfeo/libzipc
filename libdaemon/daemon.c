#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "ipc.h"
#include "daemon.h"

typedef enum RESULT
{
  OK,
  KO
} RESULT;

int __getPidv (IPC_ROLE role, void *in, size_t in_size, void **out,
	   size_t * out_size, int *retval)
{
  if (role == IPC_SERVER) {
      /*SERVER SIDE*/
    if (!out || !out_size || !retval)
      return KO;
    pid_t serverPid = getpid ();
    *out = malloc (sizeof (serverPid));
    if (*out) {
      *out_size = sizeof (pid_t);
      memcpy (*out, &serverPid, sizeof (pid_t));
      *retval = 0;
      return OK;
    }
    else
      *retval = -1;
  }
  else {
    /* CLIENT SIDE*/
    void *tmp_out = NULL;
    size_t tmp_out_size = 0;
    if (ipc_call(LIBDAEMON_TCPPORT, IPC_CLIENT, LIBNAME, __func__, NULL, 0, &tmp_out,&tmp_out_size, retval) == SUCCESS) {
      if (tmp_out_size == *out_size) {
        memcpy (*out, tmp_out, tmp_out_size);
        return OK;
      }
    }
  }
  return KO;
}

int getServerPid (pid_t * pid)
{
  int retval = 0;
  void *out = pid;
  size_t out_size = sizeof (*pid);
  if (!__getPidv (IPC_CLIENT, NULL, 0, &out, &out_size, &retval))
    return retval;
  return -1;
}

int startServer ()
{
  int retval = 0;
  void *out = NULL;
  size_t out_size = 0;
  if (ipc_call(LIBDAEMON_TCPPORT, IPC_SERVER, NULL, NULL, NULL, 0, NULL, NULL, &retval) == SUCCESS)
    return retval;
  return -1;
}
