local isNode = ...

return function()
	defines { 'V8_VERSION=93' }
	add_dependencies { 'vendor:v8-93' }

	if os.istarget('windows') then
		add_dependencies { 'vendor:minhook' }
	end

	if isNode then
		defines { 'V8_NODE' }
		add_dependencies { 'vendor:node' }
	end
end
