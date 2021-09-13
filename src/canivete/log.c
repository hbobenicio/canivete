#include "log.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

// posix
#include <strings.h> // strncasecmp

#include "error.h"

#define LOG_LEVEL_ENV_VAR "CANIVETE_LOG_LEVEL"
#define OFF_STR "off"
#define ERROR_STR "error"
#define WARN_STR "warn"
#define INFO_STR "info"
#define DEBUG_STR "debug"

static int canivete_log(struct canivete_logger* logger, enum canivete_log_level level, const char* fmt, va_list args);

struct canivete_logger canivete_log_default_logger(void)
{
    const char* level_str = getenv(LOG_LEVEL_ENV_VAR);
    if (level_str == NULL) {
        return (struct canivete_logger) {
            .stream = stderr,
            .level = CANIVETE_LOG_LEVEL_DEFAULT,
        };
    }
    enum canivete_log_level level;
    if (canivete_log_level_from_str(level_str, &level) != 0) {
        fprintf(stderr, "error: log: invalid log level. %s=\"%s\"", LOG_LEVEL_ENV_VAR, level_str);
        exit(EXIT_FAILURE);
    }
    return (struct canivete_logger) {
        .stream = stderr,
        .level = level,
    };
}

int canivete_log_level_from_str(const char* str, enum canivete_log_level* out_level)
{
    if (strncasecmp(OFF_STR, str, sizeof(OFF_STR)) == 0) {
        *out_level = CANIVETE_LOG_LEVEL_OFF;
        return CANIVETE_OK;
    }
    if (strncasecmp(ERROR_STR, str, sizeof(ERROR_STR)) == 0) {
        *out_level = CANIVETE_LOG_LEVEL_ERROR;
        return CANIVETE_OK;
    }
    if (strncasecmp(WARN_STR, str, sizeof(WARN_STR)) == 0) {
        *out_level = CANIVETE_LOG_LEVEL_WARN;
        return CANIVETE_OK;
    }
    if (strncasecmp(INFO_STR, str, sizeof(INFO_STR)) == 0) {
        *out_level = CANIVETE_LOG_LEVEL_INFO;
        return CANIVETE_OK;
    }
    if (strncasecmp(DEBUG_STR, str, sizeof(DEBUG_STR)) == 0) {
        *out_level = CANIVETE_LOG_LEVEL_DEBUG;
        return CANIVETE_OK;
    }
    return 1;
}

const char* canivete_log_str_from_level(enum canivete_log_level level)
{
    switch (level) {
    case CANIVETE_LOG_LEVEL_OFF:
        return OFF_STR;

    case CANIVETE_LOG_LEVEL_ERROR:
        return ERROR_STR;

    case CANIVETE_LOG_LEVEL_WARN:
        return WARN_STR;

    case CANIVETE_LOG_LEVEL_INFO:
        return INFO_STR;

    case CANIVETE_LOG_LEVEL_DEBUG:
        return DEBUG_STR;

    default:
        assert(0);
    }
}

int canivete_log_error(struct canivete_logger* logger, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int rc = canivete_log(logger, CANIVETE_LOG_LEVEL_ERROR, fmt, args);

    va_end(args);
    return rc;
}

int canivete_log_warn(struct canivete_logger* logger, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int rc = canivete_log(logger, CANIVETE_LOG_LEVEL_WARN, fmt, args);

    va_end(args);
    return rc;
}

int canivete_log_info(struct canivete_logger* logger, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int rc = canivete_log(logger, CANIVETE_LOG_LEVEL_INFO, fmt, args);

    va_end(args);
    return rc;
}

int canivete_log_debug(struct canivete_logger* logger, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int rc = canivete_log(logger, CANIVETE_LOG_LEVEL_DEBUG, fmt, args);

    va_end(args);
    return rc;
}

int canivete_log(struct canivete_logger* logger, enum canivete_log_level level, const char* fmt, va_list args)
{
    if (level > logger->level) {
        return 0;
    }

    time_t now = time(NULL);
    // char* now_str = asctime(localtime(&now));
    // char* now_str = ctime(&now);
    // now_str[strlen(now_str) - 1] = '\0';
    struct tm* t = localtime(&now);

    int rc = 0;
    // TODO is it better to write it all to a tmp buffer instead of making 3 I/O's (stderr is not even buffered commonly)
    rc += fprintf(logger->stream, "[%04d-%02d-%02d %02d:%02d:%02d] ", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    rc += fprintf(logger->stream, "[%s] ", canivete_log_str_from_level(level));
    rc += vfprintf(logger->stream, fmt, args);
    rc += fputc('\n', logger->stream);
    return rc;
}
