#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "daemon.h"
#include <unistd.h>

int main() {
    printf(" SERVER (%d)\n", getpid());
    startServer();
}