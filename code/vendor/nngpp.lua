return {
	include = function()
		includedirs "../vendor/nngpp/include/"
		add_dependencies 'vendor:nng'
	end,

	run = function()
		targetname 'nngpp_dummy'
		language 'C'
		kind 'StaticLib'
		
		files {
			'vendor/dummy.c'
		}
	end
}
