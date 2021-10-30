#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "xdg.h"
#include "error.h"

struct canivete_config canivete_config_from_environment(struct canivete_logger* logger)
{
    canivete_log_debug(logger, "config: loading configurations from environment variables");

    struct canivete_config cfg = {0};

    // TODO improve error handling
    if (xdg_data_home_ensure_dir("canivete") != CANIVETE_OK) {
        canivete_log_error(logger, "config: failed to ensure the existance of the XDG config dir for this application");
        return cfg;
    }

    const char* canivete_db_suffix_file_path = "/canivete/canivete.db";
    char canivete_db_abs_file_path[XDG_MAX_PATH_SIZE] = {0};
    xdg_data_home_strncat(canivete_db_abs_file_path, canivete_db_suffix_file_path, XDG_MAX_PATH_SIZE - 1);
    const size_t canivete_db_abs_file_path_len = strlen(canivete_db_abs_file_path);
    cfg.db.filepath = (struct str) {
        .cstr = strdup(canivete_db_abs_file_path),
        .cap = canivete_db_abs_file_path_len + 1,
        .len =canivete_db_abs_file_path_len,
    };

    return cfg;
}


void canivete_config_free(struct canivete_config* config)
{
    str_free(&config->db.filepath);
}
