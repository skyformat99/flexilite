---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by slanska.
--- DateTime: 2018-04-15 11:00 PM
---

--[[ Busted tests for property value access and operations]]

local test_util = require 'util'
local DBQuery = require('QueryBuilder').DBQuery

-- In memory database
---@type DBContext
local DBContext = test_util.TestContext():GetNorthwind()

local test_cases = {
    { query = [[UnitPrice > 11 and UnitPrice < 21.1]], expected_cnt = 29 },
    { query = [[QuantityPerUnit >= '24 - 12 oz bottles' and QuantityPerUnit <= '24 - 12 oz bottles']], expected_cnt = 4 },
    { query = [[tostring(QuantityPerUnit) == '24 - 12 oz bottles']], expected_cnt = 4 },
    { query = [[UnitPrice / 2.0 > 11 and UnitPrice % 5 == 4]], expected_cnt = 3 },
    { query = [[QuantityPerUnit == '24 - 12 oz bottles']], expected_cnt = 4 },
    { query = [[ProductName == 'Camembert Pierrot']], expected_cnt = 1 },
    { query = [[1 == 1]], expected_cnt = 77 },
}

describe('Property Ops:', function()
    ---@type ClassDef
    local productsClassDef = assert(DBContext:getClassDef('Products'))

    local function run_test_case(n)
        it(test_cases[n].query, function()
            local qry = DBQuery(productsClassDef, test_cases[n].query)
            qry:Run()
            assert.are.equal(test_cases[n].expected_cnt, #qry.ObjectIDs)
        end)
    end

    for i, _ in ipairs(test_cases) do
        run_test_case(i)
    end
end)