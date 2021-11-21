return function()
	filter {}

	linkoptions "/DELAYLOAD:libcef.dll"
	links { "libcef_dll", "delayimp", "libGLESv2" }
	
	links { "libcef" }

	filter {}
		libdirs { "components/ros-patches-five/lib/" }

	filter 'architecture:x86'
		libdirs { "../vendor/cef32/Release/" }
		includedirs { "../vendor/cef32/" }
		links { "steam_api" }
	
	filter 'architecture:x64'
		libdirs { "../vendor/cef/Release/" }
		includedirs { "../vendor/cef/" }
		links { "steam_api64" }

	add_dependencies { 'vendor:mojo' }
end
