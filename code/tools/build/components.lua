-- componentization handlers
local components = { }

dependency = function(name)
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

	if _G.inPrivates then
		decoded.private = true
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
	
	for k, repo in all_private_repos() do
		local repoRel = path.getrelative(path.getabsolute(''), repo) .. '/vendor/' .. name .. '.lua'
		
		if os.isfile(repoRel) then
			vendorTable = loadfile(repoRel)(vendorTable)
		end
	end

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

	-- set group depending on origin
	local groupName = ''
	local nonGameName = comp.name

	-- prefix with a game name, if any
	for _, gname in ipairs(gamenames) do
		-- if it ends in the current game name...
		if comp.name:ends(':' .. gname) then
			groupName = '/' .. gname
			nonGameName = comp.name:gsub(':' .. gname, '')
			break
		end
	end

	if groupName == '' then
		groupName = '/common'
	end

	for val in nonGameName:gmatch('([^:]+):') do
		groupName = groupName .. '/' .. val
	end

	-- replace '/common/citizen/server' prefix with '/server'
	local serverMatch = '/common/citizen/server'
	if groupName:sub(1, #serverMatch) == serverMatch then
		groupName = '/server' .. groupName:sub(#serverMatch + 1)
	end

	-- hack: `net` breaks premake/.vs group generator
	if comp.name == 'net' then
		groupName = groupName .. '/net'
	end

	if comp.private then
		groupName = '/private' .. groupName
	end

	group('components' .. groupName)

	-- project itself
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
	if name ~= 'adhesive' and name ~= 'fxdk-main' then
		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
	end

	-- add dependency requirements
	for k, v in ipairs(deps) do
		local dep = v.dep
		local data = v.data

		filter {}

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

	_G._ROOTPATH = path.getabsolute('.')

	filter {}
	local postCb = dofile(comp.absPath .. '/component.lua')
	local postCbs = {}
	
	table.insert(postCbs, postCb)
	
	filter {}
	if not comp.private then
		for k, repo in all_private_repos() do
			local repoRel = path.getrelative(path.getabsolute(''), repo) .. '/components/' .. name .. '/component_override.lua'
			
			if os.isfile(repoRel) then
				postCb = dofile(repoRel)
				table.insert(postCbs, postCb)
			end
		end
	end

	-- loop again in case a previous file has set a configuration constraint
	for k, v in ipairs(deps) do
		local dep = v.dep
		local data = v.data

		filter {}

		if data.vendor then
			if data.vendor.depend then
				data.vendor.depend()
			end
		else
			dofile(data.absPath .. '/component.lua')
		end
	end

	filter { "system:windows" }
		buildoptions "/MP"

		files {
			relPath .. "/component.rc",
		}

	filter { "system:not windows" }
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
		
	for _, v in ipairs(postCbs) do
		if v then
			v()
		end
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
