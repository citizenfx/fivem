
local json = require('dkjson')


local function printFunctionName(native)
	return native.name:lower():gsub('0x', 'n_0x'):gsub('_(%a)', string.upper):gsub('(%a)(.+)', function(a, b)
		return a:upper() .. b
	end)
end

local function printCName(native)
    return native.name:gsub('0x', '_0x')
end

local function printCType(t, v)
	local s = t.name:gsub('Ptr', '*')

	if v and v.pointer then
		s = s .. '*'
	end

	return s
end

local keywords = {
    "alignas",
    "alignof",
    "and",
    "and_eq",
    "asm",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "char16_t",
    "char32_t",
    "class",
    "compl",
    "concept",
    "const",
    "constexpr",
    "const_cast",
    "continue",
    "decltype",
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "noexcept",
    "not",
    "not_eq",
    "nullptr",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "register",
    "reinterpret_cast",
    "requires",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_assert",
    "static_cast",
    "struct",
    "switch",
    "template",
    "this",
    "thread_local",
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq",
    "and",
    "bitor",
    "or",
    "xor",
    "compl",
    "bitand",
    "and_eq",
    "or_eq",
    "xor_eq",
    "not",
    "not_eq"
}

local function printIdentifier(n)
    for _, v in ipairs(keywords) do
        if v == n then
            return n .. '_'
        end
    end

    return n
end

local function printNative(native)
    local function saveThis(name, str)
        local function tryOpen()
            return io.open(('%s/%s/%s.md'):format(os.getenv('NATIVES_MD_DIR'), native.ns or 'cfx', name), 'w')
        end

        local f = tryOpen()

        if not f then
            os.execute(('mkdir %s\\%s'):format(os.getenv('NATIVES_MD_DIR'), native.ns or 'cfx'))

            f = tryOpen()
        end

        f:write(str)
        f:close()
    end

    local function printThis(name)
        local str = ''

        str = str .. ('---\nns: %s%s%s\n---\n'):format(native.ns or 'CFX',
            #native.aliases > 0 and ('\naliases: ' .. json.encode(native.aliases)) or '',
            native.apiset[1] and ('\napiset: ' .. native.apiset[1]) or '')

        str = str .. '## ' .. printCName(native) .. '\n\n'

        str = str .. '```c\n'

        if native.jhash or #native.hash > 10 then
            str = str .. ('// %s%s\n'):format(native.hash,
                native.jhash and ((' 0x%08X'):format(native.jhash)) or '')
        end

        str = str .. (native.returns and printCType(native.returns) or 'void')

        str = str .. ' ' .. printCName(native) .. '('

        local args = {}

        for _, p in ipairs(native.arguments) do
            table.insert(args, ('%s %s'):format(printCType(p.type, p), printIdentifier(p.name)))
        end

        str = str .. table.concat(args, ', ')

        str = str .. ');\n```\n\n'

        local d = parseDocString(native)

        if d and d.summary then
            d.summary = d.summary:gsub('&gt;', '>'):gsub('&lt;', '<'):gsub('&amp;', '&')

            if (not native.ns) or (native.ns == 'CFX') then
                for line in trim(d.summary):gmatch("([^\n]+)") do
                    str = str .. trim(line) .. "\n"
                end
            else
                local firstIndent

                str = str .. '```\n'

                for line in trim(d.summary):gmatch("([^\n]+)") do
                    if not firstIndent then
                        firstIndent = line:match("^%s+")
                    end

                    line = line:gsub(firstIndent or '', '')--:gsub('^-+', '')
                    str = str .. line .. "  \n"
                end

                str = str .. '```\n'
            end
        end

        str = str .. '\n'

        args = {}

        if d and d.hasParams then
            for _, v in ipairs(d.params) do
			    args[v[1]] = v[2]
            end
        end
    
        if #native.arguments > 0 then
            str = str .. '## Parameters\n'

            for _, p in ipairs(native.arguments) do
                str = str .. ('* **%s**: %s\n'):format(printIdentifier(p.name), args[p.name] or '')
            end

            str = str .. '\n'
        end

        if native.returns and native.returns.type ~= 'void' then
            str = str .. '## Return value\n'
        end

        if d and d.returns then
            str = str .. d.returns .. '\n'
        end

        return str
    end

    saveThis(printFunctionName(native), printThis(printFunctionName(native)))
end

for _, v in pairs(_natives) do
    --if v.name == 'CALL_MINIMAP_SCALEFORM_FUNCTION' then
        printNative(v)

        --return
    --end
end