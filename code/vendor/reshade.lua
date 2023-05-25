return {
	include = function()
		includedirs { "../vendor/reshade/include/" }
	end,

	run = function()
		language 'C++'
		kind 'StaticLib'

		includedirs { "../vendor/reshade/source/", "../vendor/reshade/res/", "vendor/reshade/" }

		add_dependencies { 'vendor:utfcpp', 'vendor:imgui', 'vendor:minhook' }

		defines { 'RESHADE_FX', 'RESHADE_GUI', 'RESHADE_ADDON' }

		defines { '_GDI32_', 'WINSOCK_API_LINKAGE=' }

		-- deps/Windows.props
		defines {
			'WIN32_LEAN_AND_MEAN',
			'NOGDICAPMASKS',
			'NOMENUS',
			'NOICONS',
			'NOKEYSTATES',
			'NOSYSCOMMANDS',
			'NORASTEROPS',
			'NOATOM',
			'NOCOLOR',
			'NODRAWTEXT',
			'NONLS',
			'NOMEMMGR',
			'NOMETAFILE',
			'NOMINMAX',
			'NOOPENFILE',
			'NOSCROLL',
			'NOSERVICE',
			'NOSOUND',
			'NOTEXTMETRIC',
			'NOWH',
			'NOCOMM',
			'NOKANJI',
			'NOHELP',
			'NOPROFILER',
			'NODEFERWINDOWPOS',
			'NOMCX',
		}

		for _, fn in pairs({'ReShade.vcxproj', 'ReShadeFX.vcxproj'}) do
			local f = io.open('../vendor/reshade/' .. fn)
			local s = f:read('*all')
			f:close()

			for type, name in s:gmatch('Cl([^ ]+) Include="([^"]+)"') do
				files('../vendor/reshade/' .. name)
			end
		end

		files {
			'vendor/reshade/lib_reshade.cpp'
		}

		removefiles {
			'../vendor/reshade/source/vulkan/**',
			'../vendor/reshade/source/opengl/**',
			'../vendor/reshade/source/openvr/**',
			'../vendor/reshade/source/d3d12/**',
			'../vendor/reshade/source/d3d11/d3d11on12*',
			'../vendor/reshade/source/d3d9/**',
			'../vendor/reshade/source/dll_main.cpp',
			'../vendor/reshade/source/effect_codegen_spirv.cpp',
			'../vendor/reshade/source/runtime_gui_vr.cpp',
		}

		filter 'files:../vendor/reshade/examples/09-depth/generic_depth.cpp'
			defines { 'BUILTIN_ADDON' }
			forceincludes 'reshade.hpp'

		filter {}
	end
}
