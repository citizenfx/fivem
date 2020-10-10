local lastLang
local lastKind

local prj_root = path.getabsolute('../../')

if os.istarget('windows') and _OPTIONS['game'] == 'server' and false then
	local oldLanguage = language
	local oldProject = project
	local oldKind = kind
	
	local function check()
		if lastLang == 'C' or lastLang == 'C++' then
			if lastKind == 'SharedLib' or lastKind == 'WindowedApp' or lastKind == 'ConsoleApp' then
				links { 'libhoard' }

				filter { 'configurations:Release' }
				files {
					prj_root .. '/vendor/hoard/uselibhoard.cpp'
				}
				
				filter {}
				
				vpaths {
					["z/hoard/*"] = prj_root .. '/vendor/hoard/**'
				}
				
				lastLang = nil
				lastKind = nil
			end
		end
	end
	
	local function initAllocators()
		oldProject 'libhoard'
		oldKind 'SharedLib'
		oldLanguage 'C++'
		
		includedirs {
			prj_root .. '/../vendor/hoard/src/include/',
			prj_root .. '/../vendor/hoard/src/include/util/',
			prj_root .. '/../vendor/hoard/src/include/hoard/',
			prj_root .. '/../vendor/hoard/src/include/superblocks/',
			prj_root .. '/../vendor/heap-layers/',
		}
		
		defines { '_WINDOWS' }
		
		filter { 'configurations:Release' }
		
		files {
			prj_root .. '/vendor/hoard/winwrapper.cpp',
			prj_root .. '/vendor/hoard/wintls.cpp',
		}
		
		filter { 'configurations:Release' }
		
		files_project(prj_root .. '/../vendor/hoard/src/') {
			'source/libhoard.cpp',
		}
		
		filter { 'configurations:Release' }
		
		--[[files_project(prj_root .. '/../vendor/heap-layers/') {
			'wrappers/winwrapper.cpp',
		}]]
		
		filter {}
		
		files {
			prj_root .. '/vendor/hoard/dummy.cpp'
		}
	end
	
	project = function(n)
		local v = oldProject(n)
		
		if n then
			lastLang = nil
			lastKind = nil
		end
		
		return v
	end
	
	kind = function(k)
		oldKind(k)
		
		lastKind = k
		
		check()
	end
	
	language = function(lang)
		oldLanguage(lang)
		
		lastLang = lang
		
		check()
	end
	
	local oldInitProject = project
	
	project = function(name)
		initAllocators()
	
		oldInitProject(name)
		project = oldInitProject
	end
end