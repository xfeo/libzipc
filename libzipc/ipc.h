#ifndef __LIB_ZIPC__
#define __LIB_ZIPC__

#ifndef PROCESSOR_TARGET_IP
#define PROCESSOR_TARGET_IP "localhost"
#endif

typedef enum IPC_RESULT
{
  SUCCESS,
  FAILURE_BAD_PARAMETERS,
  FAILURE_BUFFER_SIZE,
  FAILURE_MALLOC,
  FAILURE_LIB,
  FAILURE_ZMQ
} IPC_RESULT;


typedef enum IPC_ROLE
{
  IPC_SERVER,
  IPC_CLIENT
} IPC_ROLE;

IPC_RESULT ipc_call (
#ifdef TCP_IPC
                     int port, 
#endif
                     IPC_ROLE role, const char *lib, const char *func, void *in, size_t in_size, void **out, size_t *out_size, int *ret_val);


#endif /*__LIB_ZIPC__*/
