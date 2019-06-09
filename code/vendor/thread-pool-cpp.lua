return {
	include = function()
		includedirs { '../vendor/thread-pool-cpp/include/' }
	end,
	
	run = function()
		targetname 'tpcpp_dummy'
		language 'C'
		kind 'StaticLib'
		
		files {
			'vendor/dummy.c'
		}
	end
}