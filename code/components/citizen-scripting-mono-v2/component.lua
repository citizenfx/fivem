local cwd = os.getcwd()

return function()
	filter {}
	
	add_dependencies { 'vendor:eastl' }

	if os.istarget('windows') then
		filter 'architecture:x64'
			links { "mono-2.0-sgen" }
	
		filter 'architecture:x86'
			links { "mono-2.0-sgen-x86" }
	
		filter {}
	
		if _OPTIONS['game'] == 'server' then
			includedirs { cwd .. "/../citizen-scripting-mono/deps/server/include/" }
			libdirs { cwd .. "/../citizen-scripting-mono/deps/server/lib/" }
		else
			includedirs { cwd .. "/../citizen-scripting-mono/deps/include/" }
			libdirs { cwd .. "/../citizen-scripting-mono/deps/lib/" }
		end
	else
		files { cwd .. "/../citizen-scripting-mono/src/MonoComponentHostShared.cpp" }
		
		includedirs { '/usr/include/mono-2.0/ '}
	
		linkoptions '/usr/lib/libmonosgen-2.0.a'
	end
end
