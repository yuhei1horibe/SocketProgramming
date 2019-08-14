#include "network.h"

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

