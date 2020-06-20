#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <daemon.h>

int main ()
{
  int pid = getpid ();
  printf ("This is only an example how to use libzipc.so\n");
  printf ("I am the CLIENT (pid: %d)\n", pid);
  /*Read the pid of the Server*/
  getServerPid (&pid);
  printf ("the SERVER pid is: %d\n", pid);
  return 0;
}
