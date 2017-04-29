linkoptions "/DELAYLOAD:libcef.dll"

libdirs { "../../client/libcef/lib/" }

includedirs { "../../client/libcef/" }

links { "libcef_dll", "delayimp", "libGLESv2" }

filter 'architecture:not x64'
	links { "libcef" }

filter 'architecture:x64'
	links { "libcef64" }
