libdirs { 'lib' }

links { 'ws2_32', 'avutil', 'swresample' }

return function()
	add_dependencies { 'vendor:rnnoise' }
end
