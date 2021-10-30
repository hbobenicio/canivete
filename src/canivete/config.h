#ifndef CANIVETE_CLI_CONFIG_H
#define CANIVETE_CLI_CONFIG_H

#include <stdint.h>
#include <stddef.h>

#include "str.h"

struct canivete_logger;

#ifndef CANIVETE_CONFIG_SERVER_ADDRESS_HOST_CAPACITY
#define CANIVETE_CONFIG_SERVER_ADDRESS_HOST_CAPACITY 64
#endif

#ifndef CANIVETE_CONFIG_XDG_DIR_CAP
#define CANIVETE_CONFIG_XDG_DIR_CAP 4096
#endif

struct canivete_config_server_address {
    char host[CANIVETE_CONFIG_SERVER_ADDRESS_HOST_CAPACITY];
    size_t host_len;
    uint16_t port;
};

struct canivete_config_db {
    struct str filepath;
};

struct canivete_config {
    /// An arraylist of server addresses
    struct canivete_config_server_address* addresses;
    struct canivete_config_db db;
};

struct canivete_config canivete_config_from_environment(struct canivete_logger* logger);
void canivete_config_free(struct canivete_config* config);

#endif
