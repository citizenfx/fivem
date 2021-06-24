return {
	include = function()
		includedirs { "../vendor/citizen_util/include/" }
	end,
	
	run = function()
		targetname 'citizen_util'
		language 'C'
		kind 'StaticLib'
		
		files {
			'vendor/dummy.c'
		}
	end
}
