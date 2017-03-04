//
// Created by slanska on 2016-04-23.
//

/*
 * Implementation of class alteration
 */

#include "../project_defs.h"
#include "flexi_class.h"
#include "../util/List.h"

/*
 * Global mapping of type names between Flexilite and SQLite
 */
typedef struct
{
    const char *zFlexi_t;
    const char *zSqlite_t;
    int propType;
} FlexiTypesToSqliteTypeMap;

typedef struct _ClassAlterContext_t _ClassAlterContext_t;

typedef struct _ClassAlterAction_t
{
    /*
     * Class or property reference
     */
    flexi_metadata_ref *ref;

    int (*action)(_ClassAlterContext_t *pCtx, var args);

    void (*disposeSelf)();

    var args;
} _ClassAlterAction_t;

struct _ClassAlterContext_t
{
    struct flexi_class_def *pNewClassDef;
    struct flexi_class_def *pExistingClassDef;
    char **pzErr;

    /*
     * List of actions to be executed before scanning existing data
     */
    List_t preActions;

    /*
     * List of property level actions to be performed on every existing row during scanning
     */
    List_t postActions;

    /*
     * List of actions to be performed after scanning existing data
     */
    List_t propActions;
};

/*
 * Map between Flexilite property types (in text representation) to
 * SQLite column types and internal Flexilite types (in int representation)
 */
const static FlexiTypesToSqliteTypeMap g_FlexiTypesToSqliteTypes[] = {
        {"text",      "TEXT",     PROP_TYPE_TEXT},
        {"integer",   "INTEGER",  PROP_TYPE_INTEGER},
        {"boolean",   "INTEGER",  PROP_TYPE_BOOLEAN},
        {"enum",      "TEXT",     PROP_TYPE_ENUM},
        {"number",    "FLOAT",    PROP_TYPE_NUMBER},
        {"datetime",  "FLOAT",    PROP_TYPE_DATETIME},
        {"uuid",      "BLOB",     PROP_TYPE_UUID},
        {"binary",    "BLOB",     PROP_TYPE_BINARY},
        {"name",      "TEXT",     PROP_TYPE_NAME},
        {"decimal",   "FLOAT",    PROP_TYPE_DECIMAL},
        {"json",      "JSON1",    PROP_TYPE_JSON},
        {"date",      "FLOAT",    PROP_TYPE_DATETIME},
        {"time",      "FLOAT",    PROP_TYPE_TIMESPAN},
        {"any",       "",         PROP_TYPE_ANY},
        {"text",      "NVARCHAR", PROP_TYPE_TEXT},
        {"text",      "NCHAR",    PROP_TYPE_TEXT},
        {"decimal",   "MONEY",    PROP_TYPE_DECIMAL},
        {"binary",    "IMAGE",    PROP_TYPE_BINARY},
        {"text",      "VARCHAR",  PROP_TYPE_TEXT},
        {"reference", "",         PROP_TYPE_REF},
        {"enum",      "",         PROP_TYPE_ENUM},
        {"name",      "",         PROP_TYPE_NAME},
        {NULL,        "TEXT",     PROP_TYPE_TEXT}
};

static const FlexiTypesToSqliteTypeMap *_findFlexiType(const char *t)
{
    int ii = 0;
    for (; ii < ARRAY_LEN(g_FlexiTypesToSqliteTypes); ii++)
    {
        if (g_FlexiTypesToSqliteTypes[ii].zFlexi_t && strcmp(g_FlexiTypesToSqliteTypes[ii].zFlexi_t, t) == 0)
            return &g_FlexiTypesToSqliteTypes[ii];
    }

    return &g_FlexiTypesToSqliteTypes[ARRAY_LEN(g_FlexiTypesToSqliteTypes) - 1];
}

struct PropMergeParams_t
{
    struct flexi_class_def *pExistingClass;
    struct flexi_class_def *pNewClass;
    const char **pzErr;

    /*
     * Set by _processProp and _mergeClassSchemas to reflect type of alteration
     * if true, class definition is going to be shrinked, i.e. data validation/processing would be required
     */
    bool bValidateData;
};

/*
 * Allowed type transformations:
 * bool -> int -> decimal ->  number -> text -> name
 * date -> text
 * date -> number
 * number -> date
 * text -> date
 * int|number|text|date -> enum -> ref
 * int|number|text|date -> ref
 * text -> binary
 * binary -> text
 */

/*
 * Initializes enum property
 */
static int
_initEnumProp()
{
    // If property exists and defined as scalar prop, its current values will be treated
    // as uid/name/$id of enum item

    // Scan existing data, extract distinct values, find matching items in enum def.
    return 0;
}

/*
 * Initializes reference property
 */
static int
_initRefProp()
{
    // If property exists and defined as scalar prop, its current values will be treated as uid/$id/code/name
    // from the references class

    // Scan existing data and normalize it

    // Process reverse property
    return 0;
}

static int
_initNameProp()
{
    return 0;
}

/*
 * Mapping of upgrade and downgrade property types
 */
struct PropTypeTransitRule_t
{
    /*
     * Original property type
     */
    int type;

    /*
     * Can be upgraded without problem to these types (bitmask)
     */
    int yes;

    /*
     * May be OK to downgrade, but validation of existing data is needed
     */
    int maybe;

    bool rangeProp;

    bool fullTextProp;
};

static const struct PropTypeTransitRule_t g_propTypeTransitions[] =
        {
                {.type = PROP_TYPE_TEXT, .yes = PROP_TYPE_NAME | PROP_TYPE_REF | PROP_TYPE_BINARY | PROP_TYPE_JSON},
                {.type = PROP_TYPE_BOOLEAN, .yes = PROP_TYPE_INTEGER | PROP_TYPE_DECIMAL | PROP_TYPE_NUMBER |
                                                   PROP_TYPE_TEXT |
                                                   PROP_TYPE_ENUM},
                {.type = PROP_TYPE_INTEGER, .yes = PROP_TYPE_DECIMAL | PROP_TYPE_NUMBER | PROP_TYPE_TEXT |
                                                   PROP_TYPE_REF},
                {.type = PROP_TYPE_NUMBER, .yes = PROP_TYPE_TEXT, .maybe = PROP_TYPE_DECIMAL |
                                                                           PROP_TYPE_INTEGER},
                {.type = PROP_TYPE_ENUM, .yes = PROP_TYPE_TEXT | PROP_TYPE_NAME |
                                                PROP_TYPE_REF, .maybe = PROP_TYPE_INTEGER},
                {.type = PROP_TYPE_NAME, .yes = PROP_TYPE_TEXT | PROP_TYPE_REF, .maybe = PROP_TYPE_INTEGER |
                                                                                         PROP_TYPE_ENUM |
                                                                                         PROP_TYPE_NUMBER},
                {.type = PROP_TYPE_DECIMAL, .yes = PROP_TYPE_NUMBER | PROP_TYPE_TEXT, .maybe = PROP_TYPE_INTEGER},
                {.type = PROP_TYPE_DATE, .yes = PROP_TYPE_DATETIME | PROP_TYPE_TEXT},
                {.type = PROP_TYPE_DATETIME, .yes =PROP_TYPE_TEXT | PROP_TYPE_NUMBER | PROP_TYPE_DECIMAL},
                {.type = PROP_TYPE_BINARY, .yes = PROP_TYPE_TEXT, .maybe = PROP_TYPE_UUID},
                {.type = PROP_TYPE_TIMESPAN, .yes = PROP_TYPE_TEXT | PROP_TYPE_NUMBER, .maybe = PROP_TYPE_DECIMAL},
                {.type = PROP_TYPE_JSON, .yes = PROP_TYPE_TEXT | PROP_TYPE_REF, .maybe = PROP_TYPE_NUMBER},
                {.type = PROP_TYPE_UUID, .yes = PROP_TYPE_TEXT | PROP_TYPE_BINARY},
                {.type = PROP_TYPE_REF, .yes = PROP_TYPE_TEXT | PROP_TYPE_INTEGER | PROP_TYPE_DECIMAL},
                {.type = PROP_TYPE_ENUM, .yes = PROP_TYPE_TEXT | PROP_TYPE_INTEGER | PROP_TYPE_DECIMAL}
        };

static inline const struct PropTypeTransitRule_t *
_findPropTypeTransitRule(int propType)
{
    int i;
    const struct PropTypeTransitRule_t *result;
    result = &g_propTypeTransitions[0];
    for (i = 0; i < ARRAY_LEN(g_propTypeTransitions); i++, result++)
    {
        if (result->type == propType)
            return result;
    }

    return NULL;
}

/*
 * Compares 2 ref definitions. Returns
 */
static int
_compareRefDefs(struct flexi_ref_def *p1, struct flexi_ref_def *p2)
{
    return 0;
}

/*
 * Scans 2 sets of enum defs and checks if they are identical or not
 */
static int
_compareEnumDefs(const struct flexi_enum_def *p1, const struct flexi_enum_def *p2)
{
    return 0;
}

/*
 * At beginning of this function property can be in the following states:
 * ADDED, NON_MODIFIED (copied 'as-is' from existing class definition), MODIFIED or DELETED
 * Function:
 * a) checks property definition for consistency
 * b) determines it existing data should be validated
 * c) determines if property to be renamed
 * d) processes REF and ENUM types
 * e) tries its best to resolve class and property names by setting ID if possible. If resolving is impossible (class/property does not exist yet in the database)
 * it sets Unresolved flag in class def
 */
static void
_processProp(const char *zPropName, int index, struct flexi_prop_def *p,
             var pPropMap, struct PropMergeParams_t *pp, bool *bStop)
{

    UNUSED_PARAM(pPropMap);
    UNUSED_PARAM(index);

#define CHECK_ERROR(condition, errorMessage) \
            if (condition) \
    { \
        *pp->pzErr = errorMessage; \
        *bStop = true; \
        return; \
    }

    // Skip existing properties
    if (p->eChangeStatus == CHNG_STATUS_NOT_MODIFIED)
        return;

    // Check if class2 has the same property
    struct flexi_prop_def *pProp2;
    pProp2 = HashTable_get(&pp->pExistingClass->propMap, zPropName);

    if (pProp2)
    {
        if (p->zRenameTo)
        {
            CHECK_ERROR (!db_validate_name(p->zRenameTo),
                         sqlite3_mprintf("Invalid new property name [%s] in class [%s]",
                                         p->zRenameTo, *pp->pExistingClass->name.name));
        }

        // Check if old and new types are compatible
        if (p->type != pProp2->type && p->type != PROP_TYPE_ANY)
        {
            const struct PropTypeTransitRule_t *transitRule = _findPropTypeTransitRule(p->type);

            if (transitRule)
            {
                if ((transitRule->yes & p->type) != 0)
                {
                    // Transition is OK
                }
                else
                {
                    if ((transitRule->maybe & p->type) != 0)
                    {
                        // Transition is possible but data validation would be needed
                        pp->bValidateData = true;
                    }
                    else transitRule = NULL;
                }
            }

            CHECK_ERROR (!transitRule,
                         sqlite3_mprintf(
                                 "Transition from type %s (%d) to type %s (%d) is not supported ([%s].[%s])",
                                 pProp2->zType, pProp2->type, p->zType, p->type, pp->pNewClass->name.name,
                                 zPropName));
        }

        // For REF and ENUM property, check if refDef and enumDef, respectively, did not change
        if (p->type == pProp2->type)
        {
            switch (p->type)
            {
                case PROP_TYPE_REF:
                    //_compareRefDefs();
                    break;

                case PROP_TYPE_ENUM:
                    //                    _compareEnumDefs();
                    break;

                default:
                    break;
            }
        }
    }
    else
    {
        CHECK_ERROR (p->eChangeStatus == CHNG_STATUS_DELETED,
                     sqlite3_mprintf("Cannot drop non existing property '%s'", zPropName));

        CHECK_ERROR(p->zRenameTo,
                    sqlite3_mprintf("Cannot rename non existing property '%s'", zPropName));

        CHECK_ERROR(!db_validate_name(zPropName),
                    sqlite3_mprintf("Invalid property name [%s] in class [%s]",
                                    zPropName, *pp->pExistingClass->name.name));
    }

    // Validate property and initialize what's needed
    if (p->eChangeStatus != CHNG_STATUS_DELETED)
    {
        // Check consistency
        // type
        const FlexiTypesToSqliteTypeMap *typeMap = _findFlexiType(p->zType);
        CHECK_ERROR(!typeMap, sqlite3_mprintf("Unknown type \"%s\" for property [%s].[%s]", p->zType,
                                              pp->pNewClass->name.name, zPropName));

        // minValue & maxValue
        CHECK_ERROR (p->minValue > p->maxValue,
                     sqlite3_mprintf("Property [%s].[%s]: minValue must be less than or equal maxValue",
                                     pp->pNewClass->name.name,
                                     zPropName));

        // minOccurences & maxOccurences
        CHECK_ERROR(p->minOccurences < 0 || p->minOccurences > p->maxOccurences,
                    sqlite3_mprintf("Property [%s].[%s]: minOccurences must be between 0 and maxOccurrences",
                                    pp->pNewClass->name.name,
                                    zPropName));

        // maxLength
        CHECK_ERROR(p->maxLength < 0,
                    sqlite3_mprintf("Property [%s].[%s]: maxLength must be 0 or positive integer",
                                    pp->pNewClass->name.name,
                                    zPropName));

        // if ref, check refDef
        switch (typeMap->propType)
        {
            case PROP_TYPE_REF:
                _initRefProp();
                break;

            case PROP_TYPE_ENUM:
                _initEnumProp();
                break;

            case PROP_TYPE_NAME:
                _initNameProp();
                break;

            default:
                break;

        }

    }

#undef CHECK_ERROR
}

struct ValidateClassParams_t
{
    int nInvalidPropCount;
};

static void
_validateProp(const char *zPropName, int idx, struct flexi_prop_def *prop, var propMap,
              struct ValidateClassParams_t *params, bool *bStop)
{
    if (prop->bValidate)
        // Already invalid. Nothing to do
        return;


}

/*
 * Checks if any property has 'bValidate' flag.
 * Scans existing objects and checks property data if they match property definition.
 * If so, flag 'bValidate' will be cleared.
 * zValidationMode defines how validation process will deal with invalid data:
 * ABORT (default) - processing will be aborted and SQLITE_CONSTRAINT will be returned
 * IGNORE - process will stop on first found invalid data and SQLITE_CONSTRAINT will be returned
 * MARK - process will scan all existing data and add IDs of invalid objects to the [.invalid_objects] table
 * SQLITE_CONSTRAINT will be returned.
 *
 * if no invalid data was found, SQLITE_OK will be returned
 */
static int
_validateClassData(struct flexi_class_def *pClassDef, const char *zValidationMode, char **pzResult)
{
    int result;

    struct ValidateClassParams_t params = {};

    HashTable_each(&pClassDef->propMap, (void *) _validateProp, &params);


    return result;
}

/*
 * Clones property definition to a new class def if it is not defined in the new class def
 */
static void
_copyExistingProp(const char *zPropName, int idx, struct flexi_prop_def *prop, var propMap,
                  struct PropMergeParams_t *params, bool *bStop)
{
    UNUSED_PARAM(bStop);
    UNUSED_PARAM(idx);
    UNUSED_PARAM(propMap);

    struct flexi_prop_def *pNewProp = HashTable_get(&params->pNewClass->propMap, zPropName);
    if (!pNewProp)
    {
        HashTable_set(&params->pNewClass->propMap, zPropName, prop);
        prop->eChangeStatus = CHNG_STATUS_NOT_MODIFIED;
        prop->nRefCount++;
    }
    else
        if (pNewProp->eChangeStatus != CHNG_STATUS_DELETED)
            pNewProp->eChangeStatus = CHNG_STATUS_MODIFIED;
}

/*
 * Determines if special properties have changed (or were never initialized)
 * Validates properties based on their role(s)
 * Ensures that indexing is set whenever needed
 */
static int
_processSpecialProps(struct flexi_class_def *pExistingClass, struct flexi_class_def *pNewClass,
                     const char **pzErr)
{
    int result;

    if (!flexi_metadata_ref_compare_n(&pNewClass->aSpecProps[0], &pExistingClass->aSpecProps[0],
                                      ARRAY_LEN(pNewClass->aSpecProps)))
    {
        // Special properties have changed (or, a special case, were not set in existing class def)
        // Need to reset existing properties and set new ones (indexing, not null etc.)

        //pNewClass->aSpecProps[SPCL_PROP_UID] - unique, required
        //pNewClass->aSpecProps[SPCL_PROP_CODE] - unique, required
        //        pNewClass->aSpecProps[SPCL_PROP_NAME] - unique, required
        //        SPCL_PROP_UID = 0,
        //        SPCL_PROP_NAME = 1,
        //        SPCL_PROP_DESCRIPTION = 2,
        //        SPCL_PROP_CODE = 3,
        //        SPCL_PROP_NON_UNIQ_ID = 4,
        //        SPCL_PROP_CREATE_DATE = 5,
        //        SPCL_PROP_UPDATE_DATE = 6,
        //        SPCL_PROP_AUTO_UUID = 7,
        //        SPCL_PROP_AUTO_SHORT_ID = 8,

    }


    result = SQLITE_OK;
    goto FINALLY;

    CATCH:

    FINALLY:
    return result;
}

/*
 *
 */
static int
_findPropByMetadataRef(struct flexi_class_def *pClassDef, flexi_metadata_ref *pRef, struct flexi_prop_def **pProp)
{}

static int
_processRangeProps(struct flexi_class_def *pExistingClass, struct flexi_class_def *pNewClass,
                   const char **pzErr)
{
    int result;

    if (!flexi_metadata_ref_compare_n(&pNewClass->aRangeProps[0], &pExistingClass->aRangeProps[0],
                                      ARRAY_LEN(pNewClass->aRangeProps)))
    {
        // validate
        /*
         * properties should be any
         */
        // old range data will be removed
    }

    result = SQLITE_OK;
    goto FINALLY;

    CATCH:

    FINALLY:
    return result;
}

static int
_processFtsProps(struct flexi_class_def *pExistingClass, struct flexi_class_def *pNewClass,
                 const char **pzErr)
{
    int result;

    if (!flexi_metadata_ref_compare_n(&pNewClass->aFtsProps[0], &pExistingClass->aFtsProps[0],
                                      ARRAY_LEN(pNewClass->aFtsProps)))
    {
    }

    result = SQLITE_OK;
    goto FINALLY;

    CATCH:

    FINALLY:
    return result;
}

static int
_processMixins(struct flexi_class_def *pExistingClass, struct flexi_class_def *pNewClass,
               const char **pzErr)
{
    int result;

    /* Process mixins. If mixins are omitted in the new schema, existing definition of mixins will be used
 * if mixins are defined in both old and new schema, they will be compared for equality.
 * If not equal, data validation and processing needs to be performed
 *
 */
    if (!pNewClass->aMixins)
    {
        pNewClass->aMixins = pExistingClass->aMixins;
        pNewClass->aMixins->nRefCount++;
    }
    else
    {
        // Try to initialize classes

    }


    result = SQLITE_OK;
    goto FINALLY;

    CATCH:

    FINALLY:
    return result;
}

/*
 * Merges properties and other attributes in existing and new class definitions.
 * Validates property definition and marks them for removal/add/update/rename, if needed
 * Detects if existing class data needs to be validated/transformed, depending on property definition change
 * In case of any errors returns error code and sets pzErr to specific error message
 */
static int
_mergeClassSchemas(struct flexi_class_def *pExistingClass, struct flexi_class_def *pNewClass,
                   const char **pzErr)
{
    int result;
    struct PropMergeParams_t propMergeParams = {
            .pExistingClass = pExistingClass,
            .pNewClass = pNewClass,
            .pzErr = pzErr,
            .bValidateData = false
    };

    // Copy existing properties if they are not defined in new schema
    // These properties will get eChangeStatus NONMODIFIED
    HashTable_each(&pExistingClass->propMap, (void *) _copyExistingProp, &propMergeParams);
    if (*propMergeParams.pzErr)
        goto CATCH;

    // Iterate through properties. Find props: to be renamed, to be deleted, to be updated, to be added
    HashTable_each(&pNewClass->propMap, (void *) _processProp, &propMergeParams);
    if (*propMergeParams.pzErr)
        goto CATCH;

    // Process mixins
    CHECK_CALL(_processMixins(pExistingClass, pNewClass, pzErr));

    // Process special props
    CHECK_CALL(_processSpecialProps(pExistingClass, pNewClass, pzErr));

    // Process range props
    CHECK_CALL(_processRangeProps(pExistingClass, pNewClass, pzErr));

    // Process FTS props
    CHECK_CALL(_processFtsProps(pExistingClass, pNewClass, pzErr));

    result = SQLITE_OK;
    goto FINALLY;

    CATCH:
    result = SQLITE_ERROR;

    FINALLY:

    return result;
}

static int _createClassDefFromDefJSON(struct flexi_db_context *pCtx, const char *zClassDefJson,
                                      struct flexi_class_def **pClassDef)
{
    int result;
    const char *zErr = NULL;

    *pClassDef = flexi_class_def_new(pCtx);
    if (!*pClassDef)
    {
        result = SQLITE_NOMEM;
        goto CATCH;
    }

    CHECK_CALL(flexi_class_def_parse(*pClassDef, zClassDefJson, &zErr));

    result = SQLITE_OK;
    goto FINALLY;

    CATCH:

    FINALLY:
    return result;
}

/*
 *
 */
static int _applyClassSchema(struct flexi_class_def *pClassDef, const char **pzErr)
{
    int result;

    return result;
}

/*
 * Applies changes to the class that has data. Data validation/transformation may be required, depending on
 * nature of changes
 */
static int _alter_class_with_data(struct flexi_db_context *pCtx,
                                  sqlite3_int64 lClassID, const char *zNewClassDef,
                                  const char **pzError)
{
    int result;

    struct flexi_class_def *pNewClassDef = NULL;


    // load existing schema


    // Check if there are changes in full text data, range data, indexes, reference and enum properties

    // Merge existing and new definitions

    // Validate new schema

    // Detect if we 'shrink' schema. Means that existing data validation and transformation may be needed

    // If schema is not 'shrink', simply apply new schema

    CHECK_CALL(_applyClassSchema(pNewClassDef, pzError));


    goto FINALLY;

    CATCH:

    FINALLY:
    return result;
}

/*
 * Generic function to alter class definition
 * Performs all validations and necessary data updates
 */
int flexi_class_alter(struct flexi_db_context *pCtx,
                      const char *zClassName,
                      const char *zNewClassDefJson,
                      bool bCreateVTable,
                      const char **pzError
)
{
    int result;

    // Check if class exists. If no - error
    // Check if class does not exist yet
    sqlite3_int64 lClassID;
    CHECK_CALL(db_get_class_id_by_name(pCtx, zClassName, &lClassID));
    if (lClassID <= 0)
    {
        result = SQLITE_ERROR;
        *pzError = sqlite3_mprintf("Class [%s] is not found", zClassName);
        goto CATCH;
    }

    // Check if class has any objects created. If no - treat as create
    if (!pCtx->pStmts[STMT_CLS_HAS_DATA])
    {
        CHECK_CALL(sqlite3_prepare_v2(pCtx->db,
                                      "select 1 from [.objects] where ClassID = :1 and ObjectID > 0 limit 1;",
                                      -1, &pCtx->pStmts[STMT_CLS_HAS_DATA], NULL));
    }
    CHECK_CALL(sqlite3_reset(pCtx->pStmts[STMT_CLS_HAS_DATA]));
    CHECK_CALL(sqlite3_bind_int64(pCtx->pStmts[STMT_CLS_HAS_DATA], 0, lClassID));
    CHECK_STMT(sqlite3_step(pCtx->pStmts[STMT_CLS_HAS_DATA]));
    if (result == SQLITE_DONE)
    {
        CHECK_CALL(flexi_alter_new_class(pCtx, lClassID, zNewClassDefJson, NULL, NULL, pzError));
    }
    else
    {
        CHECK_CALL(_alter_class_with_data(pCtx, lClassID, zNewClassDefJson, pzError));
    }

    goto FINALLY;

    CATCH:
    if (!*pzError)
        *pzError = sqlite3_errstr(result);

    FINALLY:
    return result;
}

/*
 * Applies new definition to the class which does not yet have data
 */
int flexi_alter_new_class(struct flexi_db_context *pCtx, sqlite3_int64 lClassID, const char *zNewClassDef,
                          bool bCreateVTable, const char *zValidateMode, const char **pzErr)
{
    int result;

    assert(pCtx && pCtx->db);

    struct flexi_class_def *pNewClassDef = NULL;

    // Load existing class def
    struct flexi_class_def *pClassDef = NULL;
    CHECK_CALL(flexi_class_def_load(pCtx, lClassID, &pClassDef, pzErr));

    // Parse new definition
    CHECK_CALL(_createClassDefFromDefJSON(pCtx, zNewClassDef, &pNewClassDef));
    pNewClassDef->lClassID = lClassID;
    pNewClassDef->bAsTable = bCreateVTable;

    CHECK_CALL(_mergeClassSchemas(pClassDef, pNewClassDef, pzErr));

    CHECK_CALL(_applyClassSchema(pNewClassDef, pzErr));

    // Last step - replace existing class definition in pCtx

    goto FINALLY;

    CATCH:
    if (pClassDef)
        flexi_class_def_free(pClassDef);

    FINALLY:

    return result;
}
