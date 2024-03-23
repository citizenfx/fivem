	project 'CitiTest'
		language 'C++'
		kind 'ConsoleApp'

		symbols 'Full'
		
		links { 'Shared', 'CitiCore' }
		
		add_dependencies { 'net:base', 'vendor:catch2' }
		
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
