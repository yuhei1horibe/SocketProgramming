#include "network.h"
#include "user_protocol.h"

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
    uint8_t* buffer      = NULL;
    int32_t  exit_status = 0;

    // Packet
    struct user_packet_t packet;
    struct item_data_t*  item_data;

    uint32_t iter;

    // Receive message and print it
    while(1) {
        memset(&packet, 0, sizeof(packet));

        // Receive operation and size
        recv(sockfd, (void*)&packet, sizeof(uint32_t) * 2, 0);

        // Quit command, or invalid command
        if(packet.operation >= QUIT) {
            exit_status = 1;
            break;
        }

        else if(packet.operation == 0) {
            printf("Invalid request from client.\n");
            exit_status = 1;
            break;
        }

        // TODO: Check data_size
        buffer = malloc(packet.data_size);
        if(buffer == NULL) {
            printf("Failed to allocate memory for data buffer\n");
            exit_status = 1;
            break;
        }

        // Receive rest of the packet
        recv(sockfd, (void*)buffer, packet.data_size, 0);
        packet.data = buffer;

        // Parse received packet
        switch(packet.operation) {
        case LIST_ITEMS:
            item_data = parse_packet(ITEM_LIST_PACKET, packet.data, packet.data_size);
            if(item_data == NULL) { 
                free(buffer);
                exit_status = 1;
                return exit_status;
            }
            for(iter = 0; iter < item_data->packet_data.item_list.num_items; iter++) {
                printf("Item%d: %s\n", iter + 1, item_data->packet_data.item_list.item_list[iter]);
            }
            break;
        case DELETE_ITEM:
        case GET_ITEM:
        case UPLOAD_ITEM:
            // TODO
            exit_status = 1;
            free(buffer);
            break;
        }
    }
    return exit_status;
}

