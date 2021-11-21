linkoptions "/DELAYLOAD:libcef.dll"


if _OPTIONS["game"] ~= "ny" then
	libdirs { "../../../vendor/cef/Release/" }

	includedirs { "../../../vendor/cef/" }
else
	libdirs { "../../../vendor/cef32/Release/" }

	includedirs { "../../../vendor/cef32/" }
end
links { "libcef_dll", "delayimp", "libGLESv2" }

links { "libcef" }

return function()
	filter {}

	linkoptions ("/DELAYLOAD:api-ms-win-core-winrt-error-l1-1-1.dll /DELAYLOAD:api-ms-win-core-winrt-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-string-l1-1-0.dll /DELAYLOAD:ole32.dll")
	add_dependencies { 'vendor:tbb' }
end
