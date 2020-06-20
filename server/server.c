#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <daemon.h>

int main ()
{
  printf ("This is only an example how to use libzipc.so\n");
  printf ("I am the SERVER (pid: %d)\n", getpid ());
  /* this start the server */
  startServer ();
  return 0;
}
