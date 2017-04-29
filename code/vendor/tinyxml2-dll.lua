return {
	include = function()
		includedirs "../vendor/tinyxml2/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		files {
			"../vendor/tinyxml2/tinyxml2.cpp"
		}
	end
}