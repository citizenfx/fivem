if _OPTIONS['game'] ~= 'launcher' and _OPTIONS['game'] ~= 'rdr3' then
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
	
	includedirs { '../../../vendor/vulkan-headers/include/' }

	add_dependencies {
		"rage:input:rdr3",
		"rage:graphics:rdr3",
		"rage:nutsnbolts:rdr3",
	}
end