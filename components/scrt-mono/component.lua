filter 'architecture:not x64'
	links { "mono-2.0" }

filter 'architecture:x64'
	links { "mono-2.064" }

includedirs { "deps/include/" }
libdirs { "deps/lib/" }
