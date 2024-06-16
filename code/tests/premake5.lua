	project 'CitiTest'
		language 'C++'
		kind 'ConsoleApp'

		symbols 'Full'
		
		links { 'Shared', 'CitiCore' }
		
		add_dependencies { 'net:base', 'vendor:catch2', 'citizen:server:impl' }
		
		if os.istarget('windows') then
			links { "psapi", "wininet", "winhttp" }
			flags { "NoManifest", "NoImportLib" }
		else
			links { 'dl', 'pthread' }
		end
		
		files
		{
			'**.cpp', '**.hpp', '**.h', '../client/common/Error.cpp'
		}
		
		includedirs
		{
			'include/',
			'../client/citicore/'
		}

		targetname 'CitiTest'
