return {
	include = function()
		includedirs { "../vendor/tinygltf/" }
		
		defines { "TINYGLTF_USE_CPP14", "TINYGLTF_NO_INCLUDE_JSON" }
	end,
	
	run = function()
		language 'C++'
		kind 'StaticLib'
		
		files {
			'vendor/tinygltf.cpp'
		}
	end
}