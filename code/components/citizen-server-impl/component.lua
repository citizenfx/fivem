-- MSVC for GLM
defines { "_ENABLE_EXTENDED_ALIGNED_STORAGE" }

if os.istarget('windows') then
	filter { "configurations:Release" }
		linktimeoptimization "On"
	
	filter {}
		buildoptions '/fp:fast'
end

return function()
	filter {}
	add_dependencies { 'vendor:folly' }

	removefiles {
		'components/citizen-server-impl/src/state/**'
	}
end
