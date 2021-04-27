return {
	include = function()
		if _OPTIONS["game"] == "ny" then
			defines { "XENIUM_ARCH_X86" }
		end

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