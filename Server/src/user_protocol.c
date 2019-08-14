#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user_protocol.h"

struct item_data_t* parse_packet(uint32_t packet_type, uint8_t* data, uint32_t size)
{
    struct item_data_t* item_data = NULL;

    // In case of list
    uint32_t item_count = 0;
    uint32_t iter;

    if(data == NULL) {
        return NULL;
    }

    if((item_data = malloc(sizeof(struct item_data_t))) == NULL) {
        printf("Failed to allocate memory for parsed packet.\n");
        return NULL;
    }
    // Store row data
    memset((void*)item_data, 0, sizeof(item_data));
    item_data->packet_type   = packet_type;
    item_data->row_data      = data;
    item_data->row_data_size = size;

    // Parse packet according to the packet type
    switch(packet_type) {
    case ITEM_LIST_PACKET:
        // Count the number of strings in this data
        // If format is wrong, last data isn't counted as a valid data
        for(iter = 0; iter < size; iter++) {
            if(data[iter] == '\0') {
                item_count++;
            }
        }

        if(item_count == 0) {
            printf("Invalid list format\n");
            free(item_data);
            return NULL;
        }

        // Allocate memory
        item_data->packet_data.item_list.item_list = malloc(sizeof(uint8_t*) * item_count);
        if(item_data->packet_data.item_list.item_list == NULL) {
            free(item_data);
            printf("Failed to allocate memory for item list.\n");
            return NULL;
        }

        item_data->packet_data.item_list.num_items = item_count;
        item_count = 0;
        item_data->packet_data.item_list.item_list[item_count] = &data[0];
        for(iter = 0; iter < size; iter++) {
            // To omit the case the last one is invalid
            if((data[iter] == '\0') && (item_count != item_data->packet_data.item_list.num_items)) {
                if(iter != (size - 1)) {
                    item_data->packet_data.item_list.item_list[++item_count] = &data[iter + 1];
                }
                // Check format
                if(strlen(item_data->packet_data.item_list.item_list[item_count - 1]) == 0) {
                    printf("Invalid item list format.\n");
                    free(item_data->packet_data.item_list.item_list);
                    free(item_data);
                    return NULL;
                }
            }
        }
        break;
    case ITEM_INFO_PACKET:
        printf("TODO\n");
        break;
    default:
        printf("Invalid packet type.\n");
        free(item_data);
        return NULL;
    }

    return item_data;
}

