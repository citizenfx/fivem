return {
	include = function()
		includedirs { "../vendor/citizen_util/include/" }
	end,
	
	run = function()
		targetname 'citizen_util'
		language 'C++'
		kind 'StaticLib'
		
        add_dependencies("vendor:xenium")
		
        files_project "../vendor/citizen_util/src/" {
			"*.cpp"
		}
	end
}