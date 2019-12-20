return {
	include = function()
		includedirs { "../vendor/fmtlib/include/" }
	end,

	run = function()
		targetname "fmtlib-crt"
		language "C++"
		kind "StaticLib"
		staticruntime "On"
		
		files
		{
			"../vendor/fmtlib/src/**.cc",
			"../vendor/fmtlib/src/**.h",
		}
	end
}