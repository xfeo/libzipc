

**INTRODUCTION**
-------------------------------------------------------------
The following example performs a simple IPC between two processes one client and the other server.
The "Server" process acts as *Remote Command Server*, it waits to receive a request by the "Client" and serves it.

This example is composed by four parts, each one of them stored in an own folder:

 * **libzipc**: it contains the IPC logic, this component utilizes ZeroMQ library to implement the ipc channel
 * **libdaemon**: it contains the service logic
 * **client**: it utilises libdaemon to perfome the service request
 * **server**: it utilises libdaemon to perfome the service response

*libdaemon* example implements only one service 'getServerPid( )' is utilised by the server to return to client its process ID (PID). The *libdaemon* source code is articulated to semplify the library exstention and to privide the developer with a standard service structure and for this reason the logic of REQUEST/RESPONCE is stored in the same function '__getServerPid( )'.

**Libzipc**
-------------------------------------------------------------
The library **libzipc** exposes the only one function:

> IPC_RESULT ipc_call (IPC_ROLE role, const char *lib, const char *func, void *in, size_t in_size, void **out, size_t *out_size, int *ret_val); 

- this fuction is utilised by both server and client; the first call it specifing the IPC_SERVER **role** the second as IPC_CLIENT.
- **func** and **lib** parameters are instead utilised by the client to specify the function name that the server must execute and the shared library where this function is stored.
- **in**, **in_size**, **out** and **out_size** are the parameters utilized to specify an input and an out parameter structure for the function. 
- finally, the remote function return is passed to the client by **ret_val**.

**Libdaemon**
-------------------------------------------------------------
The library **libdaemon** specifies the logic of the service; inside there are:
- the function **startServer ()** that rappresents the function invoked by the server to start.
- the function **getServerPid ()**, just wrapper for the function __getServerPid() to adapt/mask the function parameters to the format:

> int function_name (IPC_ROLE role, void *in, size_t in_size, void **out, size_t * out_size, int *retval);

- the function **__getServerPid()** that perform the service and it is called by both server and client, the body of this function is composed by two section:
	- the IPC_CLIENT part, called only by the client during the request.
	- the IPC_SERVER part, called only by the server when the command request is received to process the data for the responce.

usually, the IPC_SERVER part allocate the return struct value and the IPC_CLIENT part process and dellocate it.

**INSTALLATION**
-------------------------------------------------------------
 
- The only dependency of this example is the **ZeroMQ** library for C (https://zeromq.org/download/).

UBUNTU :
> sudo apt install libzmq3-dev

> cd libzipc

> mkdir build && cd build

> cmake .. && cmake --build . && sudo make install

