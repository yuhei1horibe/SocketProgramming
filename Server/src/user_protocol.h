/**
 * @author Yuhei Horibe
 * @brief User protocol communication definitions
 */

#ifndef __USER_PROTOCOL_H__
#define __USER_PROTOCOL_H__

#include <stdint.h>

// Definition of operations
#define LIST_ITEMS  1
#define DELETE_ITEM 2
#define GET_ITEM    3
#define UPLOAD_ITEM 4
#define QUIT        5

// Packet type definition
#define ITEM_LIST_PACKET 0x10
#define ITEM_INFO_PACKET 0x11

/**
 * @var operation In request packet, this indicates an operation.
 * In acknowledge packet, this contains the same value as request.
 * @var reserved This is to align 32bit boundary.
 * @var data_size This contains data size of the packet data.
 * @var data This is the actual data.
 */
struct user_packet_t {
    uint8_t  operation;
    uint8_t  reserved[3]; // For alignment
    uint32_t data_size;
    uint8_t* data;
};

/**
 * @var packet_type Indicates the type of the packet
 * @var row_data_size Contains data size of row data
 * @var row_data This is a pointer to the row data
 * @var packet This contains parsed data. This data type is a union.
 * Use apropriate type to extract correct information.
 */
struct item_data_t {
    uint32_t packet_type;
    uint32_t row_data_size;
    uint8_t* row_data;

    union packet_t {
        // Data could be parsed as these structures
        // Data structure to handle list and delete operations
        struct item_list_t {
            uint32_t  num_items;
            uint8_t** item_list; // Array of strings
        } item_list;

        // Data structure to handle upload and get
        struct item_info_t {
            uint32_t data_size;
            uint8_t* item_name; // This is assumed to be a string
            uint8_t* data;      // Pointer to the data
        } item_info;
    } packet_data;
};

/**
 * @brief It takes row data as parameter, and parses it into specific packet.
 * @param packet_type This indicates what kind of data this is.
 * @param data A pointer to the valid data.
 * @param size Size of data.
 * @return On success, it returns parsed packet data. On parse error, it returns NULL.
 */
struct item_data_t* parse_packet(uint32_t packet_type, uint8_t* data, uint32_t size);
#endif

