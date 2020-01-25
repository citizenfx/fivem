return {
	include = function()
		includedirs { "../vendor/labsound/include", '../vendor/libnyquist/include/' }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"
		
		add_dependencies { 'vendor:opus' }
		
		defines {
			'__WINDOWS_WASAPI__',
			'MODPLUG_STATIC',
			'NOMINMAX',
			'STATICALLY_LINKED_WITH_WTF',
			'_CRT_SECURE_NO_WARNINGS',
			'_SCL_SECURE_NO_WARNINGS',
			'D_VARIADIC_MAX=10',
			'WTF_USE_WEBAUDIO_KISSFFT=1',
			'HAVE_NO_OFLOG',
			'HAVE_BOOST_THREAD',
			'HAVE_LIBDL',
			'HAVE_ALLOCA',
			'HAVE_UNISTD_H',
			'__OS_WINDOWS__',
			'__LITTLE_ENDIAN__'
		}
		
		files_project '../vendor/labsound/' {
			'src/backends/windows/*.cpp',
			'third_party/rtaudio/src/RtAudio.cpp',
			'third_party/STK/src/STKInlineCompile.cpp',
			'src/core/*.cpp',
			'src/extended/*.cpp',
			'src/internal/src/*.cpp',
			'third_party/kissfft/src/*.cpp',
			'third_party/ooura/src/*.cpp'
		}
		
		includedirs {
			'../vendor/labsound/src/',
			'../vendor/labsound/src/internal/',
			'../vendor/labsound/third_party/',
			'../vendor/labsound/third_party/STK/',
			'../vendor/libnyquist/include/',
		}
		
		includedirs {
			'../vendor/libnyquist/third_party/libogg/include',
			
			'../vendor/libnyquist/include/libnyquist',
			'../vendor/libnyquist/third_party',
			'../vendor/libnyquist/third_party/FLAC/src/include',
			'../vendor/libnyquist/third_party/libogg/include',
			'../vendor/libnyquist/third_party/libvorbis/include',
			'../vendor/libnyquist/third_party/libvorbis/src',
			'../vendor/libnyquist/third_party/musepack/include',
			'../vendor/libnyquist/third_party/opus/celt',
			'../vendor/libnyquist/third_party/opus/libopus/include',
			'../vendor/libnyquist/third_party/opus/opusfile/include',
			'../vendor/libnyquist/third_party/opus/opusfile/src/include',
			'../vendor/libnyquist/third_party/opus/silk',
			'../vendor/libnyquist/third_party/opus/silk/float',
			'../vendor/libnyquist/third_party/wavpack/include',
			'../vendor/libnyquist/src'
		}
		
		files_project '../vendor/libnyquist/' {
			'src/*.cpp',
			'src/*.c',
			'third_party/wavpack/src/*.c',
			'third_party/opus/opusfile/src/*.c'
		}
		
		removefiles {
			'../vendor/libnyquist/src/OpusDependencies.c'
		}
	end
}