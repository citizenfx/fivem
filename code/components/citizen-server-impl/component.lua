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
	add_dependencies { 'vendor:folly', 'vendor:lua' }

	removefiles {
		'components/citizen-server-impl/src/state/**'
	}
end
