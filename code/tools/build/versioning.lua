local lastLang
local lastKind

local prj_root = path.getabsolute('../../')

if os.istarget('windows') then
	local oldLanguage = language
	local oldProject = project
	local oldKind = kind
	
	local function check()
		if lastLang == 'C' or lastLang == 'C++' then
			if lastKind == 'SharedLib' then
				prebuildcommands {
					'python "' .. prj_root .. '/tools/gen_rc.py" "%{prj.location}/%{prj.name}.rc" "%{prj.name}"'
				}
				
				files {
					'%{prj.location}/%{prj.name}.rc'
				}
				
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