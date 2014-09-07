-- temp: until proper components exist
includedirs { "../../client/game_ny/base/", "../../client/game_ny/rage/", "../../client/game_ny/gta/" }

links { 'GameNY' }

-- proper stuff
libdirs { "../../../vendor/luajit/src/" }

includedirs { "../../../vendor/luajit/src/" }

links { 'lua51' }