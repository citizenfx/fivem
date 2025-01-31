links { "ws2_32" }

return function()
	filter {}
	
	if _OPTIONS["game"] == 'ny' then
		add_dependencies {
			'ros-patches',
			'vendor:enet',
		}
	else
		add_dependencies {
			'ros-patches',
			"vendor:citizen_enet",
			"vendor:citizen_util",
		}
	end
end
