return function()
	filter {}
	configuration {}
	
	if _OPTIONS['game'] == 'five' then
		add_dependencies { 'gta:mission-cleanup', 'gta:streaming', 'devtools:five' }
	end
end