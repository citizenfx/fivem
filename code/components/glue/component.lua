return function()
	filter {}
	
	add_dependencies { 'vendor:mbedtls', 'vendor:xz', 'vendor:zstd', 'vendor:hdiffpatch', 'vendor:openssl_crypto', 'vendor:tinyxml2-dll', 'ros-patches', 'vendor:botan' }

	if _OPTIONS['game'] == 'five' then
		add_dependencies { 'gta:streaming:five' }
	end

	files {
		'client/launcher/GameCache.cpp',
		'client/launcher/Updater.cpp',
		'client/launcher/Download.cpp',
		'client/launcher/InstallerExtraction.cpp',
	}
end
