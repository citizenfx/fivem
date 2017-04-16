return {
	include = function()
		includedirs "../vendor/imgui/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		files_project "../vendor/imgui/" {
			"imgui.cpp",
			"imgui_draw.cpp"
		}
	end
}