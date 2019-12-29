return {
	include = function()
		includedirs "../vendor/librw/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		files_project "../vendor/librw/src/" {
			"**.cpp",
			"**.h",
		}
	end
}
