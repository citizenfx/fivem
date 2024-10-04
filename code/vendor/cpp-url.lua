return {
	include = function()
		includedirs { "../vendor/cpp-url/include/", "vendor/cpp-url/" }
	end,

	run = function()
		language "C++"
		kind "StaticLib"
		
		includedirs { "../vendor/cpp-url/src/", "../vendor/range-v3/include/" }
		
		if os.istarget('windows') then
			buildoptions '/permissive-'
		else
			buildoptions '-include algorithm'
		end

		files {
			"../vendor/cpp-url/src/**.cpp",
			"vendor/cpp-url/range_workaround.cpp"
		}
	end
}
