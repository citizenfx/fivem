return {
	include = function()
		includedirs "../vendor/imgui/"
	end,

	run = function()
		language "C++"
		kind "SharedLib"

		defines { 'IMGUI_API=__declspec(dllexport)' }

		files_project "../vendor/imgui/" {
			"imgui.cpp",
			"imgui_draw.cpp",
			"imgui_demo.cpp"
		}
	end
}
