return function()
	filter {}
	configurations {}

	add_dependencies { 'vendor:wil', 'vendor:botan' }

	includedirs { "../vendor/AntiLag2-SDK/" }
end
