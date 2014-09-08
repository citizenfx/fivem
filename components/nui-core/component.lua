linkoptions "/DELAYLOAD:libcef.dll"

libdirs { "../../client/libcef/lib/" }

includedirs { "../../client/libcef/" }

links { "libcef_dll", "delayimp" }

configuration "Debug*"
	links { "libcefd" }
	
configuration "Release*"
	links { "libcef" }