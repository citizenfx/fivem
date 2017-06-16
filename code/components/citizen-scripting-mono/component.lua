if os.is('windows') then
	filter 'architecture:x64'
		links { "mono-2.0-sgen" }

	includedirs { "deps/include/" }
	libdirs { "deps/lib/" }
else
	includedirs { '/usr/include/mono-2.0/ '}

	linkoptions '/usr/lib/libmonosgen-2.0.a'
end
