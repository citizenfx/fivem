/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */
#pragma once
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
#define LUA_INTERNAL_LINKAGE "C"

// Utility macro for the constexpr if statement
#define LUA_IF_CONSTEXPR if constexpr

// Inline utility
#if !defined(LUA_INLINE)
#ifdef _MSC_VER
#define LUA_INLINE __forceinline
#elif __has_attribute(__always_inline__)
#define LUA_INLINE inline __attribute__((__always_inline__))
#else
#define LUA_INLINE inline
#endif
#endif

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

class LuaStateHolder
{
private:
	lua_State* m_state;

public:
	LuaStateHolder()
	{
		m_state = luaL_newstate();
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

class LuaScriptRuntime : public OMClass<LuaScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime, IScriptEventRuntime, IScriptRefRuntime, IScriptMemInfoRuntime, IScriptStackWalkingRuntime, IScriptDebugRuntime>
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

	std::function<void()> m_tickRoutine;

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

public:
	LuaScriptRuntime()
	{
		m_instanceId = rand();
	}

	virtual ~LuaScriptRuntime() override;

	static const OMPtr<LuaScriptRuntime>& GetCurrent();
	static IScriptHost* GetLastHost();

	void SetTickRoutine(const std::function<void()>& tickRoutine);

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
};
}
