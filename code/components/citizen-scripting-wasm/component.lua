return function()
	filter {}
	configuration {}

	local wasmRuntime = path.getabsolute(_ROOTPATH .. '/../ext/wasm-runtime')

	-- TODO: no --release flag in debug builds
	prebuildcommands {
		('cargo build --release --manifest-path "%s/Cargo.toml"'):format(
			wasmRuntime
		)
	}

	libdirs { wasmRuntime .. '/target/release' }
	links { 'cfx_component_glue' }
		
	includedirs { wasmRuntime .. '/glue/' }
end
