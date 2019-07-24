-- we don't want this to infect any of our dependants
return function()
	configuration {}
	filter {}
	
	if not os.istarget('windows') then
		defines { "BOTAN_DLL=" }
	else
		defines { "BOTAN_DLL=__declspec(dllimport)" }
	end

	includedirs "vendor/botan/include/"
	
	links "botan"
end