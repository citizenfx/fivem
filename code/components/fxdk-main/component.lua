return function()
	filter {}

	linkoptions "/DELAYLOAD:libcef.dll"

	libdirs { "../vendor/cef/Release/" }

	includedirs { "../vendor/cef/" }

	links { "libcef_dll", "delayimp", "libGLESv2" }
	links { "libcef" }
	add_dependencies { 'vendor:mojo' }

	filter {}
	
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
end
