return {
	include = function()
		includedirs { "../vendor/fmtlib/" }
	end,

	run = function()
		targetname "fmtlib-crt"
		language "C++"
		kind "StaticLib"
		flags "StaticRuntime"
		
		files
		{
			"../vendor/fmtlib/fmt/*.cc",
			"../vendor/fmtlib/fmt/*.h",
		}
	end
}