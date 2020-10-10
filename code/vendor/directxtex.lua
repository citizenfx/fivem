return {
	include = function()
		includedirs { "../vendor/directxtex/" }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"
		
		files_project "../vendor/directxtex/" {
			"DirectXTex/BC.h",
			"DirectXTex/BCDirectCompute.h",
			"DirectXTex/d3dx12.h",
			"DirectXTex/DDS.h",
			"DirectXTex/DirectXTex.h",
			"DirectXTex/DirectXTexP.h",
			"DirectXTex/filters.h",
			"DirectXTex/scoped.h",
			"DirectXTex/BC.cpp",
			"DirectXTex/BC4BC5.cpp",
			"DirectXTex/BC6HBC7.cpp",
			--"DirectXTex/BCDirectCompute.cpp",
			"DirectXTex/DirectXTexCompress.cpp",
			"DirectXTex/DirectXTexCompressGPU.cpp",
			"DirectXTex/DirectXTexConvert.cpp",
			"DirectXTex/DirectXTexDDS.cpp",
			"DirectXTex/DirectXTexFlipRotate.cpp",
			"DirectXTex/DirectXTexHDR.cpp",
			"DirectXTex/DirectXTexImage.cpp",
			"DirectXTex/DirectXTexMipmaps.cpp",
			"DirectXTex/DirectXTexMisc.cpp",
			"DirectXTex/DirectXTexNormalMaps.cpp",
			"DirectXTex/DirectXTexPMAlpha.cpp",
			"DirectXTex/DirectXTexResize.cpp",
			"DirectXTex/DirectXTexTGA.cpp",
			"DirectXTex/DirectXTexUtil.cpp",
			"DirectXTex/DirectXTexWIC.cpp",
		}
	end
}