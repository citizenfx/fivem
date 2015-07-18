libdirs { "../../../vendor/luajit/src/" }

includedirs { "../../../vendor/luajit/src/" }

filter "architecture:not x64"
        links { 'lua51x86' }

filter "architecture:x64"
        links { "lua51x64" }