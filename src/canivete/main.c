#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

#include "canivete.h"
#include "error.h"
#include "log.h"
#include "db.h"

#ifndef IDENT
#define IDENT "    "
#endif

static void usage(const char* program_name);
static const char* shift_arg(int* argc, const char*** argv);

static void endpoint_usage(const char* program_name);
static int endpoint_cmd(struct canivete* app, const char* program_name, int* argc, const char*** argv);

static void endpoint_add_usage(const char* program_name);
static int endpoint_add_cmd(struct canivete* app, const char* program_name, int* argc, const char*** argv);
static int endpoint_ls_cmd(struct canivete* app, const char* program_name, int* argc, const char*** argv);

#include "str.h"

int main(int argc, const char* argv[])
{
    int rc;
    struct canivete app = {0};

    canivete_init_log(&app);

    // TODO this could probably be delayed to after cli parsing, but this logger is
    //      really handy at the very beginning!
    if ((rc = canivete_init(&app)) != CANIVETE_OK) {
        canivete_log_error(&app.logger, "initialization failed");
        goto err;
    }

    const char* program_name = shift_arg(&argc, &argv);
    if (argc == 0) {
        usage(program_name);
        goto end;
    }

    const char* cmd = shift_arg(&argc, &argv);

    if (strncmp("help", cmd, sizeof("help")) == 0) {
        usage(program_name);
        goto end;
    }

    if (strncmp("endpoint", cmd, sizeof("endpoint")) == 0) {
        if (endpoint_cmd(&app, program_name, &argc, &argv) != CANIVETE_OK) {
            goto err_app;
        }
        goto end;
    }

    canivete_log_error(&app.logger, "invalid command \"%s\"\n", cmd);
    usage(program_name);
    goto err_app;

    // if ((rc = canivete_get_server_certificates_chain(&app)) != CANIVETE_OK) {
    //     canivete_log_error(&app.logger, "could not get server's certificates chain");
    //     canivete_free(&app);
    //     return EXIT_FAILURE;
    // }

end:
    canivete_free(&app);
    return EXIT_SUCCESS;
err_app:
    canivete_free(&app);
err:
    return EXIT_FAILURE;
}

static void usage(const char* program_name)
{
    static const char* fmt =
        "usage:\n"
        IDENT "%s <cmd>\n\n"
        "cmds:\n"
        IDENT "endpoint: command responsible for endpoint related stuff\n\n"
    ;
    printf(fmt, program_name);
}

static const char* shift_arg(int* argc, const char*** argv)
{
    (*argc)--;
    const char* arg = (*argv)[0];
    (void) (*argv)++;
    return arg;
}

static void endpoint_usage(const char* program_name)
{
    static const char* fmt =
        "usage:\n"
        IDENT "%s endpoint <operation>\n\n"
        "operations:\n"
        IDENT "add: adds a new endpoint\n"
        IDENT "ls: lists all endpoints\n"
        IDENT "rm: removes an existing endpoint\n\n"
    ;
    printf(fmt, program_name);
}

static int endpoint_cmd(struct canivete* app, const char* program_name, int* argc, const char*** argv)
{
    if (*argc == 0) {
        endpoint_usage(program_name);
        return CANIVETE_ERR_CLI;
    }

    const char* operation = shift_arg(argc, argv);

    if (strcmp("add", operation) == 0) {
        return endpoint_add_cmd(app, program_name, argc, argv);
    } else if (strcmp("ls", operation) == 0) {
        return endpoint_ls_cmd(app, program_name, argc, argv);
    }

    canivete_log_error(&app->logger, "invalid endpoint operation \"%s\"\n", operation);
    endpoint_usage(program_name);
    return CANIVETE_ERR_CLI;
}

static void endpoint_add_usage(const char* program_name)
{
    static const char* fmt =
        "usage:\n"
        IDENT "%s endpoint add <host> <port>\n\n"
        "args:\n"
        IDENT "host: server host (ip | dns)\n"
        IDENT "port: server port\n\n"
    ;
    printf(fmt, program_name);
}

static int endpoint_add_cmd(struct canivete* app, const char* program_name, int* argc, const char*** argv)
{
    if (*argc != 2) {
        endpoint_add_usage(program_name);
        return CANIVETE_ERR_CLI;
    }

    const char* host_arg = shift_arg(argc, argv);
    const char* port_arg = shift_arg(argc, argv);

    // TODO improve safe convertion
    const char* host = host_arg;
    int port = atoi(port_arg);

    if (canivete_db_endpoints_insert(app, host, port) != CANIVETE_OK) {
        canivete_log_error(&app->logger, "failed to add new endpoint");
        return CANIVETE_ERR_CLI;
    }

    return CANIVETE_OK;
}

static int endpoint_ls_cmd(struct canivete* app, const char* program_name, int* argc, const char*** argv)
{
    (void) program_name;
    (void) argc;
    (void) argv;

    if (canivete_db_endpoints_list(app) != CANIVETE_OK) {
        canivete_log_error(&app->logger, "failed to list endpoints");
        return CANIVETE_ERR_CLI;
    }

    return CANIVETE_OK;
}
