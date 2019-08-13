
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT      5555
#define BUFF_SIZE 1024

struct sigaction sa_chld;

// To close child process properly
void child_close_handler(int signum)
{
    wait(NULL);
    return ;
}

int main (int argc, char* argv[])
{
    // Socket address
    struct sockaddr_in server_addr;
    struct sockaddr_in new_addr;

    // Create socket
    int                sockfd;
    int                new_socket;
    int                addr_size;

    // For forking
    int                pid;

    // Buffer
    char               buffer[BUFF_SIZE];
    int                exit_status = 0;

    // Setup sigaction to close child process proplerly
    memset((void*)&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = child_close_handler;
    sa_chld.sa_flags   = 0;
    if(sigaction(SIGCHLD, &sa_chld, NULL) != 0) {
        printf("Failed to setup sigaction\n");
        return -1;
    }

    // Create socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Failed to create a socket\n");
        return -1;
    }

    // Set address
    memset((void*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT);

    // Bind
    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Bind failed\n");
        close(sockfd);
        return -1;
    }

    // Listen (wait for the client to be connected
    printf("Listening on port: %d\n", PORT);
    if(listen(sockfd, 10) < 0) {
        printf("No connection.\n");
        close(sockfd);
        return -1;
    }

    // Accept connection and do some stuff
    while(1) {
        memset((void*)&new_addr, 0, sizeof(new_addr));
        addr_size = sizeof(new_addr);
        new_socket = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
        if(new_socket < 0) {
            printf("Failed to accept connection from client\n");
            close(sockfd);
            return -1;
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(new_addr.sin_addr), ntohs(new_addr.sin_port));

        pid = fork();
        if(pid < 0) {
            printf("Failed to create child process.\n");
            close(sockfd);
            close(new_socket);
            return -1;
        }

        // Child process
        if(pid == 0) {
            // Don't let child touch the server socket
            close(sockfd);

            while(1) {
                // Receive message and print it
                memset(buffer, 0, BUFF_SIZE);
                recv(new_socket, buffer, BUFF_SIZE, 0);
                if(strcmp(buffer, "quit\n") == 0) {
                    printf("Close program.\n");
                    exit_status = 1;
                    break;
                }

                else if(strcmp(buffer, "exit\n") == 0) {
                    printf("Disconnected from client.\n");
                    exit_status = 0;
                    break;
                }

                else{
                    if(strlen(buffer) > 0) {
                        printf("Received: %s\n", buffer);
                        send(new_socket, buffer, strlen(buffer), 0);
                    }
                }
            }
            // Close child process
            //close(new_socket);
            exit(exit_status);
        }

        // This is parent process
        else {
            // Close client socket
            close(new_socket);
        }
    }
    printf("Parent process exited.\n");

    return 0;
}

