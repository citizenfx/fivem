if os.getenv('WindowsSdkDir') then
	includedirs(os.getenv('WindowsSdkDir') .. '/Include/10.0.18362.0/cppwinrt')
end

linkoptions "/DELAYLOAD:ole32.dll /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-1.dll /DELAYLOAD:api-ms-win-core-winrt-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-string-l1-1-0.dll"