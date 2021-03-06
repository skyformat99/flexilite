---
--- Created by slanska.
--- DateTime: 2017-12-09 9:59 PM
---

local sqlite3 = require 'lsqlite3complete'
local base64 = require 'base64'
local JSON = require 'cjson'
local SQLiteSchemaParser = require 'sqliteSchemaParser'
local path = require 'pl.path'
local tablex = require 'pl.tablex'

-- set lua path
package.path = path.abspath(path.relpath('../lib/lua-prettycjson/lib/resty/?.lua'))
        .. ';' .. package.path

local prettyJson = require "prettycjson"
-- Dumps given table to out table
---@param out table
---@param db sqlite3
---@param classDef IClassDef
---@param tableName string
local function outTable(out, db, classDef, tableName)
    -- List of all table rows
    local rows = {}
    print(string.format('Processing [%s]...', tableName))

    -- get binary properties

    local blobProps = {}
    for propName, propDef in pairs(classDef.properties) do
        if propDef.rules.type == 'binary' then
            blobProps[propName] = propDef
        end
    end

    for row in db:nrows(string.format('select * from [%s]', tableName)) do
        for propName, blobProp in pairs(blobProps) do
            local v = row[propName]

            if v ~= nil then
                row[propName] = base64.encode(tostring(row[propName]))
            end
        end
        table.insert(rows, row)
    end
    out[tableName] = rows

    print(string.format('... %d rows processed', #rows))
end

--- Dumps database or given table to JSON
---@param dbPath string @comment Path to SQLite database file
---@param outFileName string @comment Path to output JSON file
---@param tableName string @comment Specific class or nil to dump all classes
---@param compactJson boolean @comment If true, compact JSON will be generated
local function dumpDatabase(dbPath, outFileName, tableName, compactJson)
    -- Open database
    local db, errMsg = sqlite3.open(dbPath)
    if not db then
        error(errMsg)
    end

    -- Load schema
    local sqliteParser = SQLiteSchemaParser(db)
    local schema = sqliteParser:parseSchema()

    -- Iterate through all non-system tables
    local result = {}

    if type(tableName) == 'string' and tableName ~= '' then
        local classDef = schema[tableName]
        if not classDef then
            error(string.format("Table [%s] is not found in database %s", tableName, dbPath))
        end

        outTable(result, db, classDef, tableName)
    else
        for tableName, classDef in pairs(schema) do
            outTable(result, db, classDef, tableName)
        end
    end

    -- Dump prettified JSON
    local dataJson
    if compactJson then
        dataJson = JSON.encode(result)
    else
        dataJson = prettyJson(result)
    end

    local f = io.open(outFileName, 'w')
    f:write(dataJson)
    f:close()
end

return dumpDatabase