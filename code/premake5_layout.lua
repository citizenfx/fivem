-- setup pre-build and post-build layouts
group 'helpers'

project 'CfxPrebuild'
	kind 'Utility'
	
	files {
		'tools/build/run_prebuild.ps1'
	}
	
	filter 'files:**.ps1'
		buildmessage 'Running pre-build preparation jobs...'

		buildcommands {
			path.getabsolute('tools/build/run_prebuild.cmd')
		}

		buildoutputs { 'tools/build/prebuild_run.txt' }

project 'CfxPostbuild'
	kind 'Utility'
	
	if _OPTIONS['game'] == 'server' then
		dependson { 'citizen-server-impl' }
	elseif _OPTIONS['game'] ~= 'launcher' then
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
			([["%s" "$(TargetDir.TrimEnd('\'))" %s "$(Configuration)"]]):format(
				path.getabsolute('tools/build/run_postbuild.cmd'),
				_OPTIONS['game']
			)
		}
		
		buildoutputs { 'tools/build/dummy_dont_generate.txt' }

group ''
