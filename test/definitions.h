//
// Created by slanska on 2017-01-22.
//

#ifndef FLEXILITE_DEFINITIONS_H
#define FLEXILITE_DEFINITIONS_H

#include <stddef.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <cmocka.h>
#include <sqlite3.h>

#include "../src/common/common.h"
#include "Array.h"
#include "util/db_init.h"
#include "util/file_helper.h"
#include "../src/util/Path.h"

#ifdef __cplusplus
extern "C" {
#endif

int class_tests();

void run_sql_tests(char *zBaseDir, const char *zJsonFile);

/*
 * prop_tests();
 */

#ifdef __cplusplus
}
#endif

#endif //FLEXILITE_DEFINITIONS_H
