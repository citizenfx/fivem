return {
	include = function()
		includedirs { "../vendor/citizen_enet/include/" }
	end,
	
	run = function()
		targetname 'citizen_enet'
		language 'C++'
        kind 'StaticLib'

        add_dependencies("vendor:xenium")
        add_dependencies("vendor:citizen_util")
        
        defines { "HAS_INET_NTOP", "HAS_INET_PTON" }

		if not os.istarget('windows') then
			defines { "HAS_SOCKLEN_T" }
		end

		files_project "../vendor/citizen_enet/src/" {
			"*.cpp"
		}
	end
}