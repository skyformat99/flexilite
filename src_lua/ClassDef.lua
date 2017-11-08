---
--- Created by slan_ska.
--- DateTime: 2017-10-31 3:10 PM
---

--[[
Flexilite class (table) definition
Has reference to DBContext
Corresponds to [.classes] database table + D - decoded [data] field, with initialized PropertyRef's and NameRef's

Find property
Validates class structure
Loads class def from DB
Validates existing data with new class definition
]]

local PropertyDef = require('PropertyDef')
local NameRef = require('NameRef')

--[[

]]

---@class ClassDef
local ClassDef = {}

---@private
---@param self ClassDef
---@param json string @comment As it is stored in [.classes].Data
local function fromJSON(self, json)
    local dd = json.decode(json)

    self.Properties = {}

    --[[ This function can be called in 2 contexts:
     1) from raw JSON, during create/alter class/property
     2) from database saved
     in (1) nameOrId will be property name (string), property def will not have name or name id
     in (2) nameOrId will be property id (number), property def will have name and name id
    ]]
    for nameOrId, p in pairs(dd.properties) do
        if not self.Properties[p.ID] then
            local prop = PropertyDef:import(self, p)

            -- Determine mode
            if type(nameOrId) == 'number' and p.Prop.name and p.Prop.id then
                -- Database contexts
                self.Properties[nameOrId] = prop
                self.Properties[p.Prop.name] = prop
            else
                if type(nameOrId) ~= 'string' then
                    error('Invalid type of property name: ' .. nameOrId)
                end

                -- Raw JSON context
                prop.Prop.name = nameOrId
                self.Properties[nameOrId] = prop
            end
        end
    end

    ---@param dictName string
    function dictFromJSON(dictName)
        self[dictName] = {}
        local tt = dd[dictName]
        if tt then
            for k, v in pairs(tt) do
                self[dictName][k] = NameRef:fromJSON(self.DBContext, Bv)
            end
        end
    end

    dictToJSON('specialProperties')
    dictToJSON('rangeIndexing')
    dictToJSON('fullTextIndexing')
    dictToJSON('columnMapping')
end

--- Loads class definition from database
---@public
---@param DBContext DBContext
---@param obj table
--- (optional)
function ClassDef:loadFromDB (DBContext, obj)
    assert(obj)
    setmetatable(obj, self)
    self.__index = self
    obj.DBContext = DBContext
    obj:fromJSON(obj.Data)
    obj.Data = nil
    return obj
end

-- Initializes raw table (normally loaded from database) as ClassDef object
---@public
---@param DBContext DBContext
---@param json string
function ClassDef:fromJSON(DBContext, json)
    local obj = {
        DBContext = DBContext
    }
    setmetatable(obj, self)
    self.__index = self
    fromJSON(self, json)
    return obj
end

---@param DBContext DBContext
---@param jsonString string
---@return ClassDef
function ClassDef:fromJSONString(DBContext, jsonString)
    return self:fromJSON(DBContext, json.decode(jsonString))
end

function ClassDef:selfValidate()
    -- todo implement
end

function ClassDef:hasProperty(idOrName)
    return self.Properties[idOrName]
end

-- Internal function to add property to properties collection
---@param propDef PropertyDef
function ClassDef:addProperty(propDef)
    assert(propDef)
    assert(type(propDef.ID) == 'number')
    assert(type(propDef.Name) == 'string')
    self.Properties[propDef.ID] = propDef
    self.Properties[propDef.Name] = propDef
end

function ClassDef:getProperty(idOrName)
    -- Check if exists
    local prop = self.hasProperty(idOrName)
    if not prop then
        error( "Property " .. tostring(idOrName) .. " not found")
    end
    return prop
end

function ClassDef:validateData()
    -- todo
end

---@return table @comment User friendly encoded JSON of class definition (excluding raw and internal properties)
function ClassDef:toJSON()
    local result = {
        id = self.ID,
        name = self.Name,
        allowAnyProps = self.D.allowAnyProps,
    }

    ---@return nil
    function dictToJSON(dictName)
        local dict = self[dictName]
        if dict then
            result[dictName] = {}
            for ch, n in pairs(dict) do
                assert(n)
                result[dictName][ch] = n.toJSON()
            end
        end
    end

    for i, p in ipairs(self.Properties) do
        result[p.Name] = p.toJSON()
    end

    dictToJSON('specialProperties')
    dictToJSON('fullTextIndexing')
    dictToJSON('rangeIndexing')
    dictToJSON('columnMapping')

    return result
end

return ClassDef
