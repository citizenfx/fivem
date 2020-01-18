-- MSVC for GLM
defines { "_ENABLE_EXTENDED_ALIGNED_STORAGE" }

if os.istarget('windows') then
	flags { "LinkTimeOptimization" }
	
	buildoptions '/fp:fast'
end