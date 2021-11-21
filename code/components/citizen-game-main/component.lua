return function()
	filter {}

	add_dependencies { 'vendor:tbb' }
	
	includedirs {
		'components/vfs-impl-server/include/', '../vendor/bgfx/examples/common/'
	}

	files {
		'components/vfs-impl-server/src/**.cpp',
		'components/vfs-impl-server/include/**.h',
	}

	removefiles {
		'components/vfs-impl-server/src/Component.cpp',
	}

	filter 'action:vs*'
		includedirs { "../vendor/bx/include/compat/msvc" }

	links { 'psapi' }
end
