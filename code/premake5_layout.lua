-- setup pre-build and post-build layouts
project 'CfxPrebuild'
	kind 'Utility'
	
	prebuildcommands {
		('"%s"'):format(
			path.getabsolute('tools/build/run_prebuild.cmd')
		)
	}
	
project 'CfxPostbuild'
	kind 'Utility'
	
	if _OPTIONS['game'] ~= 'launcher' then
		dependson { 'glue' }
	else
		dependson { 'citizen-game-main' }
	end
	
	files {
		'tools/build/run_postbuild.ps1'
	}

	filter 'files:**.ps1'
		buildmessage 'Preparing application to run from layout...'
		
		buildcommands {
			-- directly use $(TargetDir) and remove trailing \ so pwsh doesn't get upset
			([["%s" "$(TargetDir.TrimEnd('\'))" %s]]):format(
				path.getabsolute('tools/build/run_postbuild.cmd'),
				_OPTIONS['game']
			)
		}
		
		buildoutputs { 'tools/build/dummy_dont_generate.txt' }