--
-- lua-MessagePack : <http://fperrad.github.io/lua-MessagePack/>
--

local assert = assert
local error = error
local pairs = pairs
local pcall = pcall
local setmetatable = setmetatable
local tostring = tostring
local type = type
local char = string.char
local math_type = math.type
local tointeger = math.tointeger
local tconcat = table.concat
local pack = string.pack
local unpack = string.unpack

--[[ debug only
local format = require'string'.format
local function hexadump (s)
    return (s:gsub('.', function (c) return format('%02X ', c:byte()) end))
end
--]]

local m = {}
_G.msgpack = m

local _ENV = nil

--[[ debug only
m.hexadump = hexadump
--]]

local function argerror (caller, narg, extramsg)
    error("bad argument #" .. tostring(narg) .. " to "
          .. caller .. " (" .. extramsg .. ")")
end

local function typeerror (caller, narg, arg, tname)
    argerror(caller, narg, tname .. " expected, got " .. type(arg))
end

local function checktype (caller, narg, arg, tname)
    if type(arg) ~= tname then
        typeerror(caller, narg, arg, tname)
    end
end

local packers = setmetatable({}, {
    __index = function (t, k) error("pack '" .. k .. "' is unimplemented") end
})
m.packers = packers

packers['nil'] = function (buffer)
    buffer[#buffer+1] = char(0xC0)              -- nil
end

packers['boolean'] = function (buffer, bool)
    if bool then
        buffer[#buffer+1] = char(0xC3)          -- true
    else
        buffer[#buffer+1] = char(0xC2)          -- false
    end
end

packers['string_compat'] = function (buffer, str)
    local n = #str
    if n <= 0x1F then
        buffer[#buffer+1] = char(0xA0 + n)      -- fixstr
    elseif n <= 0xFFFF then
        buffer[#buffer+1] = char(0xDA)          -- str16
        buffer[#buffer+1] = pack('>I2', n)
    elseif n <= 0xFFFFFFFF.0 then
        buffer[#buffer+1] = char(0xDB)          -- str32
        buffer[#buffer+1] = pack('>I4', n)
    else
        error"overflow in pack 'string_compat'"
    end
    buffer[#buffer+1] = str
end

packers['_string'] = function (buffer, str)
    local n = #str
    if n <= 0x1F then
        buffer[#buffer+1] = char(0xA0 + n)      -- fixstr
    elseif n <= 0xFF then
        buffer[#buffer+1] = char(0xD9,          -- str8
                                 n)
    elseif n <= 0xFFFF then
        buffer[#buffer+1] = char(0xDA)          -- str16
        buffer[#buffer+1] = pack('>I2', n)
    elseif n <= 0xFFFFFFFF.0 then
        buffer[#buffer+1] = char(0xDB)          -- str32
        buffer[#buffer+1] = pack('>I4', n)
    else
        error"overflow in pack 'string'"
    end
    buffer[#buffer+1] = str
end

packers['binary'] = function (buffer, str)
    local n = #str
    if n <= 0xFF then
        buffer[#buffer+1] = char(0xC4,          -- bin8
                                 n)
    elseif n <= 0xFFFF then
        buffer[#buffer+1] = char(0xC5)          -- bin16
        buffer[#buffer+1] = pack('>I2', n)
    elseif n <= 0xFFFFFFFF.0 then
        buffer[#buffer+1] = char(0xC6)          -- bin32
        buffer[#buffer+1] = pack('>I4', n)
    else
        error"overflow in pack 'binary'"
    end
    buffer[#buffer+1] = str
end

local set_string = function (str)
    if str == 'string_compat' then
        packers['string'] = packers['string_compat']
    elseif str == 'string' then
        packers['string'] = packers['_string']
    elseif str == 'binary' then
        packers['string'] = packers['binary']
    else
        argerror('set_string', 1, "invalid option '" .. str .."'")
    end
end
m.set_string = set_string

packers['map'] = function (buffer, tbl, n)
    if n <= 0x0F then
        buffer[#buffer+1] = char(0x80 + n)      -- fixmap
    elseif n <= 0xFFFF then
        buffer[#buffer+1] = char(0xDE)          -- map16
        buffer[#buffer+1] = pack('>I2', n)
    elseif n <= 0xFFFFFFFF.0 then
        buffer[#buffer+1] = char(0xDF)          -- map32
        buffer[#buffer+1] = pack('>I4', n)
    else
        error"overflow in pack 'map'"
    end
    for k, v in pairs(tbl) do
        packers[type(k)](buffer, k)
        packers[type(v)](buffer, v)
    end
end

packers['array'] = function (buffer, tbl, n)
    if n <= 0x0F then
        buffer[#buffer+1] = char(0x90 + n)      -- fixarray
    elseif n <= 0xFFFF then
        buffer[#buffer+1] = char(0xDC)          -- array16
        buffer[#buffer+1] = pack('>I2', n)
    elseif n <= 0xFFFFFFFF.0 then
        buffer[#buffer+1] = char(0xDD)          -- array32
        buffer[#buffer+1] = pack('>I4', n)
    else
        error"overflow in pack 'array'"
    end
    for i = 1, n do
        local v = tbl[i]
        packers[type(v)](buffer, v)
    end
end

local set_array = function (array)
    if array == 'without_hole' then
        packers['_table'] = function (buffer, tbl)
            local is_map, n, max = false, 0, 0
            for k in pairs(tbl) do
                if type(k) == 'number' and k > 0 then
                    if k > max then
                        max = k
                    end
                else
                    is_map = true
                end
                n = n + 1
            end
            if max ~= n then    -- there are holes
                is_map = true
            end
            if is_map then
                return packers['map'](buffer, tbl, n)
            else
                return packers['array'](buffer, tbl, n)
            end
        end
    elseif array == 'with_hole' then
        packers['_table'] = function (buffer, tbl)
            local is_map, n, max = false, 0, 0
            for k in pairs(tbl) do
                if type(k) == 'number' and k > 0 then
                    if k > max then
                        max = k
                    end
                else
                    is_map = true
                end
                n = n + 1
            end
            if is_map then
                return packers['map'](buffer, tbl, n)
            else
                return packers['array'](buffer, tbl, max)
            end
        end
    elseif array == 'always_as_map' then
        packers['_table'] = function(buffer, tbl)
            local n = 0
            for k in pairs(tbl) do
                n = n + 1
            end
            return packers['map'](buffer, tbl, n)
        end
    else
        argerror('set_array', 1, "invalid option '" .. array .."'")
    end
end
m.set_array = set_array

packers['table'] = function (buffer, tbl)
    return packers['_table'](buffer, tbl)
end

packers['unsigned'] = function (buffer, n)
    if n >= 0 then
        if n <= 0x7F then
            buffer[#buffer+1] = char(n)         -- fixnum_pos
        elseif n <= 0xFF then
            buffer[#buffer+1] = char(0xCC,      -- uint8
                                     n)
        elseif n <= 0xFFFF then
            buffer[#buffer+1] = char(0xCD)      -- uint16
            buffer[#buffer+1] = pack('>I2', n)
        elseif n <= 0xFFFFFFFF.0 then
            buffer[#buffer+1] = char(0xCE)      -- uint32
            buffer[#buffer+1] = pack('>I4', n)
        else
            buffer[#buffer+1] = char(0xCF)      -- uint64
            buffer[#buffer+1] = pack('>I8', n)
        end
    else
        if n >= -0x20 then
            buffer[#buffer+1] = char(0x100 + n) -- fixnum_neg
        elseif n >= -0x80 then
            buffer[#buffer+1] = char(0xD0)      -- int8
            buffer[#buffer+1] = pack('>i1', n)
        elseif n >= -0x8000 then
            buffer[#buffer+1] = char(0xD1)      -- int16
            buffer[#buffer+1] = pack('>i2', n)
        elseif n >= -0x80000000 then
            buffer[#buffer+1] = char(0xD2)      -- int32
            buffer[#buffer+1] = pack('>i4', n)
        else
            buffer[#buffer+1] = char(0xD3)      -- int64
            buffer[#buffer+1] = pack('>i8', n)
        end
    end
end

packers['signed'] = function (buffer, n)
    if n >= 0 then
        if n <= 0x7F then
            buffer[#buffer+1] = char(n)         -- fixnum_pos
        elseif n <= 0x7FFF then
            buffer[#buffer+1] = char(0xD1)      -- int16
            buffer[#buffer+1] = pack('>i2', n)
        elseif n <= 0x7FFFFFFF then
            buffer[#buffer+1] = char(0xD2)      -- int32
            buffer[#buffer+1] = pack('>i4', n)
        else
            buffer[#buffer+1] = char(0xD3)      -- int64
            buffer[#buffer+1] = pack('>i8', n)
        end
    else
        if n >= -0x20 then
            buffer[#buffer+1] = char(0xE0 + 0x20 + n)   -- fixnum_neg
        elseif n >= -0x80 then
            buffer[#buffer+1] = char(0xD0)      -- int8
            buffer[#buffer+1] = pack('>i1', n)
        elseif n >= -0x8000 then
            buffer[#buffer+1] = char(0xD1)      -- int16
            buffer[#buffer+1] = pack('>i2', n)
        elseif n >= -0x80000000 then
            buffer[#buffer+1] = char(0xD2)      -- int32
            buffer[#buffer+1] = pack('>i4', n)
        else
            buffer[#buffer+1] = char(0xD3)      -- int64
            buffer[#buffer+1] = pack('>i8', n)
        end
    end
end

local set_integer = function (integer)
    if integer == 'unsigned' then
        packers['integer'] = packers['unsigned']
    elseif integer == 'signed' then
        packers['integer'] = packers['signed']
    else
        argerror('set_integer', 1, "invalid option '" .. integer .."'")
    end
end
m.set_integer = set_integer

packers['float'] = function (buffer, n)
    buffer[#buffer+1] = char(0xCA)
    buffer[#buffer+1] = pack('>f', n)
end

packers['double'] = function (buffer, n)
    buffer[#buffer+1] = char(0xCB)
    buffer[#buffer+1] = pack('>d', n)
end

local set_number = function (number)
    if number == 'integer' then
        packers['number'] = packers['signed']
    elseif number == 'float' then
        packers['number'] = function (buffer, n)
            if math_type(n) == 'float' then
                return packers['float'](buffer, n)
            else
                return packers['integer'](buffer, n)
            end
        end
    elseif number == 'double' then
        packers['number'] = function (buffer, n)
            if math_type(n) == 'float' then
                return packers['double'](buffer, n)
            else
                return packers['integer'](buffer, n)
            end
        end
    else
        argerror('set_number', 1, "invalid option '" .. number .."'")
    end
end
m.set_number = set_number

for k = 0, 4 do
    local n = tointeger(2^k)
    local fixext = 0xD4 + k
    packers['fixext' .. tostring(n)] = function (buffer, tag, data)
        assert(#data == n, "bad length for fixext" .. tostring(n))
        buffer[#buffer+1] = char(fixext)
        buffer[#buffer+1] = pack('>i1', tag)
        buffer[#buffer+1] = data
    end
end

packers['ext'] = function (buffer, tag, data)
    local n = #data
    if n <= 0xFF then
        buffer[#buffer+1] = char(0xC7,          -- ext8
                                 n)
        buffer[#buffer+1] = pack('>i1', tag)
    elseif n <= 0xFFFF then
        buffer[#buffer+1] = char(0xC8)          -- ext16
        buffer[#buffer+1] = pack('>I2', n)
        buffer[#buffer+1] = pack('>i1', tag)
    elseif n <= 0xFFFFFFFF.0 then
        buffer[#buffer+1] = char(0xC9)          -- ext32
        buffer[#buffer+1] = pack('>I4', n)
        buffer[#buffer+1] = pack('>i1', tag)
    else
        error"overflow in pack 'ext'"
    end
    buffer[#buffer+1] = data
end

function m.pack (data)
    local buffer = {}
    packers[type(data)](buffer, data)
    return tconcat(buffer)
end


local types_map = setmetatable({
    [0xC0] = 'nil',
    [0xC2] = 'false',
    [0xC3] = 'true',
    [0xC4] = 'bin8',
    [0xC5] = 'bin16',
    [0xC6] = 'bin32',
    [0xC7] = 'ext8',
    [0xC8] = 'ext16',
    [0xC9] = 'ext32',
    [0xCA] = 'float',
    [0xCB] = 'double',
    [0xCC] = 'uint8',
    [0xCD] = 'uint16',
    [0xCE] = 'uint32',
    [0xCF] = 'uint64',
    [0xD0] = 'int8',
    [0xD1] = 'int16',
    [0xD2] = 'int32',
    [0xD3] = 'int64',
    [0xD4] = 'fixext1',
    [0xD5] = 'fixext2',
    [0xD6] = 'fixext4',
    [0xD7] = 'fixext8',
    [0xD8] = 'fixext16',
    [0xD9] = 'str8',
    [0xDA] = 'str16',
    [0xDB] = 'str32',
    [0xDC] = 'array16',
    [0xDD] = 'array32',
    [0xDE] = 'map16',
    [0xDF] = 'map32',
}, { __index = function (t, k)
        if k < 0xC0 then
            if k < 0x80 then
                return 'fixnum_pos'
            elseif k < 0x90 then
                return 'fixmap'
            elseif k < 0xA0 then
                return 'fixarray'
            else
                return 'fixstr'
            end
        elseif k > 0xDF then
            return 'fixnum_neg'
        else
            return 'reserved' .. tostring(k)
        end
end })
m.types_map = types_map

local unpackers = setmetatable({}, {
    __index = function (t, k) error("unpack '" .. k .. "' is unimplemented") end
})
m.unpackers = unpackers

local function unpack_array (c, n)
    local t = {}
    local decode = unpackers['any']
    for i = 1, n do
        t[i] = decode(c)
    end
    return t
end

local function unpack_map (c, n)
    local t = {}
    local decode = unpackers['any']
    for i = 1, n do
        local k = decode(c)
        local val = decode(c)
        if k == nil then
            k = m.sentinel
        end
        if k ~= nil then
            t[k] = val
        end
    end
    return t
end

unpackers['any'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i > j then
        c:underflow(i)
        s, i, j = c.s, c.i, c.j
    end
    local val = s:sub(i, i):byte()
    c.i = i+1
    return unpackers[types_map[val]](c, val)
end

unpackers['nil'] = function ()
    return nil
end

unpackers['false'] = function ()
    return false
end

unpackers['true'] = function ()
    return true
end

unpackers['float'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+3 > j then
        c:underflow(i+3)
        s, i, j = c.s, c.i, c.j
    end
    c.i = i+4
    return unpack('>f', s, i)
end

unpackers['double'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+7 > j then
        c:underflow(i+7)
        s, i, j = c.s, c.i, c.j
    end
    c.i = i+8
    return unpack('>d', s, i)
end

unpackers['fixnum_pos'] = function (c, val)
    return val
end

unpackers['uint8'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i > j then
        c:underflow(i)
        s, i, j = c.s, c.i, c.j
    end
    c.i = i+1
    return unpack('>I1', s, i)
end

unpackers['uint16'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+1 > j then
        c:underflow(i+1)
        s, i, j = c.s, c.i, c.j
    end
    c.i = i+2
    return unpack('>I2', s, i)
end

unpackers['uint32'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+3 > j then
        c:underflow(i+3)
        s, i, j = c.s, c.i, c.j
    end
    c.i = i+4
    return unpack('>I4', s, i)
end

unpackers['uint64'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+7 > j then
        c:underflow(i+7)
        s, i, j = c.s, c.i, c.j
    end
    c.i = i+8
    return unpack('>I8', s, i)
end

unpackers['fixnum_neg'] = function (c, val)
    return val - 0x100
end

unpackers['int8'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i > j then
        c:underflow(i)
        s, i, j = c.s, c.i, c.j
    end
    c.i = i+1
    return unpack('>i1', s, i)
end

unpackers['int16'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+1 > j then
        c:underflow(i+1)
        s, i, j = c.s, c.i, c.j
    end
    c.i = i+2
    return unpack('>i2', s, i)
end

unpackers['int32'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+3 > j then
        c:underflow(i+3)
        s, i, j = c.s, c.i, c.j
    end
    c.i = i+4
    return unpack('>i4', s, i)
end

unpackers['int64'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+7 > j then
        c:underflow(i+7)
        s, i, j = c.s, c.i, c.j
    end
    c.i = i+8
    return unpack('>i8', s, i)
end

unpackers['fixstr'] = function (c, val)
    local s, i, j = c.s, c.i, c.j
    local n = val & 0x1F
    local e = i+n-1
    if e > j then
        c:underflow(e)
        s, i, j = c.s, c.i, c.j
        e = i+n-1
    end
    c.i = i+n
    return s:sub(i, e)
end

unpackers['str8'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i > j then
        c:underflow(i)
        s, i, j = c.s, c.i, c.j
    end
    local n = unpack('>I1', s, i)
    i = i+1
    c.i = i
    local e = i+n-1
    if e > j then
        c:underflow(e)
        s, i, j = c.s, c.i, c.j
        e = i+n-1
    end
    c.i = i+n
    return s:sub(i, e)
end

unpackers['str16'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+1 > j then
        c:underflow(i+1)
        s, i, j = c.s, c.i, c.j
    end
    local n = unpack('>I2', s, i)
    i = i+2
    c.i = i
    local e = i+n-1
    if e > j then
        c:underflow(e)
        s, i, j = c.s, c.i, c.j
        e = i+n-1
    end
    c.i = i+n
    return s:sub(i, e)
end

unpackers['str32'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+3 > j then
        c:underflow(i+3)
        s, i, j = c.s, c.i, c.j
    end
    local n = unpack('>I4', s, i)
    i = i+4
    c.i = i
    local e = i+n-1
    if e > j then
        c:underflow(e)
        s, i, j = c.s, c.i, c.j
        e = i+n-1
    end
    c.i = i+n
    return s:sub(i, e)
end

unpackers['bin8'] = unpackers['str8']
unpackers['bin16'] = unpackers['str16']
unpackers['bin32'] = unpackers['str32']

unpackers['fixarray'] = function (c, val)
    return unpack_array(c, val & 0xF)
end

unpackers['array16'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+1 > j then
        c:underflow(i+1)
        s, i, j = c.s, c.i, c.j
    end
    local n = unpack('>I2', s, i)
    c.i = i+2
    return unpack_array(c, n)
end

unpackers['array32'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+3 > j then
        c:underflow(i+3)
        s, i, j = c.s, c.i, c.j
    end
    local n = unpack('>I4', s, i)
    c.i = i+4
    return unpack_array(c, n)
end

unpackers['fixmap'] = function (c, val)
    return unpack_map(c, val & 0xF)
end

unpackers['map16'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+1 > j then
        c:underflow(i+1)
        s, i, j = c.s, c.i, c.j
    end
    local n = unpack('>I2', s, i)
    c.i = i+2
    return unpack_map(c, n)
end

unpackers['map32'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+3 > j then
        c:underflow(i+3)
        s, i, j = c.s, c.i, c.j
    end
    local n = unpack('>I4', s, i)
    c.i = i+4
    return unpack_map(c, n)
end

function m.build_ext (tag, data)
    return nil
end

for k = 0, 4 do
    local n = tointeger(2^k)
    unpackers['fixext' .. tostring(n)] = function (c)
        local s, i, j = c.s, c.i, c.j
        if i > j then
            c:underflow(i)
            s, i, j = c.s, c.i, c.j
        end
        local tag = unpack('>i1', s, i)
        i = i+1
        c.i = i
        local e = i+n-1
        if e > j then
            c:underflow(e)
            s, i, j = c.s, c.i, c.j
            e = i+n-1
        end
        c.i = i+n
        return m.build_ext(tag, s:sub(i, e))
    end
end

unpackers['ext8'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i > j then
        c:underflow(i)
        s, i, j = c.s, c.i, c.j
    end
    local n = unpack('>I1', s, i)
    i = i+1
    c.i = i
    if i > j then
        c:underflow(i)
        s, i, j = c.s, c.i, c.j
    end
    local tag = unpack('>i1', s, i)
    i = i+1
    c.i = i
    local e = i+n-1
    if e > j then
        c:underflow(e)
        s, i, j = c.s, c.i, c.j
        e = i+n-1
    end
    c.i = i+n
    return m.build_ext(tag, s:sub(i, e))
end

unpackers['ext16'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+1 > j then
        c:underflow(i+1)
        s, i, j = c.s, c.i, c.j
    end
    local n = unpack('>I2', s, i)
    i = i+2
    c.i = i
    if i > j then
        c:underflow(i)
        s, i, j = c.s, c.i, c.j
    end
    local tag = unpack('>i1', s, i)
    i = i+1
    c.i = i
    local e = i+n-1
    if e > j then
        c:underflow(e)
        s, i, j = c.s, c.i, c.j
        e = i+n-1
    end
    c.i = i+n
    return m.build_ext(tag, s:sub(i, e))
end

unpackers['ext32'] = function (c)
    local s, i, j = c.s, c.i, c.j
    if i+3 > j then
        c:underflow(i+3)
        s, i, j = c.s, c.i, c.j
    end
    local n = unpack('>I4', s, i)
    i = i+4
    c.i = i
    if i > j then
        c:underflow(i)
        s, i, j = c.s, c.i, c.j
    end
    local tag = unpack('>i1', s, i)
    i = i+1
    c.i = i
    local e = i+n-1
    if e > j then
        c:underflow(e)
        s, i, j = c.s, c.i, c.j
        e = i+n-1
    end
    c.i = i+n
    return m.build_ext(tag, s:sub(i, e))
end


local function cursor_string (str)
    return {
        s = str,
        i = 1,
        j = #str,
        underflow = function (self)
                        error "missing bytes"
                    end,
    }
end

local function cursor_loader (ld)
    return {
        s = '',
        i = 1,
        j = 0,
        underflow = function (self, e)
                        self.s = self.s:sub(self.i)
                        e = e - self.i + 1
                        self.i = 1
                        self.j = 0
                        while e > self.j do
                            local chunk = ld()
                            if not chunk then
                                error "missing bytes"
                            end
                            self.s = self.s .. chunk
                            self.j = #self.s
                        end
                    end,
    }
end

function m.unpack (s)
    checktype('unpack', 1, s, 'string')
    local cursor = cursor_string(s)
    local data = unpackers['any'](cursor)
    if cursor.i < cursor.j then
        error "extra bytes"
    end
    return data
end

function m.unpacker (src)
    if type(src) == 'string' then
        local cursor = cursor_string(src)
        return function ()
            if cursor.i <= cursor.j then
                return cursor.i, unpackers['any'](cursor)
            end
        end
    elseif type(src) == 'function' then
        local cursor = cursor_loader(src)
        return function ()
            if cursor.i > cursor.j then
                pcall(cursor.underflow, cursor, cursor.i)
            end
            if cursor.i <= cursor.j then
                return true, unpackers['any'](cursor)
            end
        end
    else
        argerror('unpacker', 1, "string or function expected, got " .. type(src))
    end
end

set_string'string_compat'
set_integer'unsigned'
if math_type(0.0) == math_type(0) then
    set_number'integer'
elseif #pack('n', 0.0) == 4 then
    m.small_lua = true
    set_number'float'
else
    m.full64bits = true
    set_number'double'
end
set_array'without_hole'

m._VERSION = '0.3.3'
m._DESCRIPTION = "lua-MessagePack : a pure Lua implementation"
m._COPYRIGHT = "Copyright (c) 2012-2015 Francois Perrad"

--
-- This library is licensed under the terms of the MIT/X11 license,
-- like Lua itself.
--
