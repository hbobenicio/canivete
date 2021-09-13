#ifndef CANIVETE_ERROR_H
#define CANIVETE_ERROR_H

typedef enum {
    CANIVETE_OK = 0,
    CANIVETE_ERR_OPENSSL,
    CANIVETE_ERR_CONFIG,
    CANIVETE_ERR_IO,
    CANIVETE_ERR_DB,
    CANIVETE_ERR_CLI,
} CaniveteOutcome;

#endif
