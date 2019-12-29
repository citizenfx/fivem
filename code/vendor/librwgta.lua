return {
	include = function()
		includedirs "../vendor/librwgta/src/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"
		
		add_dependencies { 'vendor:librw' }

		files_project "../vendor/librwgta/src/" {
			"*.cpp",
			"*.h",
		}
	end
}
