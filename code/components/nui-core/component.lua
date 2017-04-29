linkoptions "/DELAYLOAD:libcef.dll"

libdirs { "../../../vendor/cef/Release/" }

includedirs { "../../../vendor/cef/" }

links { "libcef_dll", "delayimp", "libGLESv2" }

links { "libcef" }
