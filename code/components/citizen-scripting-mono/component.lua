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
			includedirs { cwd .. "/deps/server/include/" }
			libdirs { cwd .. "/deps/server/lib/" }
		else
			includedirs { cwd .. "/deps/include/" }
			libdirs { cwd .. "/deps/lib/" }
		end
	else
		includedirs { '/usr/include/mono-2.0/ '}
	
		linkoptions '/usr/lib/libmonosgen-2.0.a'
	end
end
