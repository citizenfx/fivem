local baseURL = 'https://content.cfx.re/mirrors'

local version = ...
local baseVersion = version:match('%d+%.%d+')

return {
	include = function()
		defines {
			'V8_COMPRESS_POINTERS',
			'V8_31BIT_SMIS_ON_64BIT_ARCH',
			'V8_COMPRESS_POINTERS_IN_SHARED_CAGE',
			'CPPGC_CAGED_HEAP',
		}

		if os.istarget('windows') then
			includedirs { "../vendor/v8/" .. baseVersion .. "/include/" }
			libdirs { "../vendor/v8/" .. baseVersion .. "/lib/" }
			
			links { ('v8-%s.dll.lib'):format(version) }
		else
			links { 'v8', 'v8_libplatform' }
		end
	end,
	
	run = function()
		kind 'Utility'

		files {
			('../vendor/v8/%s/tag.txt'):format(baseVersion)
		}

		local v8share = ('vendor/v8/%s/share/'):format(baseVersion)
		local v8sharedir = path.getabsolute('../') .. '/' .. v8share

		if os.istarget('windows') then
			filter 'files:**/tag.txt'
				buildmessage 'Preparing V8'

				local v8bin = ('vendor/v8/%s/bin/%%{cfg.shortname:lower()}'):format(baseVersion)
				local v8bindir = path.getabsolute('../') .. '/' .. v8bin
				local v8url = baseURL .. '/' .. v8bin
				local v8dll = ('v8-%s.dll'):format(version)

				buildinputs {
					v8bindir .. '/snapshot_blob.bin',
					v8sharedir .. '/icudtl.dat',
					v8sharedir .. '/icudtl_extra.dat'
				}

				buildoutputs {
					('%%{cfg.targetdir}/citizen/scripting/v8/%s/icudtl.dat'):format(baseVersion),
					('%%{cfg.targetdir}/citizen/scripting/v8/%s/icudtl_extra.dat'):format(baseVersion),
					('%%{cfg.targetdir}/citizen/scripting/v8/%s/snapshot_blob.bin'):format(baseVersion),
					('%%{cfg.targetdir}/v8-%s.dll'):format(version),
				}

				buildcommands {
					('curl.exe "-z%s/%s" -L "-o%s/%s" "%s/%s"'):format(v8bindir, v8dll, v8bindir, v8dll, v8url, v8dll),
					'if %errorlevel% neq 0 (exit /b 1)',
					('{COPY} %s/%s %%{cfg.targetdir}'):format(v8bindir, v8dll),
					('{MKDIR} %%{cfg.targetdir}/citizen/scripting/v8/%s/'):format(baseVersion),
					('{COPY} %s/%s %%{cfg.targetdir}/citizen/scripting/v8/%s/'):format(v8bindir, 'snapshot_blob.bin', baseVersion),
					('{COPY} %s/%s %%{cfg.targetdir}/citizen/scripting/v8/%s/'):format(v8sharedir, '*.dat', baseVersion),
				}
		else
			filter 'files:**/tag.txt'
				buildmessage 'Preparing V8'

				buildinputs {
					v8sharedir .. '/icudtl.dat',
					v8sharedir .. '/icudtl_extra.dat'
				}

				buildoutputs {
					('%%{cfg.targetdir}/citizen/scripting/v8/%s/icudtl.dat'):format(baseVersion),
					('%%{cfg.targetdir}/citizen/scripting/v8/%s/icudtl_extra.dat'):format(baseVersion)
				}

				buildcommands {
					('{MKDIR} %%{cfg.targetdir}/citizen/scripting/v8/%s/'):format(baseVersion),
					('{COPY} %s/%s %%{cfg.targetdir}/citizen/scripting/v8/%s/'):format(v8sharedir, '*.dat', baseVersion),
				}
		end

		filter {}
	end
}
