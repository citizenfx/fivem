if not os.is('windows') then
	links { 'crypto', 'ssl' }
end

-- don't infect any downstream dependencies
return function()
	add_dependencies { 'vendor:eastl' }
end