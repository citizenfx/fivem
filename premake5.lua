local gamenames = { "ny", "payne", "server" }

newoption {
	trigger		= "game",
	value 		= "TARGET",
	description = "Choose a game to target",
	allowed 	= {
		{ "ny",		"Grand Theft Auto IV" },
		{ "payne",	"Max Payne 3" },
		{ "server", "CitizenFX server build" }
	}
}

newoption {
	trigger		= "tests",
	description	= "Enable building tests"
}

if not _OPTIONS['game'] then
	_OPTIONS['game'] = 'dummy'
end

-- final override for files to exclude platform-specific files
local origFiles = files

files = function(...)
	origFiles(...)

	configuration "windows"
		excludes { "**/*.Posix.cpp" }

	configuration "not windows"
		excludes { "**/*.Win32.cpp" }

	-- reset configuration
	configuration {}
end

-- override for GCC-style CXXFLAGS
local getcxxflags = premake.tools.gcc.getcxxflags;
function premake.tools.gcc.getcxxflags(cfg)
    local r = getcxxflags(cfg)

    table.insert(r, "-std=c++14")
    table.insert(r, "-stdlib=libc++")
    table.insert(r, "-Xclang") -- to enable the following option
	table.insert(r, "-fno-sized-deallocation") -- C++14 mode in clang causes sized deallocation operator delete to be referenced, but we don't want to depend on latest libc++

    return r
end

local config = premake.config

function premake.tools.gcc.ldflags.kind.SharedLib(cfg)
	local r = { iif(cfg.system == premake.MACOSX, "-dynamiclib", "-shared") }
	if cfg.system == "windows" and not cfg.flags.NoImportLib then
		table.insert(r, '-Wl,--out-implib="' .. cfg.linktarget.relpath .. '"')
	elseif cfg.system == premake.LINUX then
		table.insert(r, '-Wl,-soname="' .. cfg.linktarget.name .. '"')
	elseif cfg.system == premake.MACOSX then
		table.insert(r, '-Wl,-install_name,"@rpath/' .. cfg.linktarget.name .. '"')
	end
	return r
end

function premake.tools.gcc.getlinks(cfg, systemonly)
	local result = {}

	-- Don't use the -l form for sibling libraries, since they may have
	-- custom prefixes or extensions that will confuse the linker. Instead
	-- just list out the full relative path to the library.

	if not systemonly then
		local siblings = config.getlinks(cfg, "siblings", "fullpath")
		local sharedlibextension = ".so"
		sharedlibextension = iif(cfg.system == premake.WINDOWS, ".dll", sharedlibextension)
		sharedlibextension = iif(cfg.system == premake.MACOSX, ".dylib", sharedlibextension)
		for _, sibling in ipairs(siblings) do
			if path.getextension(sibling) == sharedlibextension then
				local fullpath = path.getabsolute(path.rebase(path.getdirectory(sibling), cfg.location, os.getcwd()))
				local rpath = path.getrelative(cfg.targetdir, fullpath)
				if cfg.system == premake.LINUX then
					rpath = iif(rpath == ".", "", "/" .. rpath)
					rpath = " -Wl,-rpath,'$$ORIGIN" .. rpath .. "'"
				elseif cfg.system == premake.MACOSX then
					rpath = " -Wl,-rpath,'@loader_path/" .. rpath .. "'"
				else
					rpath = ""
				end
				if (#rpath > 0) and not table.contains(result, rpath) then
					table.insert(result, rpath)
				end
			end

			table.insert(result, sibling)
		end
	end

	-- The "-l" flag is fine for system libraries

	local links = config.getlinks(cfg, "system", "fullpath")
	for _, link in ipairs(links) do
		if path.isframework(link) then
			table.insert(result, "-framework " .. path.getbasename(link))
		elseif path.isobjectfile(link) then
			table.insert(result, link)
		else
			table.insert(result, "-l" .. path.getbasename(link))
		end
	end

	return result
end

local function files_project(name)
	return function(f)
		local t = {}

		for k, file in ipairs(f) do
			table.insert(t, name .. file)
		end

		files(t)
	end
end

solution "CitizenMP"
	configurations { "Debug", "Release" }

	flags { "No64BitChecks", "Symbols", "Unicode" }
	
	flags { "NoIncrementalLink", "NoEditAndContinue" } -- this breaks our custom section ordering in citilaunch, and is kind of annoying otherwise
	
	includedirs { "shared/", "client/shared/", "../vendor/jitasm/", "deplibs/include/", "../vendor/gmock/include/", "../vendor/gtest/include", os.getenv("BOOST_ROOT") }
	
	defines { "GTEST_HAS_PTHREAD=0", "BOOST_ALL_NO_LIB" }

	libdirs { "deplibs/lib/" }

	location ("build/" .. _OPTIONS['game'])

	if _OPTIONS['game'] == 'server' then
		location ("build/server/" .. os.get())
	end
	
	configuration "Debug*"
		targetdir ("bin/" .. _OPTIONS['game'] .. "/debug")
		defines "NDEBUG"

		defines { '_ITERATOR_DEBUG_LEVEL=0' }

		if _OPTIONS['game'] == 'server' then
			targetdir ("bin/server/" .. os.get() .. "/debug")
		end
		
	configuration "Release*"
		targetdir ("bin/" .. _OPTIONS['game'] .. "/release")
		defines "NDEBUG"
		optimize "Speed"

		if _OPTIONS['game'] == 'server' then
			targetdir ("bin/server/" .. os.get() .. "/release")
		end
		
	configuration "game=ny"
		defines "GTA_NY"

	configuration "game=payne"
		defines "PAYNE"

	configuration "windows"
		links { "winmm" }

	configuration "not windows"
		buildoptions {
			"-fPIC", -- required to link on AMD64
		}

		links { "c++" }

if _OPTIONS['game'] ~= 'server' then
	project "CitiLaunch"
		language "C++"
		kind "WindowedApp"
		
		defines "COMPILING_LAUNCH"
		
		links { "SharedLibc", "dbghelp", "psapi", "libcurl", "tinyxml2", "liblzma", "comctl32", "breakpad", "wininet", "winhttp" }
		
		files
		{
			"client/launcher/**.cpp", "client/launcher/**.h", 
			"client/launcher/launcher.rc", "client/launcher/launcher.def",
			"client/common/Error.cpp"
		}
		
		pchsource "client/launcher/StdInc.cpp"
		pchheader "StdInc.h"
		
		libdirs { "client/libcef/lib/" }
		
		linkoptions "/DELAYLOAD:libcef.dll"
		
		includedirs { "client/libcef/", "../vendor/breakpad/src/", "../vendor/tinyxml2/" }

		flags { "StaticRuntime" }

		configuration "game=ny"
			targetname "CitizenFX"

		configuration "game=payne"
			targetname "CitizenPayne"
		
		configuration "Debug*"
			links { "libcefd" }
			
		configuration "Release*"
			links { "libcef" }
		
		configuration "windows"
			linkoptions "/ENTRY:main /IGNORE:4254 /DYNAMICBASE:NO /SAFESEH:NO /LARGEADDRESSAWARE" -- 4254 is the section type warning we tend to get
else
	project "DuplicityMain"
		language "C++"
		kind "ConsoleApp"

		links { "Shared", "CitiCore" }

		includedirs
		{
			"client/citicore/",
			"server/launcher/include/"
		}

		files
		{
			"server/launcher/**.cpp", "server/launcher/**.h"
		}

		pchsource "server/launcher/src/StdInc.cpp"
		pchheader "StdInc.h"

		targetname "FXServer"
end
		
	project "CitiCore"
		targetname "CoreRT" 
		language "C++"
		kind "SharedLib"

		files
		{
			"client/citicore/**.cpp", "client/citicore/**.h", "client/common/Error.cpp", "client/common/StdInc.cpp"
		}

		links { "Shared" }

		defines "COMPILING_CORE"

		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"

		configuration "not windows"
			links { "dl", "c++" }

if _OPTIONS['game'] ~= 'server' then
	project "CitiGame"
		targetname "CitizenGame"
		language "C++"
		kind "SharedLib"
		
		files
		{
			"client/citigame/**.cpp", "client/citigame/**.h", "client/common/Error.cpp", "client/citigame/**.c", "client/common/StdInc.cpp"
		}
		
		links { "Shared", "citicore" }
		
		defines "COMPILING_GAME"
		
		includedirs { "client/citigame/include/", "components/nui-core/include/", "components/downloadmgr/include/", "components/net/include/", "client/citicore/", "components/resources/include/", "components/http-client/include/", "../vendor/luajit/src/", "../vendor/yaml-cpp/include/", "../vendor/msgpack-c/include/", "deplibs/include/msgpack-c/", "client/libcef/", "client/shared/np" }
		
		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
		
		configuration "game=ny"
			includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "client/citigame/net/" }
			links { "HooksNY", "GameNY" }

			-- temp, until CitiGame is gone
			links { "rage-nutsnbolts-" .. _OPTIONS['game'], "http-client", "net", "resources", "downloadmgr", "nui-core" }

			includedirs { "components/rage-nutsnbolts-" .. _OPTIONS['game'] .. "/include/" }
end

	local buildHost = os.getenv("COMPUTERNAME") or 'dummy'

	--[[if buildHost == 'FALLARBOR' then
		project "CitiMono"
			targetname "CitizenFX.Core"
			language "C#"
			kind "SharedLib"

			files { "client/clrcore/**.cs" }

			links { "System" }

			configuration "Debug*"
				targetdir "bin/debug/citizen/clr/lib/mono/4.5"

			configuration "Release*"
				targetdir "bin/release/citizen/clr/lib/mono/4.5"
	end]]

group "managed"

if _OPTIONS['game'] ~= 'server' and buildHost == 'FALLARBOR' then
	external 'CitiMono'
		uuid 'E781BFF9-D34E-1A05-FC67-08ADE8934F93'
		kind 'SharedLib'
		language 'C#'
		location '.'

	external '010.Irony.2010'
		uuid 'D81F5C91-D7DB-46E5-BC99-49488FB6814C'
		kind 'SharedLib'
		language 'C#'
		location '../vendor/pash/Libraries/Irony/Irony/'

	external 'System.Management'
		uuid 'C5E303EC-5684-4C95-B0EC-2593E6662403'
		kind 'SharedLib'
		language 'C#'
		location '../vendor/pash/Source/System.Management/'

	external 'Microsoft.PowerShell.Commands.Utility'
		uuid '0E1D573C-C57D-4A83-A739-3A38E719D87E'
		kind 'SharedLib'
		language 'C#'
		location '../vendor/pash/Source/Microsoft.PowerShell.Commands.Utility/'
end
			
	if _OPTIONS['game'] == 'ny' then
		project "GameNY"
			targetname "game_ny"
			language "C++"
			kind "SharedLib"
			
			links { "Shared", "zlib", "CitiCore" }
			
			defines "COMPILING_GAMESPEC"
			
			pchsource "client/common/StdInc.cpp"
			pchheader "StdInc.h"
			
			configuration "game=ny"
				includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "../vendor/zlib/" }
			
				files
				{
					"client/game_ny/**.cpp", "client/game_ny/**.h", "client/common/Error.cpp", "client/common/StdInc.cpp"
				}
			
		project "HooksNY"
			targetname "hooks_ny"
			language "C++"
			kind "SharedLib"
			
			links { "Shared", "GameNY", "ws2_32", "rage-graphics-ny", "gta-core-ny", "CitiCore" }
			
			defines "COMPILING_HOOKS"	

			pchsource "client/hooks_ny/base/StdInc.cpp"
			pchheader "StdInc.h"		
			
			configuration "game=ny"
				includedirs { "components/gta-core-ny/include", "components/rage-graphics-ny/include", "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "client/hooks_ny/base/" }
				
				files
				{
					"client/hooks_ny/**.cpp", "client/hooks_ny/**.h", "client/common/Error.cpp"
				}
	end
		
	group ""

	project "Shared"
		targetname "shared"
		language "C++"
		kind "StaticLib"

		defines "COMPILING_SHARED"
		
		--includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/" }
		
		files
		{
			"shared/**.cpp", "shared/**.h", "client/shared/**.cpp", "client/shared/**.h"
		}

		configuration "not windows"
			excludes { "**/Hooking.*" }

	project "SharedLibc"
		targetname "shared_libc"
		language "C++"
		kind "StaticLib"

		flags { "StaticRuntime" }

		defines { "COMPILING_SHARED", "COMPILING_SHARED_LIBC" }
		
		--includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/" }
		
		files
		{
			"shared/**.cpp", "shared/**.h", "client/shared/**.cpp", "client/shared/**.h"
		}

		configuration "not windows"
			excludes { "**/Hooking.*" }

	group "vendor"
		
if _OPTIONS['game'] ~= 'server' then
	project "libcef_dll"
		targetname "libcef_dll_wrapper"
		language "C++"
		kind "StaticLib"
		
		defines { "USING_CEF_SHARED", "NOMINMAX", "WIN32" }
		
		flags { "NoIncrementalLink", "NoMinimalRebuild" }
		
		includedirs { ".", "client/libcef" }
		
		buildoptions "/MP"
		
		files
		{
			"client/libcef/libcef_dll/**.cc", "client/libcef/libcef_dll/**.cpp", "client/libcef/libcef_dll/**.h"
		}
end
		
	project "yaml-cpp"
		targetname "yaml-cpp"
		language "C++"
		kind "StaticLib"
		
		includedirs { "../vendor/yaml-cpp/include" }
		
		files
		{
			"../vendor/yaml-cpp/src/*.cpp"
		}

	project "msgpack-c"
		targetname "msgpack-c"
		language "C++"
		kind "StaticLib"

		includedirs { "../vendor/msgpack-c/src", "../vendor/msgpack-c/include/", "deplibs/include/msgpack-c/" }

		files
		{
			"../vendor/msgpack-c/src/*.c" 
		}

	project "protobuf_lite"
		targetname "protobuf_lite"
		language "C++"
		kind "StaticLib"

		includedirs { "../vendor/protobuf/src/", "../vendor/protobuf/vsprojects/" }

		configuration "windows"
			buildoptions "/MP /wd4244 /wd4267 /wd4018 /wd4355 /wd4800 /wd4251 /wd4996 /wd4146 /wd4305"

		files
		{
			"../vendor/protobuf/src/google/protobuf/io/coded_stream.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/common.cc",
			"../vendor/protobuf/src/google/protobuf/extension_set.cc",
			"../vendor/protobuf/src/google/protobuf/generated_message_util.cc",
			"../vendor/protobuf/src/google/protobuf/message_lite.cc",
			"../vendor/protobuf/src/google/protobuf/arena.cc",
			"../vendor/protobuf/src/google/protobuf/arenastring.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/once.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc",
			"../vendor/protobuf/src/google/protobuf/repeated_field.cc",
			"../vendor/protobuf/src/google/protobuf/wire_format_lite.cc",
			"../vendor/protobuf/src/google/protobuf/io/zero_copy_stream.cc",
			"../vendor/protobuf/src/google/protobuf/io/zero_copy_stream_impl_lite.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/stringprintf.cc"
		}

	project "zlib"
		targetname "zlib"
		language "C"
		kind "StaticLib"

		files { "../vendor/zlib/*.c", "../vendor/zlib/*.h" }
		excludes { "../vendor/zlib/example.c", "../vendor/zlib/minigzip.c" }

		configuration "windows"
			defines { "WIN32" }

	project "opus"
		targetname "opus"
		language "C"
		kind "StaticLib"

		defines { 'USE_ALLOCA', 'inline=__inline', 'OPUS_BUILD' }

		includedirs {
			'../vendor/libopus/win32/',
			'../vendor/libopus/src/',
			'../vendor/libopus/celt/',
			'../vendor/libopus/silk/',
			'../vendor/libopus/silk/float/',
			'../vendor/libopus/include/',
			'deplibs/libopus/'
		}

		files {
			'../vendor/libopus/src/analysis.c',
			'../vendor/libopus/src/mlp.c',
			'../vendor/libopus/src/mlp_data.c',
	        '../vendor/libopus/src/opus.c',
	        '../vendor/libopus/src/opus_decoder.c',
	        '../vendor/libopus/src/opus_encoder.c',
	        '../vendor/libopus/src/repacketizer.c',
	        '../vendor/libopus/celt/bands.c',
	        '../vendor/libopus/celt/celt.c',
	        '../vendor/libopus/celt/celt_lpc.c',
	        '../vendor/libopus/celt/cwrs.c',
	        '../vendor/libopus/celt/entcode.c',
	        '../vendor/libopus/celt/celt_decoder.c',
	        '../vendor/libopus/celt/celt_encoder.c',
	        '../vendor/libopus/celt/entdec.c',
	        '../vendor/libopus/celt/entenc.c',
	        '../vendor/libopus/celt/kiss_fft.c',
	        '../vendor/libopus/celt/laplace.c',
	        '../vendor/libopus/celt/mathops.c',
	        '../vendor/libopus/celt/mdct.c',
	        '../vendor/libopus/celt/modes.c',
	        '../vendor/libopus/celt/pitch.c',
	        '../vendor/libopus/celt/quant_bands.c',
	        '../vendor/libopus/celt/rate.c',
	        '../vendor/libopus/celt/vq.c',
	        '../vendor/libopus/silk/A2NLSF.c',
	        '../vendor/libopus/silk/ana_filt_bank_1.c',
	        '../vendor/libopus/silk/biquad_alt.c',
	        '../vendor/libopus/silk/bwexpander.c',
	        '../vendor/libopus/silk/bwexpander_32.c',
	        '../vendor/libopus/silk/check_control_input.c',
	        '../vendor/libopus/silk/CNG.c',
	        '../vendor/libopus/silk/code_signs.c',
	        '../vendor/libopus/silk/control_audio_bandwidth.c',
	        '../vendor/libopus/silk/control_codec.c',
	        '../vendor/libopus/silk/control_SNR.c',
	        '../vendor/libopus/silk/debug.c',
	        '../vendor/libopus/silk/decode_core.c',
	        '../vendor/libopus/silk/decode_frame.c',
	        '../vendor/libopus/silk/decode_indices.c',
	        '../vendor/libopus/silk/decode_parameters.c',
	        '../vendor/libopus/silk/decode_pitch.c',
	        '../vendor/libopus/silk/decode_pulses.c',
	        '../vendor/libopus/silk/decoder_set_fs.c',
	        '../vendor/libopus/silk/dec_API.c',
	        '../vendor/libopus/silk/enc_API.c',
	        '../vendor/libopus/silk/encode_indices.c',
	        '../vendor/libopus/silk/encode_pulses.c',
	        '../vendor/libopus/silk/gain_quant.c',
	        '../vendor/libopus/silk/HP_variable_cutoff.c',
	        '../vendor/libopus/silk/init_decoder.c',
	        '../vendor/libopus/silk/init_encoder.c',
	        '../vendor/libopus/silk/inner_prod_aligned.c',
	        '../vendor/libopus/silk/interpolate.c',
	        '../vendor/libopus/silk/lin2log.c',
	        '../vendor/libopus/silk/log2lin.c',
	        '../vendor/libopus/silk/LPC_analysis_filter.c',
	        '../vendor/libopus/silk/LPC_inv_pred_gain.c',
	        '../vendor/libopus/silk/LP_variable_cutoff.c',
	        '../vendor/libopus/silk/NLSF2A.c',
	        '../vendor/libopus/silk/NLSF_decode.c',
	        '../vendor/libopus/silk/NLSF_encode.c',
	        '../vendor/libopus/silk/NLSF_del_dec_quant.c',
	        '../vendor/libopus/silk/NLSF_stabilize.c',
	        '../vendor/libopus/silk/NLSF_unpack.c',
	        '../vendor/libopus/silk/NLSF_VQ.c',
	        '../vendor/libopus/silk/NLSF_VQ_weights_laroia.c',
	        '../vendor/libopus/silk/NSQ.c',
	        '../vendor/libopus/silk/NSQ_del_dec.c',
	        '../vendor/libopus/silk/pitch_est_tables.c',
	        '../vendor/libopus/silk/PLC.c',
	        '../vendor/libopus/silk/process_NLSFs.c',
	        '../vendor/libopus/silk/quant_LTP_gains.c',
	        '../vendor/libopus/silk/resampler.c',
	        '../vendor/libopus/silk/resampler_down2.c',
	        '../vendor/libopus/silk/resampler_down2_3.c',
	        '../vendor/libopus/silk/resampler_private_AR2.c',
	        '../vendor/libopus/silk/resampler_private_down_FIR.c',
	        '../vendor/libopus/silk/resampler_private_IIR_FIR.c',
	        '../vendor/libopus/silk/resampler_private_up2_HQ.c',
	        '../vendor/libopus/silk/resampler_rom.c',
	        '../vendor/libopus/silk/shell_coder.c',
	        '../vendor/libopus/silk/sigm_Q15.c',
	        '../vendor/libopus/silk/sort.c',
	        '../vendor/libopus/silk/stereo_decode_pred.c',
	        '../vendor/libopus/silk/stereo_encode_pred.c',
	        '../vendor/libopus/silk/stereo_find_predictor.c',
	        '../vendor/libopus/silk/stereo_LR_to_MS.c',
	        '../vendor/libopus/silk/stereo_MS_to_LR.c',
	        '../vendor/libopus/silk/stereo_quant_pred.c',
	        '../vendor/libopus/silk/sum_sqr_shift.c',
	        '../vendor/libopus/silk/table_LSF_cos.c',
	        '../vendor/libopus/silk/tables_gain.c',
	        '../vendor/libopus/silk/tables_LTP.c',
	        '../vendor/libopus/silk/tables_NLSF_CB_NB_MB.c',
	        '../vendor/libopus/silk/tables_NLSF_CB_WB.c',
	        '../vendor/libopus/silk/tables_other.c',
	        '../vendor/libopus/silk/tables_pitch_lag.c',
	        '../vendor/libopus/silk/tables_pulses_per_block.c',
	        '../vendor/libopus/silk/VAD.c',
	        '../vendor/libopus/silk/VQ_WMat_EC.c',
	        '../vendor/libopus/silk/float/apply_sine_window_FLP.c',
	        '../vendor/libopus/silk/float/autocorrelation_FLP.c',
	        '../vendor/libopus/silk/float/burg_modified_FLP.c',
	        '../vendor/libopus/silk/float/bwexpander_FLP.c',
	        '../vendor/libopus/silk/float/corrMatrix_FLP.c',
	        '../vendor/libopus/silk/float/encode_frame_FLP.c',
	        '../vendor/libopus/silk/float/energy_FLP.c',
	        '../vendor/libopus/silk/float/find_LPC_FLP.c',
	        '../vendor/libopus/silk/float/find_LTP_FLP.c',
	        '../vendor/libopus/silk/float/find_pitch_lags_FLP.c',
	        '../vendor/libopus/silk/float/find_pred_coefs_FLP.c',
	        '../vendor/libopus/silk/float/inner_product_FLP.c',
	        '../vendor/libopus/silk/float/k2a_FLP.c',
	        '../vendor/libopus/silk/float/levinsondurbin_FLP.c',
	        '../vendor/libopus/silk/float/LPC_analysis_filter_FLP.c',
	        '../vendor/libopus/silk/float/LPC_inv_pred_gain_FLP.c',
	        '../vendor/libopus/silk/float/LTP_analysis_filter_FLP.c',
	        '../vendor/libopus/silk/float/LTP_scale_ctrl_FLP.c',
	        '../vendor/libopus/silk/float/noise_shape_analysis_FLP.c',
	        '../vendor/libopus/silk/float/pitch_analysis_core_FLP.c',
	        '../vendor/libopus/silk/float/prefilter_FLP.c',
	        '../vendor/libopus/silk/float/process_gains_FLP.c',
	        '../vendor/libopus/silk/float/regularize_correlations_FLP.c',
	        '../vendor/libopus/silk/float/residual_energy_FLP.c',
	        '../vendor/libopus/silk/float/scale_copy_vector_FLP.c',
	        '../vendor/libopus/silk/float/scale_vector_FLP.c',
	        '../vendor/libopus/silk/float/schur_FLP.c',
	        '../vendor/libopus/silk/float/solve_LS_FLP.c',
	        '../vendor/libopus/silk/float/sort_FLP.c',
	        '../vendor/libopus/silk/float/warped_autocorrelation_FLP.c',
	        '../vendor/libopus/silk/float/wrappers_FLP.c',
		}
		
	project "gtest_main"
		language "C++"
		kind "StaticLib"

		includedirs { "../vendor/gtest/" }

		files { "../vendor/gtest/src/gtest-all.cc", "../vendor/gtest/src/gtest_main.cc" }

	project "gmock_main"
		language "C++"
		kind "StaticLib"
		
		includedirs { "../vendor/gmock/", "../vendor/gtest/" }
		files { "../vendor/gmock/src/gmock-all.cc", "../vendor/gmock/src/gmock_main.cc" }

	project "libuv"
		language "C"
		kind "SharedLib"

		includedirs { "../vendor/libuv/include/", "../vendor/libuv/src/" }

		files_project '../vendor/libuv/'
		{
			'include/uv.h',
			'include/tree.h',
			'include/uv-errno.h',
			'include/uv-threadpool.h',
			'include/uv-version.h',
			'src/fs-poll.c',
			'src/heap-inl.h',
			'src/inet.c',
			'src/queue.h',
			'src/threadpool.c',
			'src/uv-common.c',
			'src/uv-common.h',
			'src/version.c'
		}

		defines { 'BUILDING_UV_SHARED=1' }

		filter "system:not windows"
			defines { "_LARGEFILE_SOURCE", "_FILEOFFSET_BITS=64" }

			links { 'm' }

			linkoptions { '-pthread' }

			files_project '../vendor/libuv/'
			{
				'include/uv-unix.h',
				'include/uv-linux.h',
				'include/uv-sunos.h',
				'include/uv-darwin.h',
				'include/uv-bsd.h',
				'include/uv-aix.h',
				'src/unix/async.c',
				'src/unix/atomic-ops.h',
				'src/unix/core.c',
				'src/unix/dl.c',
				'src/unix/fs.c',
				'src/unix/getaddrinfo.c',
				'src/unix/getnameinfo.c',
				'src/unix/internal.h',
				'src/unix/loop.c',
				'src/unix/loop-watcher.c',
				'src/unix/pipe.c',
				'src/unix/poll.c',
				'src/unix/process.c',
				'src/unix/signal.c',
				'src/unix/spinlock.h',
				'src/unix/stream.c',
				'src/unix/tcp.c',
				'src/unix/thread.c',
				'src/unix/timer.c',
				'src/unix/tty.c',
				'src/unix/udp.c',

				-- linux
				'src/unix/linux-core.c',
				'src/unix/linux-inotify.c',
				'src/unix/linux-syscalls.c',
				'src/unix/linux-syscalls.h',
			}

		filter "system:windows"
			defines { "_GNU_SOURCE" }

			links { 'advapi32', 'iphlpapi', 'psapi', 'shell32', 'ws2_32' }

			files_project '../vendor/libuv/'
			{
				'src/win/async.c',
				'src/win/atomicops-inl.h',
				'src/win/core.c',
				'src/win/dl.c',
				'src/win/error.c',
				'src/win/fs.c',
				'src/win/fs-event.c',
				'src/win/getaddrinfo.c',
				'src/win/getnameinfo.c',
				'src/win/handle.c',
				'src/win/handle-inl.h',
				'src/win/internal.h',
				'src/win/loop-watcher.c',
				'src/win/pipe.c',
				'src/win/thread.c',
				'src/win/poll.c',
				'src/win/process.c',
				'src/win/process-stdio.c',
				'src/win/req.c',
				'src/win/req-inl.h',
				'src/win/signal.c',
				'src/win/stream.c',
				'src/win/stream-inl.h',
				'src/win/tcp.c',
				'src/win/tty.c',
				'src/win/timer.c',
				'src/win/udp.c',
				'src/win/util.c',
				'src/win/winapi.c',
				'src/win/winapi.h',
				'src/win/winsock.c',
				'src/win/winsock.h',
			}

		
if _OPTIONS['game'] ~= 'server' then
	project "tests_citigame"
		language "C++"
		kind "ConsoleApp"
		
		links { "gmock_main", "gtest_main", "CitiGame", "CitiCore", "Shared" }
		
		includedirs { "client/citigame/include/", "client/citicore/" }
		
		files { "tests/citigame/*.cpp", "tests/test.cpp" }
end

	project "breakpad"
		language "C++"
		kind "StaticLib"

		flags { "StaticRuntime" }

		includedirs { "../vendor/breakpad/src/" }

		configuration "windows"
			files {
				"../vendor/breakpad/src/client/windows/handler/exception_handler.cc",
				"../vendor/breakpad/src/client/windows/crash_generation/client_info.cc",
				"../vendor/breakpad/src/client/windows/crash_generation/crash_generation_client.cc",
				"../vendor/breakpad/src/client/windows/crash_generation/crash_generation_server.cc",
				"../vendor/breakpad/src/client/windows/crash_generation/minidump_generator.cc",
				"../vendor/breakpad/src/common/windows/guid_string.cc",
				"../vendor/breakpad/src/common/windows/http_upload.cc",
				"../vendor/breakpad/src/common/windows/string_utils.cc",
			}

	project "udis86"
		language "C"
		kind "StaticLib"

		files {
			"../vendor/udis86/libudis86/*.c",
			"../vendor/udis86/libudis86/*.h",
		}

	project "cpp-uri"
		language "C++"
		kind "StaticLib"

		includedirs "../vendor/cpp-uri/src/"

		files {
			"../vendor/cpp-uri/src/**.cpp",
		}

		configuration "windows"
			files { os.getenv("BOOST_ROOT") .. "/libs/system/src/error_code.cpp" } -- so there's no need for bjam messiness

		configuration "not windows"
			links { "boost_system" }

	-- StaticRuntime, intended for launcher
	project "tinyxml2"
		language "C++"
		kind "StaticLib"

		flags "StaticRuntime"

		includedirs "../vendor/tinyxml2/"

		files {
			"../vendor/tinyxml2/tinyxml2.cpp"
		}

	group "components"

	-- code for component development
	local components = { }

	dependency = function(name)
		-- find a matching component
		--[[local cname

		for _, c in ipairs(components) do
			if c == name then
				cname = c
				break
			else
				local basename = c:gsub('(.+)-ny', '%1')

				if basename == name then
					cname = c
					break
				end
			end
		end

		if not cname then
			error("Component " .. name .. " seems unknown.")
		end

		includedirs { '../' .. name .. '/include/' }

		links { name }]]

		return
	end

	package.path = '?.lua'

	function string:ends(match)
		return (match == '' or self:sub(-match:len()) == match)
	end

	local json = require('json')

	component = function(name)
		local filename = name .. '/component.json'

		io.input(filename)
		local jsonStr = io.read('*all')
		io.close()

		local decoded = json.decode(jsonStr)

		decoded.rawName = name

		-- check if the project name ends in a known game name, and if we should ignore it
		for _, name in ipairs(gamenames) do
			-- if it ends in the current game name...
			if decoded.name:ends(':' .. name) then
				-- ... and it's not the current game we're targeting...
				if name ~= _OPTIONS['game'] then
					-- ... ignore it
					return
				end
			end
		end

		-- add to the list
		table.insert(components, decoded)
	end

	local do_component = function(name, comp)
		-- do automatic dependencies
		if not comp.dependencies then
			comp.dependencies = {}
		end

		local function id_matches(full, partial)
			local tokenString = ''
			local partialTemp = partial .. ':'

			for token in string.gmatch(full:gsub('\\[.+\\]', ''), '[^:]+') do
				tokenString = tokenString .. token .. ':'

				if partialTemp == tokenString then
					return true
				end
			end

			return false
		end

		local function find_match(id)
			for _, mcomp in ipairs(components) do
				if mcomp.name == id then
					return mcomp
				end

				if id_matches(mcomp.name, id) then
					return mcomp
				end
			end

			return nil
		end

		local hasDeps = {}

		local function process_dependencies(comp)
			local isFulfilled = true

			if comp.dependencies then
				for _, dep in ipairs(comp.dependencies) do
					-- find a match for the dependency
					local match = find_match(dep)

					if match and not hasDeps[match.rawName] then
						print(comp.name .. ' dependency on ' .. dep .. ' fulfilled by ' .. match.rawName)

						if not match.dummy then
							hasDeps[match.rawName] = true
						end

						isFulfilled = isFulfilled and process_dependencies(match)
					elseif not match then
						if not dep:match('%[') then
							print('Dependency unresolved for ' .. dep .. ' in ' .. comp.name)

							return false
						end
					end
				end
			end

			return isFulfilled
		end

		if not process_dependencies(comp) then
			return
		end

		-- process the project

		project(name)

		language "C++"
		kind "SharedLib"

		includedirs { "client/citicore/", 'components/' .. name .. "/include/" }
		files {
			'components/' .. name .. "/src/**.cpp",
			'components/' .. name .. "/src/**.cc",
			'components/' .. name .. "/src/**.h",
			'components/' .. name .. "/include/**.h",
			"client/common/StdInc.cpp",
			"client/common/Error.cpp"
		}

		vpaths { ["z/common/*"] = "client/common/**", ["z/*"] = "components/" .. name .. "/component.rc", ["*"] = "components/" .. name .. "/**" }

		defines { "COMPILING_" .. name:upper():gsub('-', '_'), 'HAS_LOCAL_H' }

		links { "Shared", "CitiCore" }

		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"

		-- add dependency requirements
		for dep, _ in pairs(hasDeps) do
			includedirs { 'components/' .. dep .. '/include/' }

			links { dep }
		end

		configuration {}
		dofile('components/' .. name .. '/component.lua')

		-- loop again in case a previous file has set a configuration constraint
		for dep, _ in pairs(hasDeps) do
			configuration {}
			dofile('components/' .. dep .. '/component.lua')
		end

		configuration "windows"
			buildoptions "/MP"

			files {
				'components/' .. name .. "/component.rc",
			}

		configuration "not windows"
			files {
				'components/' .. name .. "/component.json"
			}

		filter { "system:not windows", "files:**/component.json" }
			buildmessage 'Copying %{file.relpath}'

			buildcommands {
				'{COPY} "%{file.relpath}" "%{cfg.targetdir}/lib' .. name .. '.json"'
			}

			buildoutputs {
				"%{cfg.targetdir}/lib" .. name .. ".json"
			}

		if not _OPTIONS['tests'] then
			return
		end

		-- test project
		local f = io.open('components/' .. name .. '/tests/main.cpp')

		if f then
			io.close(f)
		end

		if not f then
			return
		end

		project('tests_' .. name)

		language "C++"
		kind "ConsoleApp"

		includedirs { 'components/' .. name .. "/include/" }
		files { 'components/' .. name .. "/tests/**.cpp", 'components/' .. name .. "/tests/**.h", "client/common/StdInc.cpp" }

		if not f then
			files { "tests/test.cpp" }
		end

		links { "Shared", "CitiCore", "gmock_main", "gtest_main", name }

		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
	end

	if _OPTIONS['game'] == 'dummy' then
		return
	end

	dofile('components/config.lua')

	table.insert(components, {
		name = 'platform:' .. os.get(),
		rawName = os.get(),
		dummy = true
	})

	for _, comp in ipairs(components) do
		if not comp.dummy then
			do_component(comp.rawName, comp)
		end
	end
