gamenames = { "ny", "payne", "server" }

newoption {
	trigger		= "game",
	value 		= "TARGET",
	description = "Choose a game to target",
	allowed 	= {
		{ "ny",		"Grand Theft Auto IV" },
		{ "payne",	"Max Payne 3" },
		{ "server", "CitizenFX server build" }
	}
}

newoption {
	trigger		= "tests",
	description	= "Enable building tests"
}

if not _OPTIONS['game'] then
	_OPTIONS['game'] = 'dummy'
end