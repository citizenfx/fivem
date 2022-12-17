includedirs { '../voip-mumble/include' }
libdirs { '../voip-mumble/lib' }
links { 'avutil', 'swresample' }

links { 'runtimeobject' }
linkoptions '/DELAYLOAD:ole32.dll /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-1.dll /DELAYLOAD:api-ms-win-core-winrt-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-string-l1-1-0.dll'

return function()
	filter {}
	
	add_dependencies { 'vendor:dspfilters', 'vendor:openssl_crypto' }
end
