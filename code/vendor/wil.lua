return {
	include = function()
		includedirs { "../vendor/wil/include/" }
	end,
	
	run = function()
		language 'C'
		kind 'StaticLib'
		
		files {
			'vendor/dummy.c'
		}
	end
}