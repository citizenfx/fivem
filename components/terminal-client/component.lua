links { 'ws2_32' }

includedirs { "src/messages/" }

configuration 'Debug*'
        links { 'botand' }

configuration 'Release*'
        links { 'botan' }
