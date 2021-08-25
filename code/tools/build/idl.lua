function process_includedirs(list)
	local r = ""

	for _, v in ipairs(list) do
		r = r .. ' -I "' .. v .. '"'
	end

	return r
end

local prj_root = path.getabsolute('../../')

local old_files = files

function string.remove_null(s)
	local pos = s:find("\0")

	if not pos then
		return s
	end

	return s:sub(1, pos-1) .. s:sub(pos+1)
end

local function recurse_null(v)
	if type(v) == 'string' then
		return v:remove_null()
	elseif type(v) == 'table' then
		local t = {}

		for k,p in pairs(v) do
			t[k] = recurse_null(p)
		end

		return t
	end

	return v
end

local old_detoken = premake.detoken.expand

function premake.detoken.expand(...)
	local v = old_detoken(...)

	return recurse_null(v)
end

function files(x)
	old_files(x)

	if type(x) == 'table' then
		for _, v in ipairs(x) do
			if v:endswith('.idl') then
				filter 'files:**.idl'

				buildcommands {
					pythonExecutable .. ' "' .. prj_root .. '/tools/idl/header.py" -o "%{file and file.directory or ""}/%{file and file.basename or ""}.h" %{process_includedirs(prj.includedirs):remove_null()} %{(file and (file.abspath) or ""):remove_null()}'
				}

				buildoutputs { '%{file.abspath:gsub(".idl$", ".h")}' }

				filter {}

				break
			end
		end
	end

end
