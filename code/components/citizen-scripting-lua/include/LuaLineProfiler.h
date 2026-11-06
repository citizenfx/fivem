#pragma once

#include <ScriptLineProfiler.h>

#include <lua.hpp>

#include <string_view>

namespace fx
{
class LuaScriptRuntime;
}

namespace fx::lineprof
{
// per-runtime instance data, owned by LuaScriptRuntime
struct LuaRuntimeState
{
	uint32_t generation = 0;
	std::shared_ptr<ResourceProfile> profile;

	// chunk name pointer -> SourceClass, chunk names are interned for the lifetime of their protos
	std::unordered_map<const void*, uint8_t> sourceClasses;

	// callback ref index -> (query sample, send time), for queries still waiting on the database
	std::unordered_map<int32_t, std::pair<SampleKey, uint64_t>> pendingQueries;
};

namespace lua
{
// Lua bytes allocated so far (counted by the runtime's allocator while profiling)
extern uint64_t g_allocBytes;

inline void OnAlloc(void* ptr, size_t osize, size_t nsize)
{
	// for a new block (ptr == nullptr) osize is a type tag, not a size
	size_t oldSize = ptr ? osize : 0;

	if (g_enabled && nsize > oldSize)
	{
		g_allocBytes += nsize - oldSize;
	}
}

void Hook(lua_State* L, lua_Debug* ar);

void OnQueryCallback(LuaRuntimeState& state, int32_t refIdx);

// called for every ref call into the runtime, so keep the common case inline
inline void OnRefCalled(LuaRuntimeState& state, int32_t refIdx)
{
	if (!state.pendingQueries.empty())
	{
		OnQueryCallback(state, refIdx);
	}
}

// Wraps one entry of a Lua runtime (bookmark resume, event, ref call, file load). Time from the last
// line sample to the end of the entry is attributed to the yield point, the last sample, or the entry name.
class EntryScope
{
public:
	inline EntryScope(LuaScriptRuntime* runtime, LuaRuntimeState& state, lua_State* L, std::string_view name, lua_State* resumedThread = nullptr)
	{
		if (g_enabled)
		{
			Begin(runtime, state, L, name, resumedThread);
		}
		else if (lua_gethook(L) == Hook)
		{
			lua_sethook(L, nullptr, 0, 0);
		}
	}

	inline ~EntryScope()
	{
		if (m_index != SIZE_MAX)
		{
			End();
		}
	}

	EntryScope(const EntryScope&) = delete;
	EntryScope& operator=(const EntryScope&) = delete;

private:
	void Begin(LuaScriptRuntime* runtime, LuaRuntimeState& state, lua_State* L, std::string_view name, lua_State* resumedThread);

	void End();

private:
	size_t m_index = SIZE_MAX;
	lua_State* m_resumedThread = nullptr;
};

// Wraps a call out of Lua (native, function reference) made by the C function at stack level 0 of L.
class CallScope
{
public:
	inline explicit CallScope(lua_State* L)
		: m_L(L)
	{
		if (g_enabled)
		{
			Begin();
		}
	}

	inline ~CallScope()
	{
		if (m_index != SIZE_MAX)
		{
			End();
		}
	}

	inline explicit operator bool() const
	{
		return m_index != SIZE_MAX;
	}

	// native invocation; event natives get their event name (Lua argument at eventNameArg) as detail
	void SetNative(uint64_t hash, int eventNameArg);

	// function reference call; refs into MySQL resources are classified as queries
	void SetFunctionReference(const char* ref, const char* args, size_t argsLength);

	void SetJson(bool encode);

	CallScope(const CallScope&) = delete;
	CallScope& operator=(const CallScope&) = delete;

private:
	void Begin();

	void End();

private:
	lua_State* m_L;
	size_t m_index = SIZE_MAX;

	SampleKind m_kind = SampleKind::Native;
	uint64_t m_detailId = 0;
	uint64_t m_nativeHash = 0;
	std::string m_detail;
	std::string m_callbackRef; // MySQL calls: the callback the result will come back through
};
}
}
