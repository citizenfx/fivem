return function()
	filter {}
	configuration {}
	
	add_dependencies { 'vendor:mbedtls', 'vendor:xz', 'vendor:hdiffpatch', 'vendor:openssl_crypto', 'vendor:tinyxml2-dll', 'ros-patches' }

	files {
		'client/launcher/GameCache.cpp',
		'client/launcher/Updater.cpp',
		'client/launcher/Download.cpp',
	}
end
