-- MSVC for GLM
defines { "_ENABLE_EXTENDED_ALIGNED_STORAGE" }

if os.istarget('windows') then
	filter { "configurations:Release" }
		flags { "LinkTimeOptimization" }
	
	filter {}
		buildoptions '/fp:fast'
end
