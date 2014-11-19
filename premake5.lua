solution "CitizenMP"
	configurations { "Debug NY", "Release NY" }
	
	flags { "No64BitChecks", "Symbols", "Unicode" }
	
	flags { "NoIncrementalLink", "NoEditAndContinue" } -- this breaks our custom section ordering in citilaunch, and is kind of annoying otherwise
	
	includedirs { "shared/", "client/shared/", "../vendor/jitasm/", "deplibs/include/", "../vendor/gmock/include/", "../vendor/gtest/include", os.getenv("BOOST_ROOT") }
	
	defines { "GTEST_HAS_PTHREAD=0" }

	libdirs { "deplibs/lib/" }

	links { "winmm" }
	
	configuration "Debug*"
		targetdir "bin/debug"
		defines "NDEBUG"

		defines { '_ITERATOR_DEBUG_LEVEL=0' }
		
	configuration "Release*"
		targetdir "bin/release"
		defines "NDEBUG"
		optimize "Speed"
		
	configuration "* NY"
		defines "GTA_NY"
			
	project "CitiLaunch"
		targetname "CitizenFX"
		language "C++"
		kind "WindowedApp"
		
		defines "COMPILING_LAUNCH"
		
		links { "SharedLibc", "dbghelp", "psapi", "libcurl", "tinyxml", "liblzma", "comctl32" }
		
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
		
		includedirs { "client/libcef/" }

		flags { "StaticRuntime" }
		
		configuration "Debug*"
			links { "libcefd" }
			
		configuration "Release*"
			links { "libcef" }
		
		configuration "windows"
			linkoptions "/ENTRY:main /IGNORE:4254 /DYNAMICBASE:NO /SAFESEH:NO /LARGEADDRESSAWARE" -- 4254 is the section type warning we tend to get
		
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

	project "CitiGame"
		targetname "CitizenGame"
		language "C++"
		kind "SharedLib"
		
		files
		{
			"client/citigame/**.cpp", "client/citigame/**.h", "client/common/Error.cpp", "client/citigame/**.c", "client/common/StdInc.cpp"
		}
		
		links { "Shared", "citicore", "yaml-cpp", "msgpack-c", "rage-nutsnbolts-ny", "nui-core", "lua51", "winmm", "winhttp", "ws2_32", "libcef_dll", "libcef", "delayimp", "libnp", "http-client", "net", "resources", "downloadmgr" }
		
		defines "COMPILING_GAME"
		
		libdirs { "../vendor/luajit/src/", "client/libcef/lib/", "client/shared/np" }
		includedirs { "client/citigame/include/", "components/nui-core/include/", "components/rage-nutsnbolts-ny/include/", "components/downloadmgr/include/", "components/net/include/", "client/citicore/", "components/resources/include/", "components/http-client/include/", "../vendor/luajit/src/", "../vendor/yaml-cpp/include/", "../vendor/msgpack-c/include/", "deplibs/include/msgpack-c/", "client/libcef/", "client/shared/np" }
		
		linkoptions "/DELAYLOAD:libcef.dll"
		
		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
		
		configuration "* NY"
			includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "client/citigame/net/" }
			links { "HooksNY", "GameNY" }
			
		configuration "Debug*"
			links { "libcefd" }
			
		configuration "Release*"
			links { "libcef" }

	--[[local buildHost = os.getenv("COMPUTERNAME") or 'dummy'

	if buildHost == 'FALLARBOR' then
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

	external 'CitiMono'
		uuid 'E781BFF9-D34E-1A05-FC67-08ADE8934F93'
		kind 'SharedLib'
		language 'C#'

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
			
	project "GameNY"
		targetname "game_ny"
		language "C++"
		kind "SharedLib"
		
		links { "Shared", "zlib" }
		
		defines "COMPILING_GAMESPEC"
		
		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
		
		configuration "* NY"
			includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "../vendor/zlib/" }
		
			files
			{
				"client/game_ny/**.cpp", "client/game_ny/**.h", "client/common/Error.cpp", "client/common/StdInc.cpp"
			}
			
	project "HooksNY"
		targetname "hooks_ny"
		language "C++"
		kind "SharedLib"
		
		links { "Shared", "GameNY", "ws2_32", "rage-graphics-ny", "gta-core-ny" }
		
		defines "COMPILING_HOOKS"	

		pchsource "client/hooks_ny/base/StdInc.cpp"
		pchheader "StdInc.h"		
		
		configuration "* NY"
			includedirs { "components/gta-core-ny/include", "components/rage-graphics-ny/include", "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "client/hooks_ny/base/" }
			
			files
			{
				"client/hooks_ny/**.cpp", "client/hooks_ny/**.h", "client/common/Error.cpp"
			}
		
	project "Shared"
		targetname "shared"
		language "C++"
		kind "StaticLib"

		defines "COMPILING_SHARED"
		
		includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/" }
		
		files
		{
			"shared/**.cpp", "shared/**.h", "client/shared/**.cpp", "client/shared/**.h"
		}

	project "SharedLibc"
		targetname "shared_libc"
		language "C++"
		kind "StaticLib"

		flags { "StaticRuntime" }

		defines "COMPILING_SHARED"
		
		includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/" }
		
		files
		{
			"shared/**.cpp", "shared/**.h", "client/shared/**.cpp", "client/shared/**.h"
		}
		
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

		buildoptions "/MP /wd4244 /wd4267 /wd4018 /wd4355 /wd4800 /wd4251 /wd4996 /wd4146 /wd4305"

		files
		{
			"../vendor/protobuf/src/google/protobuf/io/coded_stream.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/common.cc",
			"../vendor/protobuf/src/google/protobuf/extension_set.cc",
			"../vendor/protobuf/src/google/protobuf/generated_message_util.cc",
			"../vendor/protobuf/src/google/protobuf/message_lite.cc",
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
		
	project "tests_citigame"
		language "C++"
		kind "ConsoleApp"
		
		links { "gmock_main", "gtest_main", "CitiGame", "CitiCore", "Shared" }
		
		includedirs { "client/citigame/include/", "client/citicore/" }
		
		files { "tests/citigame/*.cpp", "tests/test.cpp" }

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

	local json = require('json')

	component = function(name)
		local filename = name .. '/component.json'

		io.input(filename)
		local jsonStr = io.read('*all')
		io.close()

		local decoded = json.decode(jsonStr)

		decoded.rawName = name

		table.insert(components, decoded)
	end

	local do_component = function(name, comp)
		project(name)

		language "C++"
		kind "SharedLib"

		buildoptions "/MP"

		includedirs { "client/citicore/", 'components/' .. name .. "/include/" }
		files {
			'components/' .. name .. "/src/**.cpp",
			'components/' .. name .. "/src/**.cc",
			'components/' .. name .. "/src/**.h",
			'components/' .. name .. "/include/**.h",
			'components/' .. name .. "/component.rc",
			"client/common/StdInc.cpp",
			"client/common/Error.cpp"
		}

		defines { "COMPILING_" .. name:upper():gsub('-', '_'), 'HAS_LOCAL_H' }

		links { "Shared", "CitiCore" }

		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"

		dofile('components/' .. name .. '/component.lua')

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
			for _, dep in ipairs(comp.dependencies) do
				-- find a match for the dependency
				local match = find_match(dep)

				if match and not hasDeps[match.rawName] then
					print(project().name .. ' dependency on ' .. dep .. ' fulfilled by ' .. match.rawName)

					includedirs { 'components/' .. match.rawName .. '/include/' }

					links { match.rawName }

					dofile('components/' .. match.rawName .. '/component.lua')

					hasDeps[match.rawName] = true

					process_dependencies(match)
				end
			end
		end

		process_dependencies(comp)

		-- test project
		project('tests_' .. name)

		language "C++"
		kind "ConsoleApp"

		includedirs { 'components/' .. name .. "/include/" }
		files { 'components/' .. name .. "/tests/**.cpp", 'components/' .. name .. "/tests/**.h", "client/common/StdInc.cpp" }

		local f = io.open('components/' .. name .. '/tests/main.cpp')

		if f then
			io.close(f)
		end

		if not f then
			files { "tests/test.cpp" }
		end

		links { "Shared", "CitiCore", "gmock_main", "gtest_main", name }

		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
	end

	dofile('components/config.lua')

	for _, comp in ipairs(components) do
		do_component(comp.rawName, comp)
	end