return function()
	filter {}

	if _OPTIONS['game'] ~= 'server' then
		add_dependencies { "citizen:resources:client" }
	end

	if _OPTIONS['game'] == 'five' then
		add_dependencies { 'gta:mission-cleanup', 'gta:streaming', 'devtools:five' }
	end

	if _OPTIONS['game'] == 'rdr3' then
		add_dependencies { 'net', 'gta:core:rdr3' }
	end

	if _OPTIONS['game'] == 'ny' then
		add_dependencies { 'net' }
	end
end
