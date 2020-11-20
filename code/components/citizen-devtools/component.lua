return function()
	filter {}
	configuration {}

	if _OPTIONS['game'] ~= 'server' then
		add_dependencies { "citizen:resources:client" }
	end
	
	if _OPTIONS['game'] == 'five' then
		add_dependencies { 'gta:mission-cleanup', 'gta:streaming', 'devtools:five' }
	end
end