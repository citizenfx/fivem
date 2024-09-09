component 'http-client'
component 'glue'
component 'font-renderer'
component 'debug-net'
component 'scrbind-formats'
component 'scrbind-base'
component 'nui-profiles'
component 'profiles'
component 'pool-sizes-state'

if _OPTIONS['game'] == 'server' then
	component 'citizen-server-main'
	component 'citizen-server-net'
	
	if os.istarget('windows') then
		component 'citizen-server-gui'
		component 'citizen-devtools'
		component 'citizen-server-fxdk'
	end
	component 'citizen-scripting-v8node'
	component 'citizen-scripting-mono'
	component 'citizen-scripting-mono-v2'
	component 'citizen-server-instance'
	component 'citizen-server-impl'
	component 'citizen-server-state-fivesv'
	component 'citizen-server-state-rdr3sv'
	component 'conhost-server'
	component 'scripting-server'
	component 'voip-server-mumble'
	component 'citizen-server-monitor'
	component 'vfs-impl-server'
else
	component 'citizen-devtools'
	component 'sticky'
	component 'steam'

	if _OPTIONS["game"] ~= 'ny' then
		component 'tool-formats'
		component 'tool-vehrec'
	end
	
	component 'nui-core'
	component 'nui-gsclient'
	component 'nui-resources'
	component 'citizen-game-main'
	component 'citizen-game-ipc'

	if _OPTIONS['game'] ~= 'launcher' and _OPTIONS["game"] ~= 'ny' then
		component 'rage-formats-x'
		component 'voip-mumble'
	end
	
	if _OPTIONS['game'] == 'five' then
		component 'fxdk-main'
	end
	
	component 'conhost-v2'

	component 'citizen-legacy-net-resources'
	component 'citizen-resources-client'

	component 'net'

	component 'citizen-scripting-mono'
	component 'citizen-scripting-mono-v2'

	if _OPTIONS['game'] ~= 'ny' then
		component 'citizen-scripting-v8client'
		component 'citizen-scripting-v8node'
	end

	--component 'n19ui'
end

component 'net-base'
component 'net-packet'
component 'net-tcp-server'
component 'net-http-server'

component 'rage-device-five'
component 'rage-allocator-five'
component 'rage-allocator-rdr3'
component 'rage-graphics-five'
component 'rage-scripting-five'
component 'rage-scripting-rdr3'
component 'lovely-script'
component 'ros-patches-five'
component 'ros-patches-rdr3'

component 'gta-net-five'
component 'rage-input-five'
component 'gta-mission-cleanup-five'
component 'rage-nutsnbolts-five'
component 'gta-core-five'
component 'asi-five'
component 'scripthookv'
component 'gta-streaming-five'
component 'citizen-resources-core'
component 'citizen-resources-gta'
component 'vfs-core'
component 'vfs-impl-rage'
component 'citizen-level-loader-five'
component 'citizen-resources-metadata-lua'
component 'citizen-scripting-core'
component 'citizen-scripting-lua'
component 'citizen-scripting-lua54'
component 'citizen-playernames-five'
component 'scripting-gta'
component 'gta-game-five'
component 'handling-loader-five'
component 'loading-screens-five'
component 'devtools-five'
component 'devcon'
component 'extra-natives-five'
component 'discord'
component 'citizen-mod-loader-five'
component 'debug-script'
component 'rage-graphics-rdr3'
component 'rage-input-rdr3'
component 'rage-nutsnbolts-rdr3'
component 'rage-device-rdr3'

component 'gta-streaming-rdr3'
component 'gta-game-rdr3'
component 'gta-mission-cleanup-rdr3'
component 'citizen-level-loader-rdr3'
component 'gta-core-rdr3'
component 'gta-net-rdr3'
component 'extra-natives-rdr3'
component 'citizen-playernames-rdr3'
component 'devtools-rdr3'

component 'gta-game-ny'
component 'rage-graphics-ny'
component 'rage-nutsnbolts-ny'
component 'rage-input-ny'
component 'rage-scripting-ny'
component 'gta-mission-cleanup-ny'
component 'citizen-playernames-ny'
component 'rage-device-ny'
component 'rage-allocator-ny'
component 'gta-streaming-ny'
component 'gta-core-ny'
component 'citizen-level-loader-ny'
component 'extra-natives-ny'
component 'ros-patches-ny'
component 'gta-net-ny'
