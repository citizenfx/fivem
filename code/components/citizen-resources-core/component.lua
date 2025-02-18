-- don't infect any downstream dependencies
return function()
    add_dependencies {'vendor:eastl', 'net:base'}
end
