return {
	include = function()
		includedirs { '../vendor/libnode/' }
		includedirs { '../vendor/libnode/node/' }
		includedirs { '../vendor/libnode/node/src/' }
		includedirs { '../vendor/libnode/node/deps/uv/include/' }
		includedirs { '../vendor/libnode/node/deps/v8/include/' }
		includedirs { '../vendor/libnode/node/deps/openssl/' }

        if os.istarget('windows') then
            libdirs { '../vendor/libnode/bin/' }
            links { 'libnode.lib' }
        end
	end,

    run = function()
		kind 'Utility'

        files {
            -- dummy file to generate a project, so build commands are executed
			('../vendor/libnode/tag.txt'):format(baseVersion)
		}

        local baseURL = 'https://content.cfx.re/mirrors/vendor/node/v20.12.2/libnode'
        local nodeBinDir = path.getabsolute('../') .. '/vendor/libnode/bin'

		if os.istarget('windows') then
			filter 'files:**/tag.txt'
                buildmessage 'Preparing libnode'

                buildoutputs {
                    '%%{cfg.targetdir}/libnode.dll',
                }

                buildcommands {
                    -- download files, redownload only if outdated
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, 'libnode.dll', nodeBinDir, 'libnode.dll', baseURL, 'libnode.dll'),
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, 'libnode.pdb', nodeBinDir, 'libnode.pdb', baseURL, 'libnode.pdb'),
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, 'libnode.lib', nodeBinDir, 'libnode.lib', baseURL, 'libnode.lib'),
					'if %errorlevel% neq 0 (exit /b 1)',
					('{COPY} %s/libnode.dll %%{cfg.targetdir}'):format(nodeBinDir),
                    -- copy pdb manually to the server files
					('{COPY} %s/libnode.pdb %%{cfg.targetdir}/dbg'):format(nodeBinDir),
                }
		else
			filter 'files:**/tag.txt'
                buildmessage 'Preparing libnode'

                buildoutputs {
                    '%%{cfg.targetdir}/libnode.so',
                }

                buildcommands {
                    ('curl "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, 'libnode.so', nodeBinDir, 'libnode.so', baseURL, 'libnode.so'),
					('{COPY} %s/libnode.so %%{cfg.targetdir}'):format(nodeBinDir),
                }
		end

		filter {}
	end
}
