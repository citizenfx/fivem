-- MSVC for GLM
defines { "_ENABLE_EXTENDED_ALIGNED_STORAGE" }

if os.istarget('windows') then
	filter { "configurations:Release" }
		flags { "LinkTimeOptimization" }
	
	filter {}
		buildoptions '/fp:fast'
end

return function()
	filter {}
	configuration {}
	
	add_dependencies { 'vendor:folly' }
	
	local jexlEvalDir = path.getabsolute(_ROOTPATH .. '/../ext/jexl-eval/target/release/')
	
	if os.isdir(jexlEvalDir) then
		libdirs { jexlEvalDir }
		links { 'cfx_jexl_eval' }
		
		includedirs { path.getabsolute(jexlEvalDir .. '/../../') }
	end
end