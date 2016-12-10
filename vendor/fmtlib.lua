return {
	include = function()
		includedirs { "../vendor/fmtlib/" }
	end,

	run = function()
		targetname "fmtlib"
		language "C++"
		kind "StaticLib"
		
		files
		{
			"../vendor/fmtlib/fmt/*.cc",
			"../vendor/fmtlib/fmt/*.h",
		}
	end
}