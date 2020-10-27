return function()
	if _OPTIONS['game'] == 'five' then
		filter {}
		configuration {}

		add_dependencies {
			"rage:input:five",
			"rage:graphics:five",
			"rage:nutsnbolts:five",
		}
	elseif _OPTIONS['game'] == 'rdr3' then
		filter {}
		configuration {}
		
		includedirs { '../vendor/vulkan-headers/include/' }

		add_dependencies {
			"rage:input:rdr3",
			"rage:graphics:rdr3",
			"rage:nutsnbolts:rdr3",
		}
	end

	filter {}
	configuration {}
	
	links { 'delayimp' }
	linkoptions '/DELAYLOAD:gdi32.dll'
end