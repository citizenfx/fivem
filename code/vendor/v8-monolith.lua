local version = ...
local majorVersion, minorVersion = version:match("^(%d+)%.(%d+)")
local baseVersion = ('%s.%s'):format(majorVersion, minorVersion)
local libName = 'v8_monolith.lib'
local libNameDebug = 'v8_monolithd.lib'

return {
    include = function()
        defines {
            'V8_COMPRESS_POINTERS',
            'V8_31BIT_SMIS_ON_64BIT_ARCH',
            'V8_COMPRESS_POINTERS_IN_SHARED_CAGE',
            'CPPGC_CAGED_HEAP',
            'V8_ENABLE_CHECKS',
        }

        if os.istarget('windows') then
            includedirs { "../vendor/v8/" .. baseVersion .. "/source/include/" }
            libdirs { "../vendor/v8/" .. baseVersion .. "/lib/" }

            filter "configurations:Debug"
            links { 
                libNameDebug,
                'Dbghelp.lib',
            }
            filter {}

            filter "configurations:Release"
            links { 
                libName,
                'Dbghelp.lib',
            }
            filter {}
        end
    end,
    run = function()
        kind 'Utility'
        
        files {
            ('../vendor/v8/%s/tag.txt'):format(baseVersion)
        }

        local baseURL = ('https://content.cfx.re/mirrors/vendor/v8/%s'):format(baseVersion)
        local v8LibDir = (path.getabsolute('../') .. '/vendor/v8/%s/lib'):format(baseVersion)

        if os.istarget('windows') then
            filter { "files:**/tag.txt", "configurations:Debug" }
                buildmessage 'Downloading v8_monolithd.lib'
                buildoutputs {
                    ('%s/%s'):format(v8LibDir, libNameDebug),
                }
                buildcommands {
                    -- download files, redownload only if outdated
                    'echo Pre-build step for downloading v8_monolith libraries',
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s?t=%s"'):format(v8LibDir, libNameDebug, v8LibDir, libNameDebug, baseURL, libNameDebug, os.time()),
                    'if %errorlevel% neq 0 (exit /b 1)',
                }
            filter{}

            filter { "files:**/tag.txt", "configurations:Release" }
                buildmessage 'Downloading v8_monolith.lib'
                buildoutputs {
                    ('%s/%s'):format(v8LibDir, libName),
                }
                buildcommands {
                    -- download files, redownload only if outdated
                    'echo Pre-build step for downloading v8_monolith libraries',
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s?t=%s"'):format(v8LibDir, libName, v8LibDir, libName, baseURL, libName, os.time()),
                    'if %errorlevel% neq 0 (exit /b 1)',
                }
            filter{}
        end
    end
}
