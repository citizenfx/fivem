-- we don't want this to infect any of our dependants
return function()
	configuration {}
	filter {}
	
	if not os.istarget('windows') then
		defines { "BOTAN_DLL=" }
	else
		defines { "BOTAN_DLL=__declspec(dllimport)" }
	end

	if _OPTIONS['game'] ~= 'server' then
		includedirs "vendor/botan/include/"
	else
		includedirs "vendor/botan_sv/include/"
	end
	
	links "botan"
end