return function()
	filter {}
	files {
		"components/rage-device-five/src/CitizenMount.Shared.cpp",
		"components/rage-device-five/src/CitizenMount.cpp"
	}
	add_dependencies { 'vendor:minhook', 'vendor:openssl_crypto' }
end
