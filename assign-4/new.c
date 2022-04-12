#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>

//ip addr
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
//memset
#include <string.h> 
//misc/threads
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
//files
#include <sys/stat.h>
//for tid,ssize_t
#include <sys/types.h>
#include <sys/syscall.h>

#define MAXSIZE 1024
#define MAXCONN 5



int main(int argc, char const *argv[])
{
    printf("Hello!\n");
}