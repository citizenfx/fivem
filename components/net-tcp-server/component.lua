filter { 'Debug', 'architecture:x64' }
        links { 'botanx64d' }

filter { 'Debug', 'architecture:not x64' }
        links { 'botand' }

filter { 'Release', 'architecture:x64' }
        links { 'botanx64' }

filter { 'Release', 'architecture:not x64' }
        links { 'botan' }