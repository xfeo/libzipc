#include<stdlib.h>
#include<stdio.h>
#include <czmq.h>
#include <dlfcn.h>
#include "ipc.h"

#define MAX_LEN_FUNCTION_NAME 50
#define MAX_LEN_LIBRARY_NAME 50
#define MAX_LEN_CHANNEL_ZMQ 256

#ifdef TCP_IPC
static const char *g_zmq_protocol = "tcp:///";
#else
static const char *g_zmq_protocol = "ipc:///";  
#endif

typedef struct IPC_SHARED_DATA
{
  char library[MAX_LEN_LIBRARY_NAME];
  char function[MAX_LEN_FUNCTION_NAME];
  size_t data_size;
  int func_retval;
} IPC_SHARED_DATA;

static IPC_RESULT ipc_client_side (
#ifdef TCP_IPC
int port, 
#endif
void *context, const char *lib, const char *func, void *in,
		 size_t in_size, void **out, size_t * out_size, int *ret_val)
{
  IPC_RESULT result = SUCCESS;
  IPC_SHARED_DATA data;
  void *requester = zmq_socket (context, ZMQ_REQ);
  if (!requester)
    return FAILURE_ZMQ;
  char connection[MAX_LEN_CHANNEL_ZMQ] = {
    0
  };
#ifdef TCP_IPC
  snprintf (connection, sizeof (connection), "%s" PROCESSOR_TARGET_IP ":%d", g_zmq_protocol, port);
#else
  snprintf (connection, sizeof (connection), "%s%s.ipc", g_zmq_protocol, lib);    
#endif
  if (zmq_connect (requester, connection)) {
    zmq_close (requester);
    return FAILURE_ZMQ;
  }
  data.data_size = in_size;
  data.func_retval = 0;
  snprintf (data.library, sizeof (data.library), "%s", lib);
  snprintf (data.function, sizeof (data.function), "%s", func);
  if (zmq_send (requester, &data, sizeof (data), 0) < 0) {
    zmq_close (requester);
    return FAILURE_ZMQ;
  }

  if (in_size > 0) {
    if (zmq_send (requester, in, in_size, 0) < 0) {
      zmq_close (requester);
      return FAILURE_ZMQ;
    }
  }
  if (zmq_recv (requester, &data, sizeof (data), 0) < 0) {
    zmq_close (requester);
    return FAILURE_ZMQ;
  }
  *out_size = data.data_size;
  *ret_val = data.func_retval;
  *out = malloc (*out_size);
  if (!(*out)) {
    zmq_close (requester);
    return FAILURE_MALLOC;
  }
  if (*out_size > 0) {
    if (zmq_recv (requester, *out, *out_size, 0) < 0) {
      zmq_close (requester);
      return FAILURE_ZMQ;
    }
  }
  zmq_close (requester);
  return result;
}

static IPC_RESULT ipc_server_side (void *context
#ifdef TCP_IPC
, int port
#else
, const char *lib
#endif
)
{
  void *in = NULL;
  size_t in_size = 0;
  void *out = NULL;
  size_t out_size = 0;
  int (*func) (IPC_ROLE, void *, size_t, void **, size_t *, int *);
  int retval;
  IPC_RESULT result = SUCCESS;
  IPC_SHARED_DATA data;
  void *responder = zmq_socket (context, ZMQ_REP);
  if (!responder)
    return FAILURE_ZMQ;
  char connection[MAX_LEN_CHANNEL_ZMQ] = {
    0
  };
#ifdef TCP_IPC
  snprintf (connection, sizeof (connection), "%s*:%d", g_zmq_protocol, port);
#else
  snprintf (connection, sizeof (connection), "%s%s.ipc", g_zmq_protocol, lib);    
#endif
  if (zmq_bind (responder, connection)) {
    zmq_close (responder);
    return FAILURE_ZMQ;
  }
  for (;;) {
    in = NULL;
    in_size = 0;
    memset (&data, 0x00, sizeof (data));
    if (zmq_recv (responder, &data, sizeof (data), 0) < 0) {
      zmq_close (responder);
      return FAILURE_ZMQ;
    }
    if (data.data_size) {
      in = malloc (data.data_size);
      if (!in) {
        zmq_close (responder);
        return FAILURE_MALLOC;
      }
      in_size = data.data_size;
      if (zmq_recv (responder, in, in_size, 0) < 0) {
        free (in);
        zmq_close (responder);
        return FAILURE_ZMQ;
      }
    }
    void *handle = dlopen (data.library, RTLD_LAZY);
    if (!handle) {
      free (in);
      zmq_close (responder);
      return FAILURE_LIB;
    }
    dlerror ();			/* Clear any existing error */
    func = (int (*)(IPC_ROLE, void *, size_t, void **, size_t *, int *))
      dlsym (handle, data.function);
    if (func (IPC_SERVER, in, in_size, &out, &out_size, &retval)
	|| dlerror () != NULL) {
      free (in);
      zmq_close (responder);
      return FAILURE_LIB;
    }
    memset (&data, 0x00, sizeof (data));
    data.func_retval = retval;
    data.data_size = out_size;
    if (zmq_send (responder, &data, sizeof (data), ZMQ_SNDMORE) < 0) {
      free (in);
      zmq_close (responder);
      return FAILURE_ZMQ;
    }
    if (out_size > 0) {
      if (zmq_send (responder, out, out_size, 0) < 0) {
      free (in);
      zmq_close (responder);
      return FAILURE_ZMQ;
      }
    }
    free (in);
    free (out);
    dlclose (handle);
  }
  zmq_close (responder);
  return result;
}

IPC_RESULT ipc_call (
#ifdef TCP_IPC
int port,
#endif 
IPC_ROLE role, const char *lib, const char *func, void *in, size_t in_size, void **out, size_t *out_size, int *ret_val)
{
  IPC_RESULT result = SUCCESS;
  if (!ret_val || ((role == IPC_CLIENT) && (!func || !out)))
    return FAILURE_BAD_PARAMETERS;
  void *context = zmq_ctx_new ();
  if (!context)
    return FAILURE_ZMQ;
  if (role == IPC_CLIENT)
    result = ipc_client_side (
#ifdef TCP_IPC
                              port, 
#endif
                              context, lib, func, in, in_size, out, out_size, ret_val);
  else
    result = ipc_server_side (context
#ifdef TCP_IPC
                              ,port 
#else
                              ,lib
#endif
                             );
  zmq_ctx_destroy (context);
  return result;
}
