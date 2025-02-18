local majorVersion = '22'
local minorVersion = '11'
local patch = '0'
local baseVersion = ('%s.%s.%s'):format(majorVersion, minorVersion, patch)
local dllName = ('libnode%s.dll'):format(majorVersion)
local pdbName = ('libnode%s.pdb'):format(majorVersion)
local libName = ('libnode%s.lib'):format(majorVersion)
local soName = ('libnode%s.so'):format(majorVersion)
local linuxShortName = ('node%s'):format(majorVersion)

return {
	include = function()
		includedirs { '../vendor/libnode/' }
		includedirs { '../vendor/libnode/node/' }
		includedirs { '../vendor/libnode/node/src/' }
		includedirs { '../vendor/libnode/node/deps/uv/include/' }
		includedirs { '../vendor/libnode/node/deps/v8/include/' }
		includedirs { '../vendor/libnode/node/deps/openssl/' }

        libdirs { '../vendor/libnode/bin/' }
        if os.istarget('windows') then
            links { libName }
        else
			links { linuxShortName }
        end
	end,

    run = function()
		kind 'Utility'

        files {
            -- dummy file to generate a project, so build commands are executed
			('../vendor/libnode/tag.txt'):format(baseVersion)
		}

        local baseURL = ('https://content.cfx.re/mirrors/vendor/node/v%s/libnode'):format(baseVersion)
        local nodeBinDir = path.getabsolute('../') .. '/vendor/libnode/bin'

		if os.istarget('windows') then
			filter 'files:**/tag.txt'
                buildmessage 'Preparing libnode'

                buildoutputs {
                    ('%%{cfg.targetdir}/%s'):format(dllName),
                }

                buildcommands {
                    -- download files, redownload only if outdated
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s?t=%s"'):format(nodeBinDir, dllName, nodeBinDir, dllName, baseURL, dllName, os.time()),
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s?t=%s"'):format(nodeBinDir, pdbName, nodeBinDir, pdbName, baseURL, pdbName, os.time()),
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s?t=%s"'):format(nodeBinDir, libName, nodeBinDir, libName, baseURL, libName, os.time()),
					'if %errorlevel% neq 0 (exit /b 1)',
					('{COPY} %s/%s %%{cfg.targetdir}'):format(nodeBinDir, dllName),
                    -- copy pdb manually to the server files
					'{MKDIR} %{cfg.targetdir}/dbg/',
					('{COPY} %s/%s %%{cfg.targetdir}/dbg/'):format(nodeBinDir, pdbName),
                }
		else
			filter 'files:**/tag.txt'
                buildmessage 'Preparing libnode'

                buildoutputs {
                    ('%%{cfg.targetdir}/%s'):format(soName),
                }

                buildcommands {
                    ('curl "-z%s/%s" -L "-o%s/%s" "%s/%s?t=%s"'):format(nodeBinDir, soName, nodeBinDir, soName, baseURL, soName, os.time()),
					('{COPY} %s/%s %%{cfg.targetdir}'):format(nodeBinDir, soName),
                }
		end

		filter {}
	end
}
