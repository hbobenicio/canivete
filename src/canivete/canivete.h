#ifndef CANIVETE_H
#define CANIVETE_H

#include <openssl/ssl.h>

#include "log.h"
#include "config.h"

struct canivete {
    struct canivete_logger logger;
    struct canivete_config config;
    SSL_CTX* ssl_ctx;
};

void canivete_init_log(struct canivete* ctx);
int canivete_init(struct canivete* ctx);
void canivete_free(struct canivete* ctx);

int canivete_get_server_certificates_chain(struct canivete* ctx);

#endif
