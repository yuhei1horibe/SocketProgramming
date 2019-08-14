#include "network.h"

bool   close_parent = false; // When this flag is set (signal), main routine will be terminated

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
                prctl(PR_SET_PDEATHSIG, SIGKILL);

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

