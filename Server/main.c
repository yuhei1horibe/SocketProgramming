
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

bool   close_parent = false; // When this flag is set (signal), main routine will be terminated

// *******************************************
// Parent process
// *******************************************
// Signal handler (To avoid making zombie process)
void child_close_handler(int signum)
{
    int ret_val;

    while(waitpid(-1, &ret_val, WNOHANG) > 0)
    {
        printf("Return from child: %d.\n", WEXITSTATUS(ret_val));
        // If return value is 1, close parent process
        if(WEXITSTATUS(ret_val) == 1) {
            close_parent = true;
        }
    }
    // accept() will fail when this signal handler returns
    return ;
}

// Signal handler (SIG_CHLD) initialization
int setup_sig_chld_handler()
{
    struct sigaction sa_chld;

    // Setup sigaction to close child process proplerly
    memset((void*)&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = child_close_handler;
    sa_chld.sa_flags   = SA_NOCLDSTOP;
    if(sigaction(SIGCHLD, &sa_chld, NULL) != 0) {
        printf("Failed to setup sigaction\n");
        return -1;
    }
    return 0;
}

// Socket initialization
int prepare_socket(int port_num)
{
    struct sockaddr_in server_addr;
    int                sockfd;
    int                temp = 1;

    // Create socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Failed to create a socket\n");
        return -1;
    }

    // Set socket options
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int)) < 0) {
        printf("Failed to set socket options\n");
        return -1;
    }

    // Set address
    memset((void*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT);

    // Bind
    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed to bind address to socket\n");
        close(sockfd);
        return -1;
    }

    // Listen (wait for the client to be connected
    printf("Listening on port: %d\n", port_num);
    if(listen(sockfd, 10) < 0) {
        printf("No connection.\n");
        return -1;
    }
    return sockfd;
}

// *******************************************
// Child process
// *******************************************
int communicate(int sockfd)
{
    // Buffer for message handling
    char               buffer[BUFF_SIZE];
    int                exit_status = 0;

    // Receive message and print it
    while(1) {
        memset(buffer, 0, BUFF_SIZE);
        recv(sockfd, buffer, BUFF_SIZE, 0);
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
                printf("Received: %s", buffer);
                send(sockfd, buffer, strlen(buffer), 0);
            }
        }
    }
    return exit_status;
}

// *******************************************
// main
// *******************************************
int main (int argc, char* argv[])
{
    // Client socket address
    struct sockaddr_in new_addr;

    // Create socket
    int                sockfd;
    int                new_socket;
    int                addr_size;

    // For forking
    int                pid;

    // Setup signal handler for SIG_CHLD
    if(setup_sig_chld_handler() < 0) {
        return -1;
    }

    // Create socket and make it "listen" state
    if((sockfd = prepare_socket(PORT)) < 0) {
        return -1;
    }

    // Accept connection and do some stuff
    while(1) {
        memset((void*)&new_addr, 0, sizeof(new_addr));
        addr_size = sizeof(new_addr);

        // This system call fails when signal is sent to this process
        new_socket = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);

        if(new_socket < 0) {
            // Every time parent process gets signal, check the flag
            if(errno == EINTR) {
                // If the flag is set, terminate parent process
                if(close_parent) {
                    break;
                }
            }

            else {
                printf("Failed to accept connection from client\n");
                return -1;
            }
        }

        else {
            printf("Connection accepted from %s:%d\n", inet_ntoa(new_addr.sin_addr), ntohs(new_addr.sin_port));
            pid = fork();
            if(pid < 0) {
                printf("Failed to create child process.\n");
                return -1;
            }

            // Child process
            else if(pid == 0) {
                // Don't let child touch the server socket
                close(sockfd);

                // Let child be killed when parent exits
                prctl(PR_SET_PDEATHSIG, SIGHUP);

                exit(communicate(new_socket));
            }

            // This is parent process
            else {
                // Close client socket
                close(new_socket);
            }
        }
    }
    printf("Parent process exited successfully.\n");
    close(sockfd);

    return 0;
}

