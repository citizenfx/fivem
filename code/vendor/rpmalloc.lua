return {
	include = function()
		includedirs { "../vendor/rpmalloc" }
	end,

	run = function()
		targetname "rpmalloc"
		language "C"
		kind "StaticLib"
		
		if os.istarget('windows') then
			flags { "LinkTimeOptimization" }
		end

		defines { 
			"ENABLE_THREAD_CACHE=1",
			"ENABLE_GLOBAL_CACHE=0",
		}

		files
		{
			"../vendor/rpmalloc/rpmalloc/**",
		}

		removefiles {
			"../vendor/rpmalloc/rpmalloc/version.c",
		}

		optimize "Speed"
	end
}