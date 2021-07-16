local static = ...

return {
	include = function()
		includedirs { "../vendor/fmtlib/include/" }
	end,

	run = function()
		if static then
			targetname "fmtlib-crt"
		else
			targetname "fmtlib"
		end

		language "C++"
		kind "StaticLib"

		if static then
			staticruntime "On"
		end
		
		files
		{
			"../vendor/fmtlib/src/**.cc",
			"../vendor/fmtlib/src/**.h",
		}

		removefiles
		{
			-- C++ module
			"../vendor/fmtlib/src/fmt.cc",
		}
	end
}