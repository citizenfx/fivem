return {
	include = function()
		defines { 'BGFX_CONFIG_MULTITHREADED=0' }
	
		includedirs { '../vendor/bx/include/', '../vendor/bgfx/include/', '../vendor/bgfx/examples/common/' }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"
		
		buildoptions '/MP'
		
		defines {
            "__STDC_LIMIT_MACROS",
            "__STDC_FORMAT_MACROS",
            "__STDC_CONSTANT_MACROS",
        }

        includedirs
        {
            '../vendor/bgfx/../bimg/include/',
            '../vendor/bgfx/3rdparty/',
            '../vendor/bgfx/3rdparty/khronos/',
            '../vendor/bgfx/3rdparty/dxsdk/include/',
            '../vendor/bx/3rdparty/',
            '../vendor/bimg/3rdparty/',
            '../vendor/bimg/3rdparty/iqa/include/',
            '../vendor/bimg/3rdparty/astc-codec/',
            '../vendor/bimg/3rdparty/astc-codec/include/',
            '../vendor/bimg/3rdparty/nvtt/',
        }

        files
        {
            '../vendor/bgfx/src/**.cpp',
            '../vendor/bgfx/src/**.h',
            '../vendor/bgfx/examples/common/font/**.cpp',
            '../vendor/bgfx/examples/common/cube_atlas.cpp',
            '../vendor/bx/src/**.cpp',
            '../vendor/bimg/src/**.cpp',
            "../vendor/bimg/3rdparty/astc-codec/src/decoder/*.cc",
            "../vendor/bimg/3rdparty/edtaa3/edtaa3func.cpp",
            "../vendor/bimg/3rdparty/astc/*.cpp",
            "../vendor/bimg/3rdparty/pvrtc/*.cpp",
            "../vendor/bimg/3rdparty/libsquish/*.cpp",
            "../vendor/bimg/3rdparty/etc1/*.cpp",
            "../vendor/bimg/3rdparty/etc2/*.cpp",
            "../vendor/bimg/3rdparty/nvtt/**.cpp",
            "../vendor/bimg/3rdparty/iqa/source/*.c",
        }

        removefiles
        {
			'../vendor/bgfx/src/amalgamated.cpp',
			'../vendor/bx/src/amalgamated.cpp',
            '../vendor/bgfx/src/**.bin.h',
        }

        filter 'configurations:Debug*'
            defines { 'BGFX_CONFIG_DEBUG=1' }

        filter 'action:vs*'
        	includedirs { "../vendor/bx/include/compat/msvc" }
	end
}