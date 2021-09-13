#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sqlite3.h>
#include <stb_ds.h>

#include "canivete.h"
#include "error.h"
#include "utils/file.h"

#define DB_LOG_TAG "db: "
#define DB_MIGRATE_LOG_TAG DB_LOG_TAG "migrate: "
#define DB_ENDPOINTS_LOG_TAG DB_LOG_TAG "endpoint: "

static bool select_migration_file(struct dirent* dir_entry)
{
    if (!select_regular_files(dir_entry)) return false;
    if (!select_min_file_len(dir_entry, 5)) return false;

    const size_t file_name_len = strlen(dir_entry->d_name);

    // ends with 'x.sql'. indexes are safe here because of the file_len check above
    char dot = dir_entry->d_name[file_name_len - 4];
    char s = dir_entry->d_name[file_name_len - 3];
    char q = dir_entry->d_name[file_name_len - 2];
    char l = dir_entry->d_name[file_name_len - 1];
    if (dot != '.' || s != 's' || q != 'q' || l != 'l') {
        return false;
    }

    return true;
}

int canivete_db_migrate(struct canivete* app)
{
    // TODO parametrizar estas configs
    static const char* migrations_dir = "db/migrations/";

    sqlite3* db = NULL;
    if (sqlite3_open_v2(app->config.db.filepath.cstr, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
        canivete_log_error(&app->logger, DB_MIGRATE_LOG_TAG "failed to open/create database: %s", sqlite3_errmsg(db));
        goto err;
    }

    struct str* migration_file_contents = NULL;
    if (file_read_files_from_dir(migrations_dir, select_migration_file, &migration_file_contents) != CANIVETE_OK) {
        canivete_log_error(&app->logger, DB_MIGRATE_LOG_TAG "failed to read migration files at dir \"%s\"", migrations_dir);
        goto err_db_close;
    }
    const size_t migration_file_contents_len = arrlen(migration_file_contents);

    // TODO exec sqlite migration commands
    // sqlite3_exec(db, "BEGIN TRANSACTION")

    for (size_t i = 0; i < migration_file_contents_len; i++) {
        str_free(&migration_file_contents[i]);
    }

    arrfree(migration_file_contents);
    assert(sqlite3_close(db) == SQLITE_OK);
    return CANIVETE_OK;

err_db_close:
    assert(sqlite3_close(db) == SQLITE_OK);
err:
    return CANIVETE_ERR_DB;
}

int canivete_db_endpoints_insert(struct canivete* app, const char* host, int port)
{
    sqlite3* db = NULL;
    if (sqlite3_open_v2(app->config.db.filepath.cstr, &db, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK) {
        canivete_log_error(&app->logger, DB_ENDPOINTS_LOG_TAG "failed to open/create database: %s", sqlite3_errmsg(db));
        goto err;
    }

    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, "INSERT INTO endpoints (host, port) VALUES (?, ?);", -1, &stmt, NULL) != SQLITE_OK) {
        canivete_log_error(&app->logger, DB_ENDPOINTS_LOG_TAG "failed to prepare statement for insertion: %s", sqlite3_errmsg(db));
        goto err_db_close;
    }

    if (sqlite3_bind_text(stmt, 1, host, -1, NULL) != SQLITE_OK) {
        canivete_log_error(&app->logger, DB_ENDPOINTS_LOG_TAG "failed to bind sql text param 'host' for insertion statement: %s", sqlite3_errmsg(db));
        goto err_stmt_finalize;
    }

    if (sqlite3_bind_int(stmt, 2, port) != SQLITE_OK) {
        canivete_log_error(&app->logger, DB_ENDPOINTS_LOG_TAG "failed to bind sql int param 'port' for insertion statement: %s", sqlite3_errmsg(db));
        goto err_stmt_finalize;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        canivete_log_error(&app->logger, DB_ENDPOINTS_LOG_TAG "failed to exec insert statement: %s", sqlite3_errmsg(db));
        goto err_stmt_finalize;
    }

    canivete_log_info(&app->logger, DB_ENDPOINTS_LOG_TAG "new endpoint saved successfully. addr='%s:%d'", host, port);
    assert(sqlite3_finalize(stmt) == SQLITE_OK);
    assert(sqlite3_close(db) == SQLITE_OK);
    return CANIVETE_OK;

err_stmt_finalize:
    // If the most recent evaluation of statement S failed, then
    // sqlite3_finalize(S) returns the appropriate [error code] or [extended error code]
    assert(sqlite3_finalize(stmt) != SQLITE_OK);
err_db_close:
    assert(sqlite3_close(db) == SQLITE_OK);
err:
    return CANIVETE_ERR_DB;
}

static int on_row_endpoints_list(void* userdata, int ncols, char** cols, char** colnames)
{
    (void) userdata;

    assert(ncols == 3);
    assert(strcmp("id", colnames[0]) == 0);
    assert(strcmp("host", colnames[1]) == 0);
    assert(strcmp("port", colnames[2]) == 0);

    const char* id_col = cols[0];
    const char* host_col = cols[1];
    const char* port_col = cols[2];
    int port = atoi(port_col);

    printf("endpoint{id=%s, host=%s, port=%d}\n", id_col, host_col, port);

    return 0;
}

int canivete_db_endpoints_list(struct canivete* app)
{
    sqlite3* db = NULL;
    if (sqlite3_open_v2(app->config.db.filepath.cstr, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
        canivete_log_error(&app->logger, DB_ENDPOINTS_LOG_TAG "failed to open/create database: %s", sqlite3_errmsg(db));
        goto err;
    }

    char* errmsg = NULL;
    if (sqlite3_exec(db, "SELECT * FROM endpoints", on_row_endpoints_list, app, &errmsg) != SQLITE_OK) {
        canivete_log_error(&app->logger, DB_ENDPOINTS_LOG_TAG "failed to open/create database: %s", errmsg);
        sqlite3_free(errmsg);
        goto err_db_close;
    }

    assert(sqlite3_close(db) == SQLITE_OK);
    return CANIVETE_OK;

err_db_close:
    assert(sqlite3_close(db) == SQLITE_OK);
err:
    return CANIVETE_ERR_DB;
}
