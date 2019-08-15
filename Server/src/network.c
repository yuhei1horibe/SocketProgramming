#include "network.h"
#include "user_protocol.h"

// Socket initialization
int prepare_socket(int port_num)
{
    struct sockaddr_in server_addr;
    int                sockfd;
    int                temp     = 1;

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
    // Packet
    struct item_data_t*  item_data;
    uint32_t iter;
    int      exit_status = 0;

    // Receive packet
    item_data = receive_packet(sockfd);
    if(item_data == NULL){
        exit_status = 1;
    }

    // This is test for list item command
    for(iter = 0; iter < item_data->packet_data.item_list.num_items; iter++) {
        printf("Item%d: %s\n", iter + 1, item_data->packet_data.item_list.item_list[iter]);
    }
    free(item_data);
    return exit_status;
}

