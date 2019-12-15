return function()
	configuration {}
	filter {}

	includedirs {
		'components/vfs-impl-server/include/'
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
end