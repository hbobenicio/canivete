#ifndef CANIVETE_LOG_H
#define CANIVETE_LOG_H

#include <stdio.h>

enum canivete_log_level {
    CANIVETE_LOG_LEVEL_OFF,
    CANIVETE_LOG_LEVEL_ERROR,
    CANIVETE_LOG_LEVEL_WARN,
    CANIVETE_LOG_LEVEL_INFO,
    CANIVETE_LOG_LEVEL_DEBUG,
};

#define CANIVETE_LOG_LEVEL_DEFAULT CANIVETE_LOG_LEVEL_INFO

struct canivete_logger {
    FILE* stream;
    enum canivete_log_level level;
};

struct canivete_logger canivete_log_default_logger(void);

int canivete_log_level_from_str(const char* str, enum canivete_log_level* out_level);
const char* canivete_log_str_from_level(enum canivete_log_level level);

int canivete_log_error(struct canivete_logger* logger, const char* fmt, ...);
int canivete_log_warn(struct canivete_logger* logger, const char* fmt, ...);
int canivete_log_info(struct canivete_logger* logger, const char* fmt, ...);
int canivete_log_debug(struct canivete_logger* logger, const char* fmt, ...);

#endif
