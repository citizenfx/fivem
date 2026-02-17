local majorVersion = '22'
local minorVersion = '11'
local patch = '0'
local baseVersion = ('%s.%s.%s'):format(majorVersion, minorVersion, patch)
local dllName = 'libuv.dll'
local pdbName = 'libuv.pdb'
local libName = 'libuv.lib'
local soName = 'libuv.so'
local libName = 'libuv.lib'
local linuxShortName = 'uv'

return {
	include = function()
		includedirs { '../vendor/libnode/node/deps/uv/include/' }

        libdirs { '../vendor/libuv/bin/' }
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
			'../vendor/libuv/tag.txt'
		}

        local baseURL = ('https://content.cfx.re/mirrors/vendor/node/v%s/libnode'):format(baseVersion)
        local uvBinDir = path.getabsolute('../') .. '/vendor/libuv/bin'

		if os.istarget('windows') then
			filter 'files:**/tag.txt'
                buildmessage 'Preparing libuv'

                buildoutputs {
                    ('%%{cfg.targetdir}/%s'):format(dllName),
                }

                buildcommands {
                    -- download files, redownload only if outdated
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s?t=%s"'):format(uvBinDir, dllName, uvBinDir, dllName, baseURL, dllName, os.time()),
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s?t=%s"'):format(uvBinDir, pdbName, uvBinDir, pdbName, baseURL, pdbName, os.time()),
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s?t=%s"'):format(uvBinDir, libName, uvBinDir, libName, baseURL, libName, os.time()),
					'if %errorlevel% neq 0 (exit /b 1)',
					('{COPY} %s/%s %%{cfg.targetdir}'):format(uvBinDir, dllName),
                    -- copy pdb manually to the server files
					'{MKDIR} %{cfg.targetdir}/dbg/',
					('{COPY} %s/%s %%{cfg.targetdir}/dbg/'):format(uvBinDir, pdbName),
                }
		else
			filter 'files:**/tag.txt'
                buildmessage 'Preparing libuv'

                buildoutputs {
                    ('%%{cfg.targetdir}/%s'):format(soName),
                }

                buildcommands {
                    ('curl "-z%s/%s" -L "-o%s/%s" "%s/%s?t=%s"'):format(uvBinDir, soName, uvBinDir, soName, baseURL, soName, os.time()),
					('{COPY} %s/%s %%{cfg.targetdir}'):format(uvBinDir, soName),
                }
		end

	end
}
