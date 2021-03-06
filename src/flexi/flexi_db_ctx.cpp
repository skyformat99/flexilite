//
// Created by slanska on 2017-02-16.
//

#include <stddef.h>
#include "flexi_db_ctx.h"
#include "flexi_class.h"
#include "../misc/regexp.h"

/*
 * Dummy no-op function. To be used as replacement for default free item
 * callback in hash table
 */
static void _dummy_ptr(void *ptr)
{
    UNUSED_PARAM(ptr);
}

static int
_RefValue_comparer(const RBNode *a, const RBNode *b, void *arg)
{
    UNUSED_PARAM(arg);

    flexi_RefValue_t *aNode = (void *) a;
    flexi_RefValue_t *bNode = (void *) b;
    sqlite3_int64 diff = aNode->lObjectID - bNode->lObjectID;
    if (diff == 0)
    {
        diff = aNode->lPropID - bNode->lPropID;
        if (diff == 0)
        {
            diff = aNode->lPropIndex - bNode->lPropIndex;
        }
    }

    if (diff < 0)
        return -1;

    if (diff > 0)
        return 1;

    return 0;
}

static void
_RefValue_combiner(RBNode *existing, const RBNode *newdata, void *arg)
{}

static RBNode *
_RefValue_alloc(void *arg)
{
    return sqlite3_malloc(sizeof(flexi_RefValue_t));
}

static void
_RefValue_free(RBNode *x, void *arg)
{
    sqlite3_free(x);
}

/*
 * Forward declarations
 */
struct flexi_Context_t *flexi_Context_new(sqlite3 *db)
{
    struct flexi_Context_t *result = sqlite3_malloc(sizeof(struct flexi_Context_t));
    if (!result)
        return NULL;
    memset(result, 0, sizeof(struct flexi_Context_t));
    result->db = db;
    HashTable_init(&result->classDefsByName, DICT_STRING, (void *) flexi_ClassDef_free);
    HashTable_init(&result->classDefsById, DICT_INT, _dummy_ptr);

    rb_create(&result->refValueCache, sizeof(flexi_RefValue_t),
              _RefValue_comparer, _RefValue_combiner, _RefValue_alloc, _RefValue_free, result);
    return result;
}

/*
 * Gets name ID by value. Name is expected to exist
 */
int flexi_Context_getNameId(struct flexi_Context_t *pCtx,
                            const char *zName, sqlite3_int64 *pNameID)
{
    int result;
    sqlite3_stmt *p;
    if (pNameID)
    {
        flexi_Context_stmtInit(pCtx, STMT_SEL_NAME_ID,
                               "select NameID from [.names] where [Value] = :1;",
                               &p);

        sqlite3_bind_text(p, 1, zName, -1, NULL);
        int stepRes = sqlite3_step(p);
        if (stepRes != SQLITE_ROW)
            return stepRes;

        *pNameID = sqlite3_column_int64(p, 0);
    }

    result = SQLITE_OK;
    goto EXIT;

    ONERROR:

    EXIT:
    return result;
}

int flexi_Context_getPropIdByClassIdAndName(struct flexi_Context_t *pCtx,
                                            sqlite3_int64 lClassID, const char *zPropName,
                                            sqlite3_int64 *plPropID)
{
    int result;
    sqlite3_stmt *pGetPropIDStmt;
    CHECK_CALL(flexi_Context_stmtInit(pCtx, STMT_SEL_PROP_ID_BY_NAME, "select ID from [.names_props] where "
                                              "PropNameID = (select ID from [.names_props] where [Value] = :1 limit 1)"
                                              " and ClassID = :2 limit 1;",
                                      &pGetPropIDStmt));
    sqlite3_bind_text(pGetPropIDStmt, 1, zPropName, -1, NULL);
    sqlite3_bind_int64(pGetPropIDStmt, 2, lClassID);
    CHECK_STMT_STEP(pGetPropIDStmt, pCtx->db);
    if (result == SQLITE_ROW)
        *plPropID = sqlite3_column_int64(pGetPropIDStmt, 0);
    else *plPropID = -1;

    result = SQLITE_OK;
    goto EXIT;

    ONERROR:

    EXIT:
    return result;
}

/*
 * Finds property ID by its class ID and name ID
 */
int flexi_Context_getPropIdByClassAndNameIds
        (struct flexi_Context_t *pCtx,
         sqlite3_int64 lClassID, sqlite3_int64 lPropNameID, sqlite3_int64 *plPropID)
{
    assert(plPropID);

    int result;
    sqlite3_stmt *p;
    CHECK_CALL(flexi_Context_stmtInit(
            pCtx, STMT_SEL_PROP_ID,
            "select PropertyID from [flexi_prop] where ClassID = :1 and NameID = :2;",
            &p));

    sqlite3_bind_int64(p, 1, lClassID);
    sqlite3_bind_int64(p, 2, lPropNameID);
    int stepRes = sqlite3_step(p);
    if (stepRes != SQLITE_ROW)
        return stepRes;

    *plPropID = sqlite3_column_int64(p, 0);

    result = SQLITE_OK;
    goto EXIT;

    ONERROR:

    EXIT:
    return result;
}

/*
 * Ensures that there is given Name in [.names] table.
 * Returns name id in pNameID (if not null)
 */
int flexi_Context_insertName(struct flexi_Context_t *pCtx, const char *zName, sqlite3_int64 *pNameID)
{
    int result;
    assert(zName);
    {
        sqlite3_stmt *p;
        const char *zInsNameSQL = "insert or replace into [.names] ([Value], NameID)"
                " values (:1, (select ID from [.names_props] where Value = :1 limit 1));";
        CHECK_CALL(flexi_Context_stmtInit(pCtx, STMT_INS_NAME, zInsNameSQL, &p));

        sqlite3_bind_text(p, 1, zName, -1, NULL);
        int stepRes = sqlite3_step(p);
        if (stepRes != SQLITE_DONE)
            return stepRes;
    }

    CHECK_CALL(flexi_Context_getNameId(pCtx, zName, pNameID));
    result = SQLITE_OK;
    goto EXIT;

    ONERROR:

    EXIT:
    return result;
}

static void
_freeMetadata(struct flexi_Context_t *pCtx)
{
    HashTable_clear(&pCtx->classDefsByName);
    HashTable_clear(&pCtx->classDefsById);
}

void flexi_Context_free(struct flexi_Context_t *pCtx)
{
    // Release prepared SQL statements
    for (int ii = 0; ii <= STMT_DEL_FTS; ii++)
    {
        if (pCtx->pStmts[ii])
        {
            sqlite3_finalize(pCtx->pStmts[ii]);
            pCtx->pStmts[ii] = NULL;
        }
    }

    if (pCtx->pMatchFuncSelStmt != NULL)
    {
        sqlite3_finalize(pCtx->pMatchFuncSelStmt);
        pCtx->pMatchFuncSelStmt = NULL;
    }

    if (pCtx->pMatchFuncInsStmt != NULL)
    {
        sqlite3_finalize(pCtx->pMatchFuncInsStmt);
        pCtx->pMatchFuncInsStmt = NULL;
    }

    if (pCtx->pMemDB != NULL)
    {
        sqlite3_close(pCtx->pMemDB);
        pCtx->pMemDB = NULL;
    }

    flexi_UserInfo_free(pCtx->pCurrentUser);

    _freeMetadata(pCtx);

    /*
     *TODO Check 2nd param
     */
    if (pCtx->pDuk)
        duk_free(pCtx->pDuk, NULL);

    sqlite3_free(pCtx->zLastErrorMessage);

    // TODO rb_delete(&pCtx->refValueCache);

    sqlite3_free(pCtx);
}

int flexi_Context_getClassIdByName(struct flexi_Context_t *pCtx,
                                   const char *zClassName, sqlite3_int64 *pClassID)
{
    assert(pCtx);

    int result;
    sqlite3_stmt *st;
    CHECK_CALL(flexi_Context_stmtInit(pCtx, STMT_CLS_ID_BY_NAME,
                                      "select ClassID from [.classes] "
                                              "where NameID = (select ID from [.names_props] where Value = :1 limit 1);",
                                      &st));
    CHECK_CALL(sqlite3_bind_text(st, 1, zClassName, -1, NULL));
    CHECK_STMT_STEP(st, pCtx->db);
    if (result == SQLITE_ROW)
    {
        *pClassID = sqlite3_column_int64(st, 0);
    }
    else
    {
        *pClassID = -1;
    }
    result = SQLITE_OK;

    goto EXIT;

    ONERROR:

    EXIT:
    return result;
}

/*
 * Initializes database connection wide SQL statements
 */
static int flexi_prepare_db_statements(struct flexi_Context_t *pCtx)
{
    int result;
    sqlite3 *db = pCtx->db;

    // TODO move to RTRee related code
    CHECK_STMT_PREPARE(
            db,
            "insert into [.range_data] ([ObjectID], [ClassID], [ClassID_1], "
                    "[A0], [_1], [B0], [B1], [C0], [C1], [D0], [D1]) values "
                    "(:1, :2, :2, :3, :4, :5, :6, :7, :8, :9, :10);",
            &pCtx->pStmts[STMT_INS_RTREE]);

    CHECK_STMT_PREPARE(
            db,
            "update [.range_data] set [ClassID] = :2, [ClassID_1] = :2, "
                    "[A0] = :3, [A1] = :4, [B0] = :5, [B1] = :6, "
                    "[C0] = :7, [C1] = :8, [D0] = :9, [D1] = :10 where ObjectID = :1;",
            &pCtx->pStmts[STMT_UPD_RTREE]);

    CHECK_STMT_PREPARE(
            db, "delete from [.range_data] where ObjectID = :1;",
            &pCtx->pStmts[STMT_DEL_RTREE]);

    goto EXIT;
    ONERROR:

    EXIT:
    return result;
}

static ReCompiled *pNameRegex = NULL;

static void NameRegex_free()
{
    re_free(pNameRegex);
}

bool db_validate_name(const char *zName)
{
    if (!zName)
        return false;

    const char *zNameRegex = "[_a-zA-Z][\\-_a-zA-Z0-9]{1,128}";
    if (!pNameRegex)
    {
        re_compile(&pNameRegex, zNameRegex, 1);
        atexit(NameRegex_free);
    }
    int result = re_match(pNameRegex, (const unsigned char *) zName, -1);
    return result != 0;
}

int flexi_Context_addClassDef(struct flexi_Context_t *self, flexi_ClassDef_t *pClassDef)
{
    int result;

    HashTable_set(&self->classDefsByName, (DictionaryKey_t) {.pKey = pClassDef->name.name}, pClassDef);
    HashTable_set(&self->classDefsById, (DictionaryKey_t) {.iKey = pClassDef->lClassID}, pClassDef);
    pClassDef->nRefCount++;

    result = SQLITE_OK;
    goto EXIT;

    ONERROR:

    EXIT:

    return result;
}

int flexi_Context_getClassByName(struct flexi_Context_t *self, const char *zClassName, flexi_ClassDef_t **ppClassDef)
{
    *ppClassDef = HashTable_get(&self->classDefsByName, (DictionaryKey_t) {.pKey = zClassName});
    return *ppClassDef != NULL;
}

int flexi_Context_getClassById(struct flexi_Context_t *self, sqlite3_int64 lClassId, flexi_ClassDef_t **ppClassDef)
{
    *ppClassDef = HashTable_get(&self->classDefsByName, (DictionaryKey_t) {.iKey = lClassId});
    return *ppClassDef != NULL;
}

int getColumnAsText(char **pzDest, sqlite3_stmt *pStmt, int iCol)
{
    *pzDest = NULL;
    char *zSrc = (char *) sqlite3_column_text(pStmt, iCol);
    if (zSrc == NULL)
        return SQLITE_OK;
    size_t len = strlen(zSrc);
    if (len == 0)
        return SQLITE_OK;

    *pzDest = sqlite3_malloc((int) len + 1);
    if (*pzDest == NULL)
        return SQLITE_NOMEM;
    strncpy(*pzDest, zSrc, len);
    (*pzDest)[len] = 0;

    return SQLITE_OK;
}

int String_copy(const char *zIn, char **pzOut)
{
    *pzOut = NULL;
    if (zIn == NULL)
        return SQLITE_OK;

    size_t sourceLen = strlen(zIn);
    if (sourceLen > 0)
    {
        *pzOut = sqlite3_malloc((int) (sourceLen + 1));
        if (*pzOut == NULL)
            return SQLITE_NOMEM;

        strncpy(*pzOut, zIn, sourceLen);
        (*pzOut)[sourceLen] = 0;
    }

    return SQLITE_OK;
}

char *String_substr(const char *zSource, intptr_t start, intptr_t len)
{
    if (len == 0)
        return NULL;

    size_t sourceLen = strlen(zSource);
    assert(start >= 0 && start + len < sourceLen);
    char *result = sqlite3_malloc((int) (len + 1));
    if (result == NULL)
        return NULL;

    strncpy(result, zSource + start, len);
    result[len] = 0;

    return result;
}

int flexi_Context_userVersion(struct flexi_Context_t *pCtx, sqlite3_int64 *plUserVersion, bool bIncrement)
{
    int result;

    char *zSetUserVersion = NULL;

    if (pCtx->pStmts[STMT_USER_VERSION_GET] == NULL)
    {
        CHECK_STMT_PREPARE(pCtx->db, "pragma user_version;", &pCtx->pStmts[STMT_USER_VERSION_GET]);
    }
    CHECK_STMT_STEP(pCtx->pStmts[STMT_USER_VERSION_GET], pCtx->db);
    if (result == SQLITE_ROW)
    {
        *plUserVersion = sqlite3_column_int64(pCtx->pStmts[STMT_USER_VERSION_GET], 0);

        if (true == bIncrement)
        {
            (*plUserVersion)++;
            zSetUserVersion = sqlite3_mprintf("pragma user_version=%" PRId64, plUserVersion);
            CHECK_CALL(sqlite3_exec(pCtx->db, zSetUserVersion, NULL, NULL, &pCtx->zLastErrorMessage));
        }

        result = SQLITE_OK;
    }

    goto EXIT;

    ONERROR:

    EXIT:
    sqlite3_free(zSetUserVersion);
    return result;
}

int flexi_Context_checkMetaDataCache(struct flexi_Context_t *pCtx)
{
    int result;

    sqlite3_int64 lNewUserVersion;
    CHECK_CALL(flexi_Context_userVersion(pCtx, &lNewUserVersion, false));
    if (lNewUserVersion != pCtx->lUserVersion)
    {
        _freeMetadata(pCtx);
        pCtx->lUserVersion = lNewUserVersion;
    }

    result = SQLITE_OK;
    goto EXIT;

    ONERROR:

    EXIT:

    return result;
}

void flexi_Context_setError(struct flexi_Context_t *pCtx, int iErrorCode, char *zErrorMessage)
{
    sqlite3_free(pCtx->zLastErrorMessage);
    pCtx->zLastErrorMessage = NULL;
    if (zErrorMessage != NULL)
    {
        pCtx->zLastErrorMessage = zErrorMessage;
    }
    else
    {
        pCtx->zLastErrorMessage = sqlite3_mprintf("DB error: %s", sqlite3_errmsg(pCtx->db));
    }
    pCtx->iLastErrorCode = iErrorCode;
}

int flexi_Context_getNameValueByID(struct flexi_Context_t *pCtx, sqlite3_int64 lNameID, char **pzName)
{
    int result;
    sqlite3_stmt *pStmt;
    flexi_Context_stmtInit(pCtx, STMT_GET_NAME_BY_ID, "select [Value] from [.names_props] where ID = :1 limit 1;",
                           &pStmt);
    sqlite3_bind_int64(pStmt, 1, lNameID);
    result = sqlite3_step(pStmt);
    if (result == SQLITE_ROW)
    {
        CHECK_CALL(getColumnAsText(pzName, pStmt, 1));
        result = SQLITE_OK;
    }
    else
        if (result == SQLITE_DONE)
        {
            result = SQLITE_NOTFOUND;
        }

    goto EXIT;

    ONERROR:

    EXIT:
    return result;
}

int flexi_Context_stmtInit(struct flexi_Context_t *pCtx, enum FLEXI_CTX_STMT stmt, const char *zSql,
                           sqlite3_stmt **pStmt)
{
    int result;
    if (pStmt != NULL)
        *pStmt = NULL;

    if (pCtx->pStmts[stmt] == NULL)
    {
        CHECK_STMT_PREPARE(pCtx->db, zSql, &pCtx->pStmts[stmt]);
    }
    else
    {
        CHECK_SQLITE(pCtx->db, sqlite3_reset(pCtx->pStmts[stmt]));
    }

    if (pStmt != NULL)
        *pStmt = pCtx->pStmts[stmt];
    result = SQLITE_OK;
    goto EXIT;

    ONERROR:

    EXIT:
    return result;
}

void flexi_config_func(sqlite3_context *context,
                       int argc,
                       sqlite3_value **argv)
{
    if (argc != 1 && argc != 2)
    {
        sqlite3_result_error(context, "Usage: flexi('config', name, value) or flexi('config', name)", -1);
        return;
    }

    // TODO current_user
    // TODO row_mode
}
