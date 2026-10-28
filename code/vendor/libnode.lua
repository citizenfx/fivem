local majorVersion = '22'
local minorVersion = '23'
local patch = '4'
local baseVersion = ('%s.%s.%s'):format(majorVersion, minorVersion, patch)
local dllName = ('libnode%s.dll'):format(majorVersion)
local pdbName = ('libnode%s.pdb'):format(majorVersion)
local libName = ('libnode%s.lib'):format(majorVersion)
local soName = ('libnode%s.so'):format(majorVersion)
local linuxShortName = ('node%s'):format(majorVersion)

local depotURLs = {
	[dllName] = 'https://downloads.cfx-services.net/prod/01a1825b-1ef5-71a5-8068-b4c46a3833b7/libnode22.dll',
	[libName] = 'https://downloads.cfx-services.net/prod/01a1825b-3432-7e80-aaf4-b18f1dee3f76/libnode22.lib',
	[pdbName] = 'https://downloads.cfx-services.net/prod/01a1825b-3b2f-7202-91f1-41e147059a2d/libnode22.pdb',
	[soName]  = 'https://downloads.cfx-services.net/prod/01a1825b-8f97-71c7-b6ae-0bd82df2973a/libnode22.so',
}

local function download(curl, dir, name)
	local tlsFlags = (curl == 'curl.exe') and ' --ssl-revoke-best-effort' or ''
	return ('%s --fail -L%s "-z%s/%s" "-o%s/%s" "%s"'):format(curl, tlsFlags, dir, name, dir, name, depotURLs[name])
end

return {
	include = function()
		defines { 'V8_ENABLE_CHECKS' }

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
			'../vendor/libnode/tag.txt'
		}

        local nodeBinDir = path.getabsolute('../') .. '/vendor/libnode/bin'

		if os.istarget('windows') then
			filter 'files:**/tag.txt'
                buildmessage(('Preparing libnode %s'):format(baseVersion))

                buildoutputs {
                    ('%%{cfg.targetdir}/%s'):format(dllName),
                }

                buildcommands {
                    -- download files, redownload only if outdated
                    download('curl.exe', nodeBinDir, dllName),
					'if %errorlevel% neq 0 (exit /b 1)',
                    download('curl.exe', nodeBinDir, pdbName),
					'if %errorlevel% neq 0 (exit /b 1)',
                    download('curl.exe', nodeBinDir, libName),
					'if %errorlevel% neq 0 (exit /b 1)',
					('{COPY} %s/%s %%{cfg.targetdir}'):format(nodeBinDir, dllName),
                    -- copy pdb manually to the server files
					'{MKDIR} %{cfg.targetdir}/dbg/',
					('{COPY} %s/%s %%{cfg.targetdir}/dbg/'):format(nodeBinDir, pdbName),
                }
		else
			filter 'files:**/tag.txt'
                buildmessage(('Preparing libnode %s'):format(baseVersion))

                buildoutputs {
                    ('%%{cfg.targetdir}/%s'):format(soName),
                }

                buildcommands {
                    download('curl', nodeBinDir, soName),
					('{COPY} %s/%s %%{cfg.targetdir}'):format(nodeBinDir, soName),
                }
		end

		filter {}
	end
}
