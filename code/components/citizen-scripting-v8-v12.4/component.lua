return function()
	filter {}
	defines { 
		'V8_12_2',
	}
	add_dependencies { 'vendor:v8-12.4' }

	if os.istarget('windows') then
		add_dependencies { 'vendor:minhook' }
	end
end
