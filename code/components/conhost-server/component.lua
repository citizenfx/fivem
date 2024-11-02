return function()
	filter {}
	
	if os.istarget('windows') then
		add_dependencies { "vendor:imgui" }
		
		includedirs {
			"components/conhost-v2/include/",
			"components/conhost-v2/src/",
			"../vendor/range-v3/include/",
			"../vendor/utfcpp/source",
		}
		
		files {
			"components/conhost-v2/src/ConsoleHost.cpp",
			"components/conhost-v2/src/ConsoleHostGui.cpp",
			"components/conhost-v2/src/DevGui.cpp",
			"components/conhost-v2/src/backends/**.cpp",
			"components/conhost-v2/src/Textselect.cpp",
		}
	end
end
