if _OPTIONS['game'] ~= 'launcher' then
	filter {}
	configuration {}

	add_dependencies {
		"rage:input:five",
		"rage:graphics:five",
		"rage:nutsnbolts:five",
	}
end