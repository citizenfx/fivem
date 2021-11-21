
return function()
	if _OPTIONS['game'] == 'five' or _OPTIONS['game'] == 'rdr3' or _OPTIONS['game'] == 'ny' then
		filter {}

		add_dependencies {
			"rage:input",
			"rage:graphics",
			"rage:nutsnbolts",
		}
	end
	
	if _OPTIONS['game'] == 'rdr3' then
		filter {}
		
		includedirs { '../vendor/vulkan-headers/include/' }
	end

	filter {}
	
	links { 'delayimp' }
	linkoptions '/DELAYLOAD:gdi32.dll'
end
