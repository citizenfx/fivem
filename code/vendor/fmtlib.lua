return {
	include = function()
		includedirs { "../vendor/fmtlib/include/" }
	end,

	run = function()
		targetname "fmtlib"
		language "C++"
		kind "StaticLib"
		
		files
		{
			"../vendor/fmtlib/src/**.cc",
			"../vendor/fmtlib/src/**.h",
		}
	end
}