return function()
	configuration {}
	filter {}
	
	includedirs {
		'components/vfs-impl-server/include/'
	}

	files {
		'components/vfs-impl-server/src/PlatformDevice.Win32.cpp',
		'components/vfs-impl-server/src/MemoryDevice.cpp',
		'components/vfs-impl-server/include/**.h',
	}
	
	removefiles {
		'components/vfs-impl-server/src/Component.cpp',
	}
end