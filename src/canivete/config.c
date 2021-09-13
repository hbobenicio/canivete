#include "config.h"

#include <stdlib.h>

#include "log.h"

struct canivete_config canivete_config_from_environment(struct canivete_logger* logger)
{
    canivete_log_debug(logger, "config: loading configurations from environment variables");

    struct canivete_config cfg = {0};

    const char* db_filepath_env_key = "CANIVETE_DB_FILEPATH";
    const char* db_filepath_env_val = getenv(db_filepath_env_key);
    const char* db_filepath_cstr = (db_filepath_env_val == NULL) ? "canivete.db" : db_filepath_env_val;
    cfg.db.filepath = str_borrow_from_cstr(db_filepath_cstr);

    return cfg;
}
