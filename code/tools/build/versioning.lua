local lastLang
local lastKind

local prj_root = path.getabsolute('../../')

function resign()
	-- proprietary and confidential tooling
	if os.isfile("C:\\f\\shadesofgray.cmd") then
		removepostbuildcommands {
			"call C:\\f\\shadesofgray.cmd \"$(TargetPath)\""
		}

		postbuildcommands {
			"call C:\\f\\shadesofgray.cmd \"$(TargetPath)\""
		}
	end
end

if os.istarget('windows') then
	local oldLanguage = language
	local oldProject = project
	local oldKind = kind
	
	local function check()
		if lastLang == 'C' or lastLang == 'C++' then
			if lastKind == 'SharedLib' or lastKind == 'WindowedApp' or lastKind == 'ConsoleApp' then
				resign()
			end
		
			if lastKind == 'SharedLib' then
				prelinkcommands {
					'python "' .. prj_root .. '/tools/gen_rc.py" "%{prj.location}/%{prj.name}.rc" "%{prj.location}/%{prj.name}.res" "$(SDK_ExecutablePath_x64)" "%{prj.name}" "' .. _OPTIONS['game'] .. '"'
				}
				
				linkoptions {
					'%{prj.location}/%{prj.name}.res'
				}
				
				lastLang = nil
				lastKind = nil
			elseif lastKind == 'StaticLib' then
				targetdir '$(IntDir)/out/'
			
				lastLang = nil
				lastKind = nil
			elseif lastKind == 'WindowedApp' or lastKind == 'ConsoleApp' then
				lastLang = nil
				lastKind = nil
			end
		end
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
end