return function()
	configuration {}

	local wasmRuntime = path.getabsolute(_ROOTPATH .. '/../ext/wasm-runtime')

	filter { "configurations:Debug" }
		prebuildcommands {
			('cargo build --manifest-path "%s/Cargo.toml"'):format(
				wasmRuntime
			)
		}
		libdirs { wasmRuntime .. '/target/debug' }

	filter { "configurations:Release" }
		prebuildcommands {
			('cargo build --release --manifest-path "%s/Cargo.toml"'):format(
				wasmRuntime
			)
		}
		libdirs { wasmRuntime .. '/target/release' }

	filter {}

	links { 'cfx_component_glue' }
		
	includedirs { wasmRuntime .. '/glue/' }
end
