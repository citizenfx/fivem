links { 'ws2_32', 'protobuf_lite', 'cpp-uri' }

includedirs { "../../../vendor/protobuf/src/", "../../../vendor/cpp-uri/src/", "src/messages/" }

configuration 'Debug*'
        links { 'botand' }

configuration 'Release*'
        links { 'botan' }
