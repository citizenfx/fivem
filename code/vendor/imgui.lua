return {
	include = function()
		includedirs "../vendor/imgui/"
	end,

	run = function()
		language "C++"
		kind "SharedLib"

		defines { 'IMGUI_API=__declspec(dllexport)', 'IMGUI_ENABLE_FREETYPE' }

		add_dependencies {
			'vendor:freetype'
		}

		files_project "../vendor/imgui/" {
			"imgui.cpp",
			"imgui_draw.cpp",
			"imgui_demo.cpp",
			"imgui_tables.cpp",
			"imgui_widgets.cpp",
			"misc/freetype/imgui_freetype.*",
		}
	end
}
