---
--- Created by slanska.
--- DateTime: 2017-12-19 7:12 AM
---

--[[
Internally used facade to [.object] row.
Provides access to property values, saving in database etc.
]]

local class = require 'pl.class'
local bits = type(jit) == 'table' and require('bit') or require('bit32')







--[[]]
---@class DBObject
local DBObject = class()

---@param classDef IClassDef
---@param objectId number @comment optional Int64
function DBObject:_init(classDef, objectId)
    self.classDef = classDef
    self.id = objectId

    -- [.ref-values] collection: table of table
    -- Each ref-value entry is stored in list
    -- as Value, ctlv, OriginalValue
    self.RV = {}
end

--- Set property value by name
function DBObject:setPropValueByName(propName, propIdx, value)
    local p = self.classDef:getProperty(propName)
    self:setPropertyByID(p.PropertyID, propIdx, value)
end

-- Returns ref-value entry for given property ID and index
function DBObject:getRefValue(propID, propIdx)
    local values = self.RV[propID]
    if not values then
        values = {}
        self.RV[propID] = values
    end

    local rv = values[propIdx]
    if not rv then
        rv = {}
        values[propIdx] = rv
    end

    return rv
end

function DBObject:deletePropertyByName(propName, propIdx)

end

function DBObject:deletePropertyByID(propID, propIdx)

end

--- Set property value by id
function DBObject:setPropertyByID(propID, propIdx, value)
    local rv = self:getRefValue(propID, propIdx)
    rv.Value = value
    rv.ctlv = bits.bor(rv.ctlv or 0, 1) -- TODO
end

function DBObject:setMappedPropertyValue(prop, value)
    -- TODO
end

--- Get property value by id

-- saveToDB

-- validate

-- loadFromDB

return DBObject