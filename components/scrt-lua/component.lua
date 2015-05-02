-- temp: until proper components exist
if _OPTIONS['game'] == 'ny' then
	includedirs { "../../client/game_ny/base/", "../../client/game_ny/rage/", "../../client/game_ny/gta/" }

	links { 'GameNY' }
end

-- proper stuff
libdirs { "../../../vendor/luajit/src/" }

includedirs { "../../../vendor/luajit/src/" }

filter "architecture:not x64"
	links { 'lua51x86' }

filter "architecture:x64"
	links { "lua51x64" }
