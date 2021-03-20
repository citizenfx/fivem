
return function()
	if _OPTIONS['game'] == 'five' or _OPTIONS['game'] == 'rdr3' or _OPTIONS['game'] == 'ny' then
		filter {}
		configuration {}

		add_dependencies {
			"rage:input",
			"rage:graphics",
			"rage:nutsnbolts",
		}
	end
	
	if _OPTIONS['game'] == 'rdr3' then
		filter {}
		configuration {}
		
		includedirs { '../vendor/vulkan-headers/include/' }
	end

	filter {}
	configuration {}
	
	links { 'delayimp' }
	linkoptions '/DELAYLOAD:gdi32.dll'
end
