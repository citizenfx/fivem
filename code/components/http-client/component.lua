dependency 'rage-device'

flags { 'NoRuntimeChecks' }

return function()
	filter {}

	add_dependencies { 'net:tcp-server' }
end
