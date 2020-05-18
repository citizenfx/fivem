return {
	include = function()
		includedirs "../vendor/librw/"
		
		defines { 'RW_BGFX' }
	end,

	run = function()
		language "C++"
		kind "StaticLib"
		
		add_dependencies { 'vendor:bgfx' }

		files_project "../vendor/librw/src/" {
			"**.cpp",
			"**.h",
		}
	end
}
