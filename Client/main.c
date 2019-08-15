
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "user_protocol.h"

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

    static char*       dummy[] = 
    {
        "test",
        "list",
        "program",
        "protocol",
        "study"
    };
    int                iter;
    int                data_ptr;

    struct user_packet_t packet;

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

        if(strcmp(buffer, "list\n") == 0) { 
            printf("List command.\n");
            // Initialize packet
            memset(&packet, 0, sizeof(packet));
            packet.operation = LIST_ITEMS;

            // Dummy data
            for(iter = 0; iter < (sizeof(dummy) / sizeof(char*)); iter++) {
                packet.data_size += strlen(dummy[iter]) + 1;
            }
            printf("Packet data size: %d\n", packet.data_size);
            packet.data = malloc(packet.data_size);
            if(packet.data == NULL) {
                printf("Failed to allocate memory for packet data.\n");
                close(sockfd);
                return -1;
            }

            // Prepare data
            data_ptr = 0;
            for(iter = 0; iter < (sizeof(dummy) / sizeof(char*)); iter++) {
                strcpy(&packet.data[data_ptr], dummy[iter]);
                packet.data[data_ptr + strlen(dummy[iter]) + 1] = '\0';
                //printf("data_ptr: %d, Copied string: %s\n", data_ptr, (char*)&packet.data[data_ptr]);
                data_ptr += strlen(dummy[iter]);
                packet.data[data_ptr++] = '\0';
            }

            // For debugging
            printf("packet_type: %d, data_size: %d\n", packet.operation, packet.data_size);
            for(iter = 0; iter < packet.data_size; iter++) {
                printf("data[%d]: %c\n", iter, packet.data[iter]);
            }

            // Send packet
            send(sockfd, (void*)&packet, sizeof(uint32_t) * 2, 0);  // Header
            send(sockfd, (void*)packet.data, packet.data_size, 0); // Data
        }

        // Send messages to server
        //send(sockfd, buffer, strlen(buffer), 0);
        //if((strcmp(buffer, "exit\n") == 0) || (strcmp(buffer, "quit\n") == 0)) {
        //    printf("Disconnected from client.\n");
        //    printf("Sent: %s\n", buffer);
        //    recv(sockfd, buffer, BUFF_SIZE, 0);
        //    printf("Received: %s\n", buffer);
        //    break;
        //}

        //else{
        //    printf("Sent: %s\n", buffer);
        //    recv(sockfd, buffer, BUFF_SIZE, 0);
        //    printf("Received: %s\n", buffer);
        //}
    }
    close(sockfd);

    return 0;
}

