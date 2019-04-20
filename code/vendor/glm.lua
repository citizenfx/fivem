return {
	include = function()
		includedirs { "../vendor/glm/" }
	end,
	
	run = function()
		targetname 'glm_dummy'
		language 'C'
		kind 'StaticLib'
		
		files {
			'vendor/dummy.c'
		}
	end
}