links { 'ws2_32', 'protobuf_lite', 'avutil', 'avresample', 'opus' }

includedirs { "../../../vendor/protobuf/src/", "../../../vendor/libopus/include/" }

configuration 'Debug*'
	links { 'botand' }

configuration 'Release*'
	links { 'botan' }