linkoptions "/DELAYLOAD:libcef.dll"

libdirs { "../../../vendor/cef/Release/" }

includedirs { "../../../vendor/cef/" }

links { "libcef_dll", "delayimp", "libGLESv2" }

links { "libcef" }

return function()
	filter {}
	configuration {}

	linkoptions ("/DELAYLOAD:api-ms-win-core-winrt-error-l1-1-1.dll /DELAYLOAD:api-ms-win-core-winrt-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-string-l1-1-0.dll /DELAYLOAD:ole32.dll")
end