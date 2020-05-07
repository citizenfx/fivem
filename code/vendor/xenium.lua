return {
	include = function()
		includedirs { "../vendor/xenium/" }
	end,
	
	run = function()
		targetname 'xenium_dummy'
		language 'C'
		kind 'StaticLib'
		
		files {
			'vendor/dummy.c'
		}
	end
}