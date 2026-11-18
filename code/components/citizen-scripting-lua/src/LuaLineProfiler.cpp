#include "StdInc.h"

#include "LuaScriptRuntime.h"
#include "LuaLineProfiler.h"

#include <msgpack.hpp>

extern LUA_INTERNAL_LINKAGE
{
#include <lstate.h>
}

namespace fx::lineprof::lua
{
enum SourceClass : uint8_t
{
	SourceUser = 1,
	SourceRuntime,
	SourceMySQL,
};

struct Frame
{
	bool isEntry = false;
	uint64_t start = 0;

	// call frames: inclusive time/allocations of script runtimes entered during the call
	uint64_t nestedNs = 0;
	uint64_t nestedAlloc = 0;
	uint64_t startAlloc = 0;
	bool siteValid = false;
	lua_Debug site;

	// entry frames
	LuaRuntimeState* state = nullptr;
	const void* universe = nullptr;
	uint64_t lastTs = 0;
	uint64_t lastAlloc = 0;
	bool haveLast = false;
	SampleKey lastKey{};
	std::string_view name;
};

static thread_local std::vector<Frame> g_frames;

uint64_t g_allocBytes;

static constexpr uint64_t kTriggerEventInternal = 0x91310870;
static constexpr uint64_t kTriggerClientEventInternal = 0x2f7a49e6;
static constexpr uint64_t kTriggerLatentClientEventInternal = 0x70b35890;

static inline int GetInterval()
{
	return std::max(g_sampleInterval, 10);
}

static uint8_t ClassifySource(LuaRuntimeState& state, const char* source)
{
	auto it = state.sourceClasses.find(source);

	if (it != state.sourceClasses.end())
	{
		return it->second;
	}

	std::string_view s{ source };
	uint8_t cls = SourceUser;

	if (s.rfind("@citizen:/", 0) == 0 || s.rfind("@nativesLua:/", 0) == 0)
	{
		cls = SourceRuntime;
	}
	else if (s.size() > 1 && s[0] == '@')
	{
		// '@oxmysql/lib/MySQL.lua' loaded into this resource counts as MySQL client overhead
		auto slash = s.find('/');

		if (slash != std::string_view::npos)
		{
			auto owner = s.substr(1, slash - 1);

			if (owner != state.profile->GetName() && IsMySQLResource(owner))
			{
				cls = SourceMySQL;
			}
		}
	}

	state.sourceClasses.emplace(source, cls);
	return cls;
}

// Finds the line to blame: the first frame from `level` up that belongs to the resource's own code.
// Runtime glue and MySQL library frames are skipped, and change the reported kind.
static bool ResolveSite(lua_State* L, int level, LuaRuntimeState& state, lua_Debug& out, SampleKind& via)
{
	lua_Debug ar;
	lua_Debug fallback;
	bool haveFallback = false;

	via = SampleKind::Lua;

	for (int lvl = level; lvl < level + 24; lvl++)
	{
		if (!lua_getstack(L, lvl, &ar) || !lua_getinfo(L, "Sl", &ar))
		{
			break;
		}

		if (ar.what[0] == 'C')
		{
			continue;
		}

		auto cls = ClassifySource(state, ar.source);

		if (cls == SourceUser)
		{
			out = ar;
			return true;
		}

		if (cls == SourceMySQL)
		{
			via = SampleKind::MySQL;
		}
		else if (via == SampleKind::Lua)
		{
			via = SampleKind::Runtime;
		}

		if (!haveFallback)
		{
			fallback = ar;
			haveFallback = true;
		}
	}

	if (haveFallback)
	{
		out = fallback;
		return true;
	}

	return false;
}

static void FillInfo(lua_State* L, const lua_Debug& site, SampleInfo& info)
{
	const char* source = site.source;
	info.source = (source[0] == '@' || source[0] == '=') ? source + 1 : site.short_src;
	info.lineDefined = site.linedefined;
	info.lastLineDefined = site.lastlinedefined;
	info.line = site.currentline;

	lua_Debug named = site;

	if (lua_getinfo(L, "n", &named) && named.name)
	{
		info.function = named.name;
	}
	else
	{
		info.function = (site.linedefined == 0) ? "main chunk" : "anonymous";
	}
}

static void RecordSample(lua_State* L, Frame& entry, const lua_Debug& site, SampleKind via, uint64_t now)
{
	uint64_t elapsed = now - entry.lastTs;
	uint64_t alloc = g_allocBytes - entry.lastAlloc;
	SampleKey key{ entry.state->generation, site.source, site.linedefined, site.currentline, via, 0 };

	entry.state->profile->Record(
	key, [&](SampleInfo& info)
	{
		FillInfo(L, site, info);
		info.kind = via;
	},
	elapsed, elapsed, false, alloc);

	entry.lastKey = key;
	entry.haveLast = true;
	entry.lastTs = now;
	entry.lastAlloc = g_allocBytes;
}

// time we can't place on a live stack goes to the previous sample, or to the entry itself
static void FlushTail(Frame& entry, uint64_t now)
{
	uint64_t elapsed = now - entry.lastTs;
	uint64_t alloc = g_allocBytes - entry.lastAlloc;
	entry.lastTs = now;
	entry.lastAlloc = g_allocBytes;

	if (elapsed == 0 && alloc == 0)
	{
		return;
	}

	if (entry.haveLast && entry.state->profile->AddToExisting(entry.lastKey, elapsed, alloc))
	{
		return;
	}

	auto name = entry.name;
	SampleKey key{ entry.state->generation, nullptr, 0, 0, SampleKind::Runtime, std::hash<std::string_view>()(name) };

	entry.state->profile->Record(
	key, [name](SampleInfo& info)
	{
		info.source = "(entry)";
		info.function = std::string{ name };
		info.kind = SampleKind::Runtime;
	},
	elapsed, elapsed, false, alloc);
}

void Hook(lua_State* L, lua_Debug*)
{
	if (!g_enabled)
	{
		lua_sethook(L, nullptr, 0, 0);
		return;
	}

	if (g_frames.empty())
	{
		return;
	}

	auto& entry = g_frames.back();

	if (!entry.isEntry || entry.universe != G(L))
	{
		return;
	}

	if (lua_gethookcount(L) != GetInterval())
	{
		lua_sethook(L, Hook, LUA_MASKCOUNT, GetInterval());
	}

	uint64_t now = Now();

	lua_Debug site;
	SampleKind via;

	if (ResolveSite(L, 0, *entry.state, site, via))
	{
		RecordSample(L, entry, site, via, now);
	}
	else
	{
		FlushTail(entry, now);
	}
}

void EntryScope::Begin(LuaScriptRuntime* runtime, LuaRuntimeState& state, lua_State* L, std::string_view name, lua_State* resumedThread)
{
	if (!state.profile)
	{
		state.generation = NewGeneration();
		state.profile = GetResourceProfile(runtime->GetResourceName());
	}

	// don't replace a hook someone else owns (lmprof, debug.sethook)
	auto hook = lua_gethook(L);

	if (!hook || (hook == Hook && lua_gethookcount(L) != GetInterval()))
	{
		lua_sethook(L, Hook, LUA_MASKCOUNT, GetInterval());
	}

	uint64_t now = Now();

	if (!g_frames.empty() && g_frames.back().isEntry)
	{
		FlushTail(g_frames.back(), now);
	}

	Frame frame;
	frame.isEntry = true;
	frame.start = now;
	frame.lastTs = now;
	frame.startAlloc = g_allocBytes;
	frame.lastAlloc = g_allocBytes;
	frame.state = &state;
	frame.universe = G(L);
	frame.name = name;

	m_index = g_frames.size();
	m_resumedThread = resumedThread;

	g_frames.push_back(frame);
}

void EntryScope::End()
{
	if (m_index >= g_frames.size())
	{
		return;
	}

	g_frames.resize(m_index + 1);

	uint64_t now = Now();
	auto& entry = g_frames.back();

	bool placed = false;

	// a yielded thread still has its stack, so the tail belongs to the line that called Wait()
	if (m_resumedThread && lua_status(m_resumedThread) == LUA_YIELD)
	{
		lua_Debug site;
		SampleKind via;

		if (ResolveSite(m_resumedThread, 0, *entry.state, site, via))
		{
			RecordSample(m_resumedThread, entry, site, via, now);
			placed = true;
		}
	}

	if (!placed)
	{
		FlushTail(entry, now);
	}

	uint64_t total = now - entry.start;
	uint64_t totalAlloc = g_allocBytes - entry.startAlloc;
	g_frames.pop_back();

	if (!g_frames.empty())
	{
		auto& parent = g_frames.back();

		if (parent.isEntry)
		{
			parent.lastTs = now;
			parent.lastAlloc = g_allocBytes;
		}
		else
		{
			parent.nestedNs += total;
			parent.nestedAlloc += totalAlloc;
		}
	}
}

void CallScope::Begin()
{
	if (g_frames.empty())
	{
		return;
	}

	auto& entry = g_frames.back();

	if (!entry.isEntry || entry.universe != G(m_L))
	{
		return;
	}

	Frame call;

	// level 0 is the C function making the call
	SampleKind via;

	if (ResolveSite(m_L, 1, *entry.state, call.site, via))
	{
		RecordSample(m_L, entry, call.site, via, Now());
		call.siteValid = true;
	}
	else
	{
		FlushTail(entry, Now());
	}

	call.start = Now();
	call.startAlloc = g_allocBytes;

	m_index = g_frames.size();
	g_frames.push_back(call);
}

void CallScope::End()
{
	if (m_index >= g_frames.size())
	{
		return;
	}

	g_frames.resize(m_index + 1);

	uint64_t now = Now();
	Frame call = g_frames.back();
	g_frames.pop_back();

	if (g_frames.empty() || !g_frames.back().isEntry)
	{
		return;
	}

	auto& entry = g_frames.back();

	uint64_t total = now - call.start;
	uint64_t self = total - std::min(total, call.nestedNs);

	uint64_t totalAlloc = g_allocBytes - call.startAlloc;
	uint64_t alloc = totalAlloc - std::min(totalAlloc, call.nestedAlloc);

	SampleKey key{
		entry.state->generation,
		call.siteValid ? call.site.source : nullptr,
		call.siteValid ? call.site.linedefined : 0,
		call.siteValid ? call.site.currentline : 0,
		m_kind,
		m_detailId
	};

	entry.state->profile->Record(
	key, [&](SampleInfo& info)
	{
		if (call.siteValid)
		{
			FillInfo(m_L, call.site, info);
		}
		else
		{
			info.source = "(unknown)";
		}

		info.kind = m_kind;

		if (m_kind == SampleKind::Native)
		{
			lua_Debug ar;

			if (lua_getstack(m_L, 0, &ar) && lua_getinfo(m_L, "n", &ar) && ar.name && strcmp(ar.name, "InvokeNative") != 0)
			{
				info.detail = ar.name;
			}
			else
			{
				info.detail = fmt::sprintf("0x%016llx", m_nativeHash);
			}
		}
		else
		{
			info.detail = m_detail;
		}
	},
	self, total, true, alloc);

	// remember our own callback, so the round trip can be measured when the result arrives
	if (!m_callbackRef.empty())
	{
		std::string_view ref{ m_callbackRef };
		const auto& resourceName = entry.state->profile->GetName();

		if (ref.size() > resourceName.size() && ref.substr(0, resourceName.size()) == resourceName && ref[resourceName.size()] == ':')
		{
			auto& pending = entry.state->pendingQueries;

			// callbacks that never come back (errors, stopped resources) shouldn't pile up
			if (pending.size() > 4096)
			{
				pending.clear();
			}

			pending[atoi(ref.data() + ref.rfind(':') + 1)] = { key, call.start };
		}
	}

	entry.lastTs = Now();
	entry.lastAlloc = g_allocBytes;
}

void OnQueryCallback(LuaRuntimeState& state, int32_t refIdx)
{
	auto it = state.pendingQueries.find(refIdx);

	if (it == state.pendingQueries.end())
	{
		return;
	}

	auto [key, sentAt] = it->second;
	state.pendingQueries.erase(it);

	if (state.profile)
	{
		state.profile->AddWait(key, Now() - sentAt);
	}
}

void CallScope::SetNative(uint64_t hash, int eventNameArg)
{
	m_nativeHash = hash;
	m_kind = SampleKind::Native;
	m_detailId = hash;

	if (hash == kTriggerEventInternal || hash == kTriggerClientEventInternal || hash == kTriggerLatentClientEventInternal)
	{
		if (lua_type(m_L, eventNameArg) == LUA_TSTRING)
		{
			std::string_view eventName = lua_tostring(m_L, eventNameArg);

			m_kind = SampleKind::Event;
			m_detailId = hash ^ std::hash<std::string_view>()(eventName);
			m_detail = fmt::sprintf("%s: %s", (hash == kTriggerEventInternal) ? "TriggerEvent" : (hash == kTriggerClientEventInternal) ? "TriggerClientEvent" : "TriggerLatentClientEvent", std::string{ eventName });
		}
	}
}

void CallScope::SetJson(bool encode)
{
	m_kind = SampleKind::Json;
	m_detail = encode ? "json.encode" : "json.decode";
	m_detailId = encode ? 1 : 2;
}

// exports go through scheduler.lua's wrapper closure, which holds the export name as upvalue 'k2'
static std::string FindExportName(lua_State* L)
{
	lua_Debug ar;

	for (int lvl = 1; lvl < 12 && lua_getstack(L, lvl, &ar); lvl++)
	{
		if (!lua_getinfo(L, "Sf", &ar))
		{
			break;
		}

		if (ar.what[0] == 'L' && strcmp(ar.source, "@citizen:/scripting/lua/scheduler.lua") == 0)
		{
			for (int i = 1;; i++)
			{
				const char* name = lua_getupvalue(L, -1, i);

				if (!name)
				{
					break;
				}

				if (strcmp(name, "k2") == 0 && lua_type(L, -1) == LUA_TSTRING)
				{
					std::string exportName = lua_tostring(L, -1);
					lua_pop(L, 2);

					return exportName;
				}

				lua_pop(L, 1);
			}
		}

		lua_pop(L, 1);
	}

	return {};
}

static std::string NormalizeQuery(std::string_view query)
{
	std::string out;
	out.reserve(std::min<size_t>(query.size(), 300));

	bool space = false;

	for (char c : query)
	{
		if (isspace(static_cast<unsigned char>(c)))
		{
			space = !out.empty();
			continue;
		}

		if (space)
		{
			out += ' ';
			space = false;
		}

		out += c;

		if (out.size() >= 300)
		{
			out += "...";
			break;
		}
	}

	return out;
}

static std::string DescribeTransaction(const msgpack::object& queries)
{
	const auto& first = queries.via.array.ptr[0];

	if (first.type == msgpack::type::STR)
	{
		return fmt::sprintf("[transaction x%d] %s", queries.via.array.size, NormalizeQuery({ first.via.str.ptr, first.via.str.size }));
	}

	if (first.type == msgpack::type::MAP)
	{
		for (uint32_t j = 0; j < first.via.map.size; j++)
		{
			const auto& kv = first.via.map.ptr[j];

			if (kv.val.type == msgpack::type::STR && (kv.key.type != msgpack::type::STR || std::string_view{ kv.key.via.str.ptr, kv.key.via.str.size } == "query"))
			{
				return fmt::sprintf("[transaction x%d] %s", queries.via.array.size, NormalizeQuery({ kv.val.via.str.ptr, kv.val.via.str.size }));
			}
		}
	}

	return fmt::sprintf("[transaction x%d]", queries.via.array.size);
}

// args are a msgpack array: the query is the first string (or a transaction list), and the
// result callback is the first function reference (msgpack ext 10/11 holding the ref string)
static void ExtractQuery(const char* args, size_t argsLength, std::string& query, std::string& callbackRef)
{
	try
	{
		auto unpacked = msgpack::unpack(args, argsLength);
		const auto& root = unpacked.get();

		if (root.type != msgpack::type::ARRAY)
		{
			return;
		}

		for (uint32_t i = 0; i < root.via.array.size; i++)
		{
			const auto& arg = root.via.array.ptr[i];

			if (query.empty() && arg.type == msgpack::type::STR)
			{
				query = NormalizeQuery({ arg.via.str.ptr, arg.via.str.size });
			}
			else if (query.empty() && arg.type == msgpack::type::ARRAY && arg.via.array.size > 0)
			{
				query = DescribeTransaction(arg);
			}
			else if (callbackRef.empty() && arg.type == msgpack::type::EXT && (arg.via.ext.type() == 10 || arg.via.ext.type() == 11))
			{
				callbackRef.assign(arg.via.ext.data(), arg.via.ext.size);
			}
		}
	}
	catch (std::exception&)
	{
	}
}

void CallScope::SetFunctionReference(const char* ref, const char* args, size_t argsLength)
{
	std::string_view refView{ ref };
	auto target = refView.substr(0, refView.find(':'));
	auto exportName = FindExportName(m_L);

	if (IsMySQLResource(target))
	{
		std::string query;
		ExtractQuery(args, argsLength, query, m_callbackRef);

		m_kind = SampleKind::MySQL;
		m_detail = fmt::sprintf("%s%s%s", exportName, exportName.empty() ? "" : ": ", query.empty() ? "(no query text)" : query);
	}
	else
	{
		m_kind = SampleKind::Export;
		m_detail = exportName.empty() ? fmt::sprintf("%s (function reference)", std::string{ target }) : fmt::sprintf("exports.%s:%s", std::string{ target }, exportName);
	}

	m_detailId = std::hash<std::string>()(m_detail);
}
}
