links { 'v8', 'v8_libplatform' }

--defines { 'USING_V8_SHARED' }

if os.istarget('windows') then
	includedirs { '../../deplibs/include/include/' }
end
