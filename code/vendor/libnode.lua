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
            links { 'libnode20.lib' }
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
                    '%%{cfg.targetdir}/libnode20.dll',
                }

                buildcommands {
                    -- download files, redownload only if outdated
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, 'libnode20.dll', nodeBinDir, 'libnode20.dll', baseURL, 'libnode20.dll'),
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, 'libnode20.pdb', nodeBinDir, 'libnode20.pdb', baseURL, 'libnode20.pdb'),
                    ('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, 'libnode20.lib', nodeBinDir, 'libnode20.lib', baseURL, 'libnode20.lib'),
					'if %errorlevel% neq 0 (exit /b 1)',
					('{COPY} %s/libnode20.dll %%{cfg.targetdir}'):format(nodeBinDir),
                    -- copy pdb manually to the server files
					('{COPY} %s/libnode20.pdb %%{cfg.targetdir}/dbg'):format(nodeBinDir),
                }
		else
			filter 'files:**/tag.txt'
                buildmessage 'Preparing libnode'

                buildoutputs {
                    '%%{cfg.targetdir}/libnode20.so',
                }

                buildcommands {
                    ('curl "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(nodeBinDir, 'libnode20.so', nodeBinDir, 'libnode20.so', baseURL, 'libnode20.so'),
					('{COPY} %s/libnode20.so %%{cfg.targetdir}'):format(nodeBinDir),
                }
		end

		filter {}
	end
}
