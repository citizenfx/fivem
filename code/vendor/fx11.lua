return {
	include = function()
		includedirs { "../vendor/fx11/inc/" }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"

		includedirs { "../vendor/fx11/", "../vendor/fx11/Binary/" }
		
		files_project "../vendor/fx11/" {
			"d3dxGlobal.cpp",
			"EffectAPI.cpp",
			"EffectLoad.cpp",
			"EffectNonRuntime.cpp",
			"EffectReflection.cpp",
			"EffectRuntime.cpp",
			"Effect.h",
			"EffectLoad.h",
			"IUnknownImp.h",
			"EffectVariable.inl",
			"Binary/EffectBinaryFormat.h",
			"Binary/EffectStateBase11.h",
			"Binary/EffectStates11.h",
			"Binary/SOParser.h",
			"inc/d3dx11effect.h",
			"inc/d3dxGlobal.h",
		}
	end
}
