return function()
	filter {}
	
	links 'CitiCore'
	files {
		'components/gta-core-rdr3/src/ErrorHandler.cpp'
	}
	add_dependencies { 'vendor:eastl' }
end
