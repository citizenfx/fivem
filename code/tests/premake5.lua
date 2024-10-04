	project 'CitiTest'
		language 'C++'
		kind 'ConsoleApp'

		symbols 'Full'

		links { 'Shared' }

		add_dependencies { 'net:base', 'vendor:catch2' }

		if os.istarget('windows') then
			links { "psapi", "wininet", "winhttp" }
			flags { "NoManifest", "NoImportLib" }
		else
			links { 'dl', 'pthread' }
		end

		if _OPTIONS['game'] == 'server' then
			links { 'CitiCore' }
			add_dependencies { 'citizen:server:impl' }
			files { 'server/**.cpp', 'server/**.hpp', 'server/**.h' }
    	else
			links { 'CitiCore', 'dbghelp', 'psapi', 'comctl32', 'wininet', 'winhttp', 'crypt32' }
            add_dependencies { 'vendor:tinyxml2', 'vendor:concurrentqueue', 'vendor:tbb', 'vendor:boost_locale', 'vendor:openssl_crypto', 'vendor:citizen_util', 'vendor:xenium', 'citizen:resources:core', 'citizen:scripting:core' }
			files { 'client/**.cpp', 'client/**.hpp', 'client/**.h', '../client/common/StdInc.cpp', '../client/common/CfxLocale.Win32.cpp' }
		end

		files
		{
			'*.cpp', '*.hpp', '*.h', 'shared/**.cpp', 'shared/**.hpp', 'shared/**.h', '../client/common/Error.cpp'
		}
	
		includedirs
		{
			'shared/',
			'include/',
			'../client/citicore/'
		}

		targetname 'CitiTest'
