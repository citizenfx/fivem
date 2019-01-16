return {
	include = function()
		includedirs { '../vendor/slikenet/Source/include/' }
		
		defines { 'RAKNET_SUPPORT_IPV6=1' }
	end,
	
	run = function()
		kind 'StaticLib'
		language 'C++'
		
		files_project '../vendor/slikenet/' {
			'Source/include/**.h',
			'Source/src/*.cpp'
		}
	end
}