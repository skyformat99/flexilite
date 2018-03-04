---
--- Created by slanska.
--- DateTime: 2017-12-25 10:38 AM
---

local path = require 'pl.path'

-- set lua paths
package.path = path.abspath(path.relpath('../lib/lua-prettycjson/lib/resty/?.lua'))
.. ';' .. package.path

package.path = path.abspath(path.relpath('../src_lua/?.lua'))
.. ';' .. package.path

package.path = path.abspath(path.relpath('../lib/lua-sandbox/?.lua'))
.. ';' .. package.path

package.path = path.abspath(path.relpath('../lib/lua-schema/?.lua'))
.. ';' .. package.path

package.path = path.abspath(path.relpath('../lib/lua-date/?.lua'))
.. ';' .. package.path

package.path = path.abspath(path.relpath('../lib/lua-metalua/?.lua'))
.. ';' .. package.path

package.path = path.abspath(path.relpath('../lib/lua-metalua/compiler/?.lua'))
.. ';' .. package.path

package.path = path.abspath(path.relpath('../lib/lua-metalua/compiler/bytecode/?.lua'))
.. ';' .. package.path

package.path = path.abspath(path.relpath('../lib/lua-metalua/compiler/parser/?.lua'))
.. ';' .. package.path

package.path = path.abspath(path.relpath('../lib/lua-metalua/extension/?.lua'))
.. ';' .. package.path

package.path = path.abspath(path.relpath('../lib/lua-metalua/treequery/?.lua'))
.. ';' .. package.path

unpack = table.unpack

local mobdebug = require('mobdebug')
mobdebug.start()
