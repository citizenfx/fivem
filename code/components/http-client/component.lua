dependency 'rage-device'

runtimechecks "Off"

return function()
	filter {}

	add_dependencies { 'net:tcp-server' }
end
