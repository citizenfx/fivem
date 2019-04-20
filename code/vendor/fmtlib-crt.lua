return {
	include = function()
		includedirs { "../vendor/fmtlib/" }
	end,

	run = function()
		targetname "fmtlib-crt"
		language "C++"
		kind "StaticLib"
		staticruntime 'On'
		
		files
		{
			"../vendor/fmtlib/fmt/*.cc",
			"../vendor/fmtlib/fmt/*.h",
		}
	end
}