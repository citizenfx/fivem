dependency 'rage-device'

flags { 'NoRuntimeChecks' }

return function()
	filter {}
	configuration {}

	add_dependencies { 'net:tcp-server' }
end