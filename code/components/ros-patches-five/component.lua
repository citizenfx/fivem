linkoptions "/DELAYLOAD:libcef.dll"

libdirs { "../../../vendor/cef/Release/" }

includedirs { "../../../vendor/cef/" }

links { "libcef_dll", "delayimp", "libGLESv2" }

links { "libcef" }

flags { 'Maps' }

filter 'architecture:x64'
	links { "steam_api64" }

filter()
	libdirs { "lib/" }
