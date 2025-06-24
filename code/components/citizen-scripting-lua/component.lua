-- Preprocessor definitions that carryover from lua54.lua
defines { 
	"GRIT_POWER_BLOB", -- Enable 'raw string' usage
	
	--[[
		Experimental Intrinsics  
		See: citizen-server-impl/include/state/ServerGameState.h
	--]]
	'GLM_FORCE_DEFAULT_ALIGNED_GENTYPES',
	'GLM_FORCE_SSE2',
	--'GLM_FORCE_SSE3',
}

if os.istarget('windows') and not _OPTIONS['with-asan'] then
	flags { "LinkTimeOptimization" }
	
	buildoptions '/Zc:threadSafeInit- /EHa /fp:fast'
end