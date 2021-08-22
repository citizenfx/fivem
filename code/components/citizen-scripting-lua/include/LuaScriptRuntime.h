/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

// explicit include guard as we may include this with different names
#ifndef LSRT_H
#define LSRT_H
#undef LuaScriptRuntime
#define LUA_LIB

#include "StdInc.h"

#include <fxScripting.h>

#include <Resource.h>
#include <ManifestVersion.h>

#include <om/OMComponent.h>

#include <lua.hpp>

// Linkage specified in lua.hpp to include/link-against internal structure
// definitions. Note, for ELF builds LUAI_FUNC will mark the function as hidden.
// Lua5.4 is compiled as a C++ library.
#if LUA_VERSION_NUM == 504
#define LUA_INTERNAL_LINKAGE "C++"
#else
#define LUA_INTERNAL_LINKAGE "C"
#endif

// Utility macro for the constexpr if statement
#define LUA_IF_CONSTEXPR if constexpr

// Inline utility
#if !defined(LUA_INLINE)
#ifdef _MSC_VER
#ifndef _DEBUG
#define LUA_INLINE __forceinline
#else
#define LUA_INLINE
#endif
#elif __has_attribute(__always_inline__)
#define LUA_INLINE inline __attribute__((__always_inline__))
#else
#define LUA_INLINE inline
#endif
#endif

/// <summary>
/// Active Lua profiler state; see IScriptProfiler_Tick documentation
/// </summary>
enum class LuaProfilingMode : uint8_t
{
	None,
	Setup,
	Profiling,
	Shutdown,
};

enum class LuaMetaFields : uint8_t
{
	PointerValueInt,
	PointerValueFloat,
	PointerValueVector,
	ReturnResultAnyway,
	ResultAsInteger,
	ResultAsLong,
	ResultAsFloat,
	ResultAsString,
	ResultAsVector,
	ResultAsObject,
	Max
};

/// <summary>
/// </summary>
namespace fx
{
struct PointerFieldEntry
{
	bool empty;
	uintptr_t value;
	PointerFieldEntry()
		: empty(true), value(0)
	{
	}
};

struct PointerField
{
	PointerFieldEntry data[64];
};

#if LUA_VERSION_NUM >= 504 && defined(_WIN32)
#define LUA_USE_RPMALLOC
#endif

class LuaStateHolder
{
private:
	lua_State* m_state;

#if defined(LUA_USE_RPMALLOC)
	/// <summary>
	/// Create a lua_State instance with a rpmalloc allocator.
	/// </summary>
	static lua_State* lua_rpmalloc_state(void*& opaque);

	/// <summary>
	/// Free/Dispose any additional resources associated with the Lua state.
	/// </summary>
	static void lua_rpmalloc_free(void* opaque);

	/// <summary>
	/// Reference to the heap_t pointer. At the time of destruction lua_getallocf
	/// may point to the profiler allocator hook.
	/// </summary>
	void* rpmalloc_data = nullptr;
#endif

public:
	LuaStateHolder()
	{
#if defined(LUA_USE_RPMALLOC)
		m_state = lua_rpmalloc_state(rpmalloc_data);
#else
		m_state = luaL_newstate();
#endif
#if LUA_VERSION_NUM >= 504
		lua_gc(m_state, LUA_GCGEN, 0, 0);  /* GC in generational mode */
#endif
	}

	~LuaStateHolder()
	{
		Close();
	}

	void Close()
	{
		if (m_state)
		{
			lua_close(m_state);

#if defined(LUA_USE_RPMALLOC)
			lua_rpmalloc_free(rpmalloc_data);
			rpmalloc_data = nullptr;
#endif
			m_state = nullptr;
		}
	}

	operator lua_State*()
	{
		return m_state;
	}

	LUA_INLINE lua_State* Get()
	{
		return m_state;
	}
};

class LuaScriptRuntime : public OMClass<LuaScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime, IScriptEventRuntime, IScriptRefRuntime, IScriptMemInfoRuntime, IScriptStackWalkingRuntime, IScriptDebugRuntime, IScriptProfiler>
{
private:
	typedef std::function<void(const char*, const char*, size_t, const char*)> TEventRoutine;

	typedef std::function<void(int32_t, const char*, size_t, char**, size_t*)> TCallRefRoutine;

	typedef std::function<int32_t(int32_t)> TDuplicateRefRoutine;

	typedef std::function<void(int32_t)> TDeleteRefRoutine;

	typedef std::function<void(void*, void*, char**, size_t*)> TStackTraceRoutine;

private:
	LuaStateHolder m_state;

	lua_CFunction m_dbTraceback;

	IScriptHost* m_scriptHost;

	IScriptHostWithResourceData* m_resourceHost;

	IScriptHostWithManifest* m_manifestHost;

	OMPtr<IDebugEventListener> m_debugListener;

	std::function<void(uint64_t, bool)> m_tickRoutine;

	TEventRoutine m_eventRoutine;

	TCallRefRoutine m_callRefRoutine;

	TDuplicateRefRoutine m_duplicateRefRoutine;

	TDeleteRefRoutine m_deleteRefRoutine;

	TStackTraceRoutine m_stackTraceRoutine;

	void* m_parentObject;

	PointerField m_pointerFields[3];

	int m_instanceId;

	std::string m_nativesDir;

	std::unordered_map<std::string, int> m_scriptIds;

	int m_profilingId = 0; // Timeline identifier from fx::ProfilerComponent

	LuaProfilingMode m_profilingMode = LuaProfilingMode::None; // Current fx::ProfilerComponent state.

public:
	LuaScriptRuntime()
	{
		m_instanceId = rand();
	}

	virtual ~LuaScriptRuntime() override;

	static const OMPtr<LuaScriptRuntime>& GetCurrent();
	static IScriptHost* GetLastHost();

	void SetTickRoutine(const std::function<void(uint64_t, bool)>& tickRoutine);

	void SetEventRoutine(const TEventRoutine& eventRoutine);

	LUA_INLINE void SetCallRefRoutine(const TCallRefRoutine& routine)
	{
		if (!m_callRefRoutine)
		{
			m_callRefRoutine = routine;
		}
	}

	LUA_INLINE void SetDuplicateRefRoutine(const TDuplicateRefRoutine& routine)
	{
		if (!m_duplicateRefRoutine)
		{
			m_duplicateRefRoutine = routine;
		}
	}

	LUA_INLINE void SetDeleteRefRoutine(const TDeleteRefRoutine& routine)
	{
		if (!m_deleteRefRoutine)
		{
			m_deleteRefRoutine = routine;
		}
	}

	LUA_INLINE void SetStackTraceRoutine(const TStackTraceRoutine& routine)
	{
		if (!m_stackTraceRoutine)
		{
			m_stackTraceRoutine = routine;
		}
	}

	LUA_INLINE IScriptHost* GetScriptHost()
	{
		return m_scriptHost;
	}

	LUA_INLINE IScriptHostWithResourceData* GetScriptHost2()
	{
		return m_resourceHost;
	}

	LUA_INLINE PointerField* GetPointerFields()
	{
		return m_pointerFields;
	}

	LUA_INLINE const char* GetResourceName()
	{
		char* resourceName = "";
		m_resourceHost->GetResourceName(&resourceName);

		return resourceName;
	}

	LUA_INLINE std::string GetNativesDir()
	{
		return m_nativesDir;
	}

	LUA_INLINE lua_CFunction GetDbTraceback()
	{
		return m_dbTraceback;
	}

	/// <summary>
	/// Manage the fx::ProfilerComponent state while the script runtime is active
	///
	/// Profiler initialization may require additional Lua allocations, e.g.,
	/// placing tables into the registry, and in turn may generate a garbage
	/// collection step/cycle. As finalizers are allowed to use script natives,
	/// e.g., DeleteFunctionReference, it requires an active script runtime.
	/// </summary>
	bool IScriptProfiler_Tick(bool begin);

private:
	result_t LoadFileInternal(OMPtr<fxIStream> stream, char* scriptFile);

	result_t LoadHostFileInternal(char* scriptFile);

	result_t LoadSystemFileInternal(char* scriptFile);

	result_t RunFileInternal(char* scriptFile, std::function<result_t(char*)> loadFunction);

	result_t LoadSystemFile(char* scriptFile);

	result_t LoadNativesBuild(const std::string& nativeBuild);

public:
	NS_DECL_ISCRIPTRUNTIME;

	NS_DECL_ISCRIPTFILEHANDLINGRUNTIME;

	NS_DECL_ISCRIPTTICKRUNTIME;

	NS_DECL_ISCRIPTEVENTRUNTIME;

	NS_DECL_ISCRIPTREFRUNTIME;

	NS_DECL_ISCRIPTMEMINFORUNTIME;

	NS_DECL_ISCRIPTSTACKWALKINGRUNTIME;

	NS_DECL_ISCRIPTDEBUGRUNTIME;

	NS_DECL_ISCRIPTPROFILER;
};
}
#endif
