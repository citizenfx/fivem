links { "ws2_32" }

return function()
	filter {}
	
	add_dependencies { 'legitimacy' }

	if _OPTIONS["game"] == 'ny' then
		add_dependencies {
			'vendor:enet',
		}
	else
		add_dependencies {
			"vendor:citizen_enet",
			"vendor:citizen_util",
		}
	end
end
