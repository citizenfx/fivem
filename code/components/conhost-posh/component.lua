includedirs { "../scrt-mono/deps/include/" }
libdirs { "../scrt-mono/deps/lib/" }

filter 'architecture:not x64'
	links { "mono-2.0-insecure" }

filter 'architecture:x64'
	links { "mono-2.0-insecure64" }