return {
	include = function()
		includedirs "../vendor/discord-rpc/include/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"
		
		defines { "DISCORD_BUILDING_SDK" }
		
		links { "psapi" }

		files_project "../vendor/discord-rpc/" {
			"src/discord-rpc.cpp",
			"src/rpc_connection.cpp",
			"src/serialization.cpp",
			"src/connection_win.cpp",
			"src/discord_register_win.cpp",
		}
	end
}