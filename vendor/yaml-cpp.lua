return {
	include = function()
		includedirs { "../vendor/yaml-cpp/include" }
	end,

	run = function()
		targetname "yaml-cpp"
		language "C++"
		kind "StaticLib"
		
		files
		{
			"../vendor/yaml-cpp/src/*.cpp"
		}
	end
}