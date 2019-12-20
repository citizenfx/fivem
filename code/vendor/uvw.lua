return {
	include = function()
		includedirs { "../vendor/uvw/src/" }
	end,
	
	run = function()
		targetname 'uvw_dummy'
		language 'C'
		kind 'StaticLib'
		
		files {
			'vendor/dummy.c'
		}
	end
}