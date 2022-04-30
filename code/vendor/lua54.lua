local noGLM = ...

return {
	include = function()
		includedirs { "../vendor/lua/" }
		includedirs { "../vendor/lua/libs/glm-binding/" }
		includedirs { "../vendor/lua-cmsgpack/src" }
		includedirs { "../vendor/lua-rapidjson/src" }
		includedirs { "../vendor/lmprof/src" }
		
		-- Dependencies
		add_dependencies 'vendor:msgpack-c'
		includedirs { "../vendor/glm" }
	end,

	run = function()
		targetname "lua54"
		language "C++"
		kind "StaticLib"
		vectorextensions "SSE2" -- @EXPERIMENT
		
		if os.istarget('windows') then
			flags { "LinkTimeOptimization" }

			-- longjmp *should* be exception-safe on Windows non-x86
			defines { "LUA_USE_LONGJMP" }
		elseif os.istarget('linux') then
			defines { "LUA_USE_POSIX" }
		end

		defines {
			'MAKE_LIB', -- Required specification when building onelua.c
			'NOMINMAX',

			--[[ Lua Flags ]]
			'LUA_COMPAT_5_3',
			-- 'LUA_NO_BYTECODE', -- Disables the usage of lua_load with binary chunks

			--[[ Lua Extensions ]]
			'LUA_SANDBOX', -- Disable many features within ldblib.c
			'LUA_C99_MATHLIB', -- Include c99 math functions in lmathlib
			-- disabled (worse yield performance)
			--'LUA_CPP_EXCEPTIONS', -- @EXPERIMENT: unprotected calls are wrapped in typed C++ exceptions
			'GRIT_POWER_COMPOUND', -- Add compound operators
			'GRIT_POWER_INTABLE', -- Support for unpacking named values from tables using the 'in' keyword
			'GRIT_POWER_TABINIT', -- Syntactic sugar to improve the syntax for specifying sets
			'GRIT_POWER_SAFENAV', -- An indexing operation that suppresses errors on accesses into undefined table
			'GRIT_POWER_CCOMMENT', -- Support for C-style block comments
			'GRIT_POWER_DEFER_OLD', -- Import func2close from ltests.h into the base library as _G.defer
			'GRIT_POWER_JOAAT', -- Enable compile time Jenkins' one-at-a-time hashing
			'GRIT_POWER_EACH', -- __iter metamethod support; see documentation
			'GRIT_POWER_WOW', -- Expose lua_createtable and other compatibility functions common in other custom runtimes
			'GRIT_POWER_CHRONO', -- Enable nanosecond resolution timers and x86 rdtsc sampling in loslib.c
			'GRIT_COMPAT_IPAIRS', -- Reintroduce compatibility for the __ipairs metamethod
			'GRIT_POWER_BLOB', -- Enable an API to create non-internalized contiguous byte sequences

			--[[ GLM Flags ]]
			'GLM_ENABLE_EXPERIMENTAL', -- Enable experimental GLM functions
			'GLM_FORCE_INLINE',
			'GLM_FORCE_Z_UP', -- Unit up vector is along the Z-axis; coordinate system: right-handed + z-up

			--[[ GLM Binding Flags ]]
			'LUA_GLM_INCLUDE_ALL', -- Include all GLM modules: glm, gtc, gtx
			'LUA_GLM_ALIASES', -- Include function aliases
			'LUA_GLM_GEOM_EXTENSIONS', -- Geometry API
			'LUA_GLM_RECYCLE', -- Recycle trailing (unused) function parameters
			'LUA_INCLUDE_LIBGLM', -- integrate glm-binding into onelua

			--[[ Serialization Libraries ]]
			'LUA_COMPILED_AS_HPP', -- Serialization libraries to use C++ linkage
			'LUACMSGPACK_COMPAT', -- Require strict MessagePack.lua compatibility
			-- 'LUA_RAPIDJSON_COMPAT', -- Require strict dkjson.lua compatibility (i.e., handle non-JSON compliant edge-cases)
			'LUA_RAPIDJSON_SANITIZE_KEYS',
			'LUA_RAPIDJSON_ALLOCATOR', -- Use lua_getallocf binding for the rapidjson allocator class; otherwise uses default CRT allocator

			--[[ Profiler Library --]]
			'LMPROF_BUILTIN', -- Use internal headers: lobject.h/lstate.h
			'LMPROF_HASH_SPLITMIX', -- Use splitmix record hashing
			'LMPROF_DISABLE_OUTPUT_PATH', -- Disable 'output_path' argument handling.

			--[[
				Experimental Intrinsics
				See: citizen-server-impl/include/state/ServerGameState.h
			--]]
			'GLM_FORCE_DEFAULT_ALIGNED_GENTYPES',
			'GLM_FORCE_SSE2',
			--'GLM_FORCE_SSE3', -- Change vectorextensions
		}

		if noGLM then
			removedefines {
				'LUA_INCLUDE_LIBGLM',
			}
		end

		files {
			"../vendor/lua/onelua.c",
			"../vendor/lua-cmsgpack/src/**",
			"../vendor/lua-rapidjson/src/**",
			"../vendor/lmprof/src/**",
		}

		filter { "files:**.c" }
			compileas "C++"

		filter { "configurations:Release" }
			optimize "Speed"
	end
}
