//
// Created by slanska on 2016-03-13.
//

#ifndef SQLITE_EXTENSIONS_MAIN_H
#define SQLITE_EXTENSIONS_MAIN_H

#include "../lib/sqlite/sqlite3ext.h"

SQLITE_EXTENSION_INIT1

#include <string.h>

int sqlite3_eval_init(
        sqlite3 *db,
        char **pzErrMsg,
        const sqlite3_api_routines *pApi
);

int sqlite3_fileio_init(
        sqlite3 *db,
        char **pzErrMsg,
        const sqlite3_api_routines *pApi
);

int sqlite3_regexp_init(
        sqlite3 *db,
        char **pzErrMsg,
        const sqlite3_api_routines *pApi
);

int sqlite3_totype_init(
        sqlite3 *db,
        char **pzErrMsg,
        const sqlite3_api_routines *pApi
);

int sqlite3_var_init(
        sqlite3 *db,
        char **pzErrMsg,
        const sqlite3_api_routines *pApi
);

int sqlite3_hash_init(
        sqlite3 *db,
        char **pzErrMsg,
        const sqlite3_api_routines *pApi
);

int sqlite3_flexi_get_init(
        sqlite3 *db,
        char **pzErrMsg,
        const sqlite3_api_routines *pApi
);

#endif //SQLITE_EXTENSIONS_MAIN_H
