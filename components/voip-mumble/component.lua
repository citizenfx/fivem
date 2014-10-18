links { 'ws2_32', 'protobuf_lite' }

includedirs { "../../../vendor/protobuf/src/" }

configuration 'Debug*'
	links { 'botand' }

configuration 'Release*'
	links { 'botan' }