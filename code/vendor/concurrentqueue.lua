return {
	include = function()
		includedirs { "../vendor/concurrentqueue/" }
	end,
	
	run = function()
		targetname 'concurrentqueue_dummy'
		language 'C'
		kind 'StaticLib'
		
		files {
			'vendor/dummy.c'
		}
	end
}