return function()
	filter {}
	
	links 'CitiCore'
	files {
		'components/gta-core-rdr3/src/ErrorHandler.cpp',
		'components/gta-core-rdr3/src/SimpleAllocator.cpp'
	}
	add_dependencies { 'vendor:eastl' }
end
