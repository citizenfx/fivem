-- componentization handlers
local components = { }

dependency = function(name)
	-- find a matching component
	--[[local cname

	for _, c in ipairs(components) do
		if c == name then
			cname = c
			break
		else
			local basename = c:gsub('(.+)-ny', '%1')

			if basename == name then
				cname = c
				break
			end
		end
	end

	if not cname then
		error("Component " .. name .. " seems unknown.")
	end

	includedirs { '../' .. name .. '/include/' }

	links { name }]]

	return
end

package.path = '?.lua'

function string:ends(match)
	return (match == '' or self:sub(-match:len()) == match)
end

local json = require('json')

-- declaration function for components (for components/config.lua)
component = function(name)
	local decoded

	if type(name) == 'string' then
		local filename = name .. '/component.json'

		io.input(filename)
		local jsonStr = io.read('*all')
		io.close()

		decoded = json.decode(jsonStr)

		decoded.rawName = name
		decoded.absPath = path.getabsolute(name)
	else
		decoded = name

		decoded.dummy = true
	end

	-- check if the project name ends in a known game name, and if we should ignore it
	for _, name in ipairs(gamenames) do
		-- if it ends in the current game name...
		if decoded.name:ends(':' .. name) then
			-- ... and it's not the current game we're targeting...
			if name ~= _OPTIONS['game'] then
				-- ... ignore it
				return function() end
			end
		end
	end

	-- add to the list
	table.insert(components, decoded)

	-- return a function to allow table merging for additional parameters
	return function(t)
		for k, v in pairs(t) do
			decoded[k] = v
		end
	end
end

vendor_component = function(name)
	local vendorTable = dofile(name .. '.lua')

	if vendorTable then
		component {
			name = 'vendor:' .. name,
			vendor = vendorTable,
			rawName = name
		}
	end
end

local function id_matches(full, partial)
	local tokenString = ''
	local partialTemp = partial .. ':'

	for token in string.gmatch(full:gsub('\\[.+\\]', ''), '[^:]+') do
		tokenString = tokenString .. token .. ':'

		if partialTemp == tokenString then
			return true
		end
	end

	return false
end

local function find_match(id)
	for _, mcomp in ipairs(components) do
		if mcomp.name == id then
			return mcomp
		end

		if id_matches(mcomp.name, id) then
			return mcomp
		end
	end

	return nil
end

local function process_dependencies(list, basename, deps, hasDeps)
	local isFulfilled = true

	if not basename then
		basename = project().name
	end

	if list then
		for _, dep in ipairs(list) do
			-- find a match for the dependency
			local match = find_match(dep)

			if match and not hasDeps[match.rawName] then
				print(basename .. ' dependency on ' .. dep .. ' fulfilled by ' .. match.rawName)

				hasDeps[match.rawName] = true
				table.insert(deps, { dep = match.rawName, data = match })

				match.tagged = true

				isFulfilled = isFulfilled and process_dependencies(match.dependencies, match.name, deps, hasDeps)
			elseif not match then
				if not dep:match('%[') then
					print('Dependency unresolved for ' .. dep .. ' in ' .. basename)

					return false
				end
			end
		end
	end

	return isFulfilled
end

add_dependencies = function(list)
	if type(list) == 'string' then
		list = { list }
	end

	local deps = {}
	local hasDeps = {}
	local cwd = os.getcwd()
	os.chdir(root_cwd)

	if not process_dependencies(list, nil, deps, hasDeps) then
		error('component dependency from ' .. project().name .. ' unresolved!')
	end

	-- loop over the dependency handlers
	for k, v in ipairs(deps) do
		local dep = v.dep
		local data = v.data

		configuration {}
		filter {}

		if not data.vendor or not data.vendor.dummy then
			links { dep }
		end
		
		if not data.vendor then
			includedirs { 'components/' .. dep .. '/include/' }
		end

		if data.vendor and data.vendor.include then
			data.vendor.include()
		end

		configuration {}
		filter {}

		if data.vendor and data.vendor.depend then
			data.vendor.depend()
		end
	end

	os.chdir(cwd)
end

local do_component = function(name, comp)
	-- do automatic dependencies
	if not comp.dependencies then
		comp.dependencies = {}
	end

	local deps = {}
	local hasDeps = {}

	if not process_dependencies(comp.dependencies, comp.name, deps, hasDeps) then
		return
	end

	-- process the project

	-- path stuff
	local relPath = path.getrelative(path.getabsolute(''), comp.absPath)

	-- set group depending on privateness
	if comp.private then
		group 'components/private'
	else
		group 'components'
	end

	project(name)

	language "C++"
	kind "SharedLib"

	dependson { 'CitiCore' }

	includedirs { "client/citicore/", relPath .. "/include/" }
	files {
		relPath .. "/src/**.cpp",
		relPath .. "/src/**.cc",
		relPath .. "/src/**.h",
		relPath .. "/include/**.h",
		"client/common/StdInc.cpp",
		"client/common/Error.cpp"
	}

	vpaths { ["z/common/*"] = "client/common/**", ["*"] = relPath .. "/**" }

	if not comp.private then
		for k, repo in all_private_repos() do
			local repoRel = path.getrelative(path.getabsolute(''), repo) .. '/components/' .. name

			includedirs { repoRel .. '/include/' }

			files {
				repoRel .. "/src/**.cpp",
				repoRel .. "/src/**.cc",
				repoRel .. "/src/**.h",
				repoRel .. "/include/**.h",
			}

			vpaths {
				["private/*"] = repoRel .. '/**'
			}
		end
	end

	files {
		relPath .. "/include/**.idl",
	}

	defines { "COMPILING_" .. name:upper():gsub('-', '_'), "_CFX_COMPONENT_NAME=" .. name, 'HAS_LOCAL_H' }

	links { "Shared", "fmtlib" }

	-- HACKHACK: premake doesn't allow unsetting these
	if name ~= 'adhesive' then
		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
	end

	-- add dependency requirements
	for k, v in ipairs(deps) do
		local dep = v.dep
		local data = v.data

		configuration {}

		if not data.vendor or not data.vendor.dummy then
			links { dep }
		end

		if data.vendor then
			if data.vendor.include then
				data.vendor.include()
			end
		else
			includedirs { 'components/' .. dep .. '/include/' }
		end
	end

	configuration {}
	local postCb = dofile(comp.absPath .. '/component.lua')

	-- loop again in case a previous file has set a configuration constraint
	for k, v in ipairs(deps) do
		local dep = v.dep
		local data = v.data

		configuration {}

		if data.vendor then
			if data.vendor.depend then
				data.vendor.depend()
			end
		else
			dofile(data.absPath .. '/component.lua')
		end
	end

	configuration "windows"
		buildoptions "/MP"

		files {
			relPath .. "/component.rc",
		}

	configuration "not windows"
		files {
			relPath .. "/component.json"
		}

	filter { "system:not windows", "files:**/component.json" }
		buildmessage 'Copying %{file.relpath}'

		buildcommands {
			'{COPY} "%{file.relpath}" "%{cfg.targetdir}/lib' .. name .. '.json"'
		}

		buildoutputs {
			"%{cfg.targetdir}/lib" .. name .. ".json"
		}

	filter()
		vpaths { ["z/*"] = relPath .. "/component.rc" }
		
	if postCb then
		postCb()
	end

	if not _OPTIONS['tests'] then
		return
	end

	-- test project
	local f = io.open('components/' .. name .. '/tests/main.cpp')

	if f then
		io.close(f)
	end

	if not f then
		return
	end

	project('tests_' .. name)

	language "C++"
	kind "ConsoleApp"

	includedirs { 'components/' .. name .. "/include/" }
	files { 'components/' .. name .. "/tests/**.cpp", 'components/' .. name .. "/tests/**.h", "client/common/StdInc.cpp" }

	if not f then
		files { "tests/test.cpp" }
	end

	links { "Shared", "CitiCore", "gmock_main", "gtest_main", name }

	pchsource "client/common/StdInc.cpp"
	pchheader "StdInc.h"
end

do_components = function()
	for _, comp in ipairs(components) do
		if not comp.dummy then
			do_component(comp.rawName, comp)
		end
	end
end

do_vendor = function()
	local changed = false

	for _, comp in ipairs(components) do
		if comp.vendor and comp.vendor.run and comp.tagged and not comp.ran then
			comp.ran = true
			changed = true

			project(comp.rawName)

			if comp.vendor.include then
				comp.vendor.include()
			end

			comp.vendor.run()
		end
	end

	if changed then
		do_vendor()
	end
end
