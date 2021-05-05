return function()
	filter {}
	configuration {}

	local wasmRuntime = path.getabsolute(_ROOTPATH .. '/../vendor/fivem-wasm/target/release/')
	
	if os.isdir(wasmRuntime) then
		libdirs { wasmRuntime }
		links { 'cfx_component_glue' }
		
		includedirs { path.getabsolute(wasmRuntime .. '/../../glue/') }
	end
end
