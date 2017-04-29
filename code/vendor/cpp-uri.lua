return {
	include = function()
		includedirs "../vendor/cpp-uri/src/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		files {
			"../vendor/cpp-uri/src/**.cpp",
		}

		configuration "windows"
			files { os.getenv("BOOST_ROOT") .. "/libs/system/src/error_code.cpp" } -- so there's no need for bjam messiness

		configuration "not windows"
			links { "boost_system" }
	end
}
