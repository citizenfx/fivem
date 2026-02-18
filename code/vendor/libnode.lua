local majorVersion = '22'
local minorVersion = '22'
local patch = '0'
local baseVersion = ('%s.%s.%s'):format(majorVersion, minorVersion, patch)
local dllName = ('libnode%s.dll'):format(majorVersion)
local pdbName = ('libnode%s.pdb'):format(majorVersion)
local libName = ('libnode%s.lib'):format(majorVersion)
local soName = ('libnode%s.so'):format(majorVersion)
local linuxShortName = ('node%s'):format(majorVersion)

return {
	include = function()
		includedirs { '../vendor/libnode/include/' }
		includedirs { '../vendor/libnode/include/node/' }
		includedirs { '../vendor/libnode/include/node/uv/include/' }
		includedirs { '../vendor/libnode/include/node/v8/include/' }
		includedirs { '../vendor/libnode/include/node/openssl/' }

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

        local baseURL = ('https://github.com/citizenfx/libnode/releases/download/v%s'):format(baseVersion)
        local nodeBinDir = path.getabsolute('../') .. '/vendor/libnode/bin'

		if os.istarget('windows') then
			filter 'files:**/tag.txt'
                buildmessage 'Preparing libnode'

                buildoutputs {
                    ('%%{cfg.targetdir}/%s'):format(dllName),
                }

                buildcommands {
                    -- download files, redownload only if outdated
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, dllName, nodeBinDir, dllName, baseURL, dllName),
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, pdbName, nodeBinDir, pdbName, baseURL, pdbName),
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, libName, nodeBinDir, libName, baseURL, libName),
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
                    ('curl "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, soName, nodeBinDir, soName, baseURL, soName),
					('{COPY} %s/%s %%{cfg.targetdir}'):format(nodeBinDir, soName),
                }
		end

		filter {}
	end
}
