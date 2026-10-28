local majorVersion = '22'
local minorVersion = '23'
local patch = '4'
local baseVersion = ('%s.%s.%s'):format(majorVersion, minorVersion, patch)
local dllName = 'libuv.dll'
local pdbName = 'libuv.pdb'
local libName = 'libuv.lib'
local soName = 'libuv.so'
local linuxShortName = 'uv'

local depotURLs = {
	[dllName] = 'https://downloads.cfx-services.net/prod/01a1825c-41ca-7ce8-ad60-879018c0dcd7/libuv.dll',
	[libName] = 'https://downloads.cfx-services.net/prod/01a1825c-450b-7a29-9e0a-54351c1694af/libuv.lib',
	[pdbName] = 'https://downloads.cfx-services.net/prod/01a1825c-47c4-7437-8396-d26296811f35/libuv.pdb',
	[soName]  = 'https://downloads.cfx-services.net/prod/01a1825c-4c4c-7d59-82e9-105f07ff9e06/libuv.so',
}
local function download(curl, dir, name)
	local tlsFlags = (curl == 'curl.exe') and ' --ssl-revoke-best-effort' or ''
	return ('%s --fail -L%s "-z%s/%s" "-o%s/%s" "%s"'):format(curl, tlsFlags, dir, name, dir, name, depotURLs[name])
end

return {
	include = function()
		includedirs { '../vendor/libnode/include/node/uv/include/' }

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

        local uvBinDir = path.getabsolute('../') .. '/vendor/libuv/bin'

		if os.istarget('windows') then
			filter 'files:**/tag.txt'
                buildmessage(('Preparing libuv for libnode %s'):format(baseVersion))

                buildoutputs {
                    ('%%{cfg.targetdir}/%s'):format(dllName),
                }

                buildcommands {
                    -- download files, redownload only if outdated
                    download('curl.exe', uvBinDir, dllName),
					'if %errorlevel% neq 0 (exit /b 1)',
                    download('curl.exe', uvBinDir, pdbName),
					'if %errorlevel% neq 0 (exit /b 1)',
                    download('curl.exe', uvBinDir, libName),
					'if %errorlevel% neq 0 (exit /b 1)',
					('{COPY} %s/%s %%{cfg.targetdir}'):format(uvBinDir, dllName),
                    -- copy pdb manually to the server files
					'{MKDIR} %{cfg.targetdir}/dbg/',
					('{COPY} %s/%s %%{cfg.targetdir}/dbg/'):format(uvBinDir, pdbName),
                }
		else
			filter 'files:**/tag.txt'
                buildmessage(('Preparing libuv for libnode %s'):format(baseVersion))

                buildoutputs {
                    ('%%{cfg.targetdir}/%s'):format(soName),
                }

                buildcommands {
                    download('curl', uvBinDir, soName),
					('{COPY} %s/%s %%{cfg.targetdir}'):format(uvBinDir, soName),
                }
		end

		filter {}
	end
}
