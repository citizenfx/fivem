local a = ...

return {
	include = function()
		includedirs { "../vendor/lua-rapidjson/src" }

		add_dependencies 'vendor:lua'
	end,

	run = function()
		targetname "lua-rapidjson"
		language "C"
		kind "StaticLib"
		if a then
			staticruntime "On"
		end

		defines { 
			'GRIT_POWER_TTYPE',
		}

		files {
			"../vendor/lua-rapidjson/src/*.h",
			"../vendor/lua-rapidjson/src/*.hpp",
			"../vendor/lua-rapidjson/src/*.cpp"
		}
	end
}
