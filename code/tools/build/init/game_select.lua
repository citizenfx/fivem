gamenames = { "ny", "payne", "server", "five", "launcher", "rdr3" }

newoption {
	trigger		= "game",
	value 		= "TARGET",
	description = "Choose a game to target",
	allowed 	= {
		{ "ny",		  "Grand Theft Auto IV" },
		{ "payne",	  "Max Payne 3" },
		{ "five",     "Grand Theft Auto V" },
		{ "rdr3",     "Bob" },
		{ "launcher", "reverse-game" },
		{ "server",   "CitizenFX server build" }
	}
}

newoption {
	trigger		= "tests",
	description	= "Enable building tests"
}

if not _OPTIONS['game'] then
	_OPTIONS['game'] = 'dummy'
end