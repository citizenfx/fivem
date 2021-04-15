return {
	include = function()
		includedirs { "../vendor/im3d/" }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"

		files_project "../vendor/im3d/" {
			"im3d.cpp"
		}
	end
}
