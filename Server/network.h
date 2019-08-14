#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT      5555
#define BUFF_SIZE 1024

// Initialize socket and make it "listen" state
int prepare_socket(int port_num);

// This is for child process
int communicate(int sockfd);

