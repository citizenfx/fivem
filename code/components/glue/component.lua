return function()
	filter {}
	configuration {}
	
	add_dependencies { 'vendor:mbedtls', 'vendor:xz', 'vendor:hdiffpatch', 'vendor:openssl_crypto' }

	files {
		'client/launcher/GameCache.cpp',
		'client/launcher/Updater.cpp',
		'client/launcher/Download.cpp',
	}
end