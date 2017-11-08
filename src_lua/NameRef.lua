---
--- Created by slanska.
--- DateTime: 2017-10-31 3:20 PM
---

--[[
NameRef class.
Properties:
name
id
]]

---@class MetadataRef
local MetadataRef = {

    --- '==' operator for name refs
    ---@overload
    ---@param a MetadataRef
    ---@param b MetadataRef
    __eq = function(a, b)
        if getmetatable(a) ~= getmetatable(b) then
            return false
        end
        if a.id and b.id and a.id == b.id then
            return true
        end
        return a.name and b.name and a.name == b.name
    end,

    --- () method object
    ---@param self MetadataRef
    ---@return string @comment name value. So that. ref() -> ref.name
    __call = function (self)
        return self.name
    end
}

local NameRef = {}
setmetatable(NameRef, MetadataRef)

local ClassNameRef = {}
setmetatable(ClassNameRef, MetadataRef)
ClassNameRef.__index = ClassNameRef

local PropNameRef = {}
setmetatable(PropNameRef, MetadataRef)
PropNameRef._index = PropNameRef

--- Ensures that class with given name/id exists (uses classDef.DBContext
---@param classDef ClassDef
function NameRef:resolve(classDef)
    -- TODO create name
    if not self.id then
        self.id = classDef.DBContext:ensureName(self.name)
    end
end

---@param classDef ClassDef
function ClassNameRef:resolve(classDef)
    if self.id or self.name then
        local cc = classDef.DBContext:LoadClassDefinition(self.id and self.id or self.name)
        if cc ~= nil then
            self.id = cc.ClassID
        else
            self.id = nil
        end
    else
        error 'Neither name nor id are defined in class name reference'
    end
end

--- Ensures that class owner has given property (by name/id)
---@param classDef ClassDef
function PropNameRef:resolve(classDef)
    -- will throw error is property does not exist
    if not self.id then
        local pp = classDef:getProperty(self.name)
        self.id = pp.id
    end
end

---@param classDef ClassDef
function NameRef:isResolved(classDef)
    return self.id ~= nil
end

---@param classDef ClassDef
function ClassNameRef:isResolved(classDef)
    return type(self.id) == 'number'
end

---@param classDef ClassDef
function PropNameRef:isResolved(classDef)
    local pp = classDef.Properties[self.id and self.id or self.name]
    return pp ~= nil
end

---@return table
function NameRef:export()
    return self
end

return NameRef, ClassNameRef, PropNameRef