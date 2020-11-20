return function()
	filter {}
	configuration {}
	
	if os.istarget('windows') then
		add_dependencies { "vendor:imgui" }
		
		includedirs {
			"components/conhost-v2/include/",
			"components/conhost-v2/src/",
		}
		
		files {
			"components/conhost-v2/src/ConsoleHost.cpp",
			"components/conhost-v2/src/ConsoleHostGui.cpp",
			"components/conhost-v2/src/DevGui.cpp",
			"components/conhost-v2/src/backends/**.cpp",
		}
	end
end