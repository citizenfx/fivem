return {
	include = function()
		includedirs { '../vendor/websocketpp/' }
		
		defines { '_WEBSOCKETPP_CPP11_STL_' }
	end,
	
	run = function()
		targetname 'websocketpp_dummy'
		language 'C'
		kind 'StaticLib'
		
		files {
			'vendor/dummy.c'
		}
	end
}