
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT      5555
#define BUFF_SIZE 1024

int main (int argc, char* argv[])
{
    // Socket address
    struct sockaddr_in server_addr;

    // Create socket
    int                sockfd;
    int                new_socket;
    int                addr_size;

    // For forking
    int                pid;

    // Buffer
    char               buffer[BUFF_SIZE];

    // Check command line argument
    if(argc != 2) {
        printf("Usage: main.out <Server IP address>.\n");
        return -1;
    }

    // Create socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Failed to create a socket\n");
        return -1;
    }

    // Set address
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(PORT);
    if(inet_pton(AF_INET, argv[1], &server_addr.sin_addr) < 0) {
        printf("Failed to convert IP address %s\n", argv[1]);
        return -1;
    }

    // Connect to the server
    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed to connect to server.\n");
        close(sockfd);
        return -1;
    }

    // Accept connection and do some stuff
    while(1) {
        printf("Prompt> ");
        fgets(buffer, BUFF_SIZE, stdin);

        // Send messages to server
        send(sockfd, buffer, strlen(buffer), 0);
        if((strcmp(buffer, "exit\n") == 0) || (strcmp(buffer, "quit\n") == 0)) {
            printf("Disconnected from client.\n");
            printf("Sent: %s\n", buffer);
            recv(sockfd, buffer, BUFF_SIZE, 0);
            printf("Received: %s\n", buffer);
            break;
        }

        else{
            printf("Sent: %s\n", buffer);
            recv(sockfd, buffer, BUFF_SIZE, 0);
            printf("Received: %s\n", buffer);
        }
    }
    close(sockfd);

    return 0;
}

