local majorVersion = '22'
local minorVersion = '22'
local patch = '0'
local baseVersion = ('%s.%s.%s'):format(majorVersion, minorVersion, patch)
local dllName = 'libuv.dll'
local pdbName = 'libuv.pdb'
local libName = 'libuv.lib'
local soName = 'libuv.so'
local linuxShortName = 'uv'

return {
	include = function()
		includedirs { '../vendor/libnode/include/node/uv/include/' }

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

        local uvBinDir = path.getabsolute('../') .. '/vendor/libnode/bin'

		if os.istarget('windows') then
			filter 'files:**/tag.txt'
                buildmessage 'Preparing libuv'

                buildoutputs {
                    ('%%{cfg.targetdir}/%s'):format(dllName),
                }

                buildcommands {
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
					('{COPY} %s/%s %%{cfg.targetdir}'):format(uvBinDir, soName),
                }
		end

	end
}
