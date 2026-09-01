#include "StdInc.h"

#include <array>
#include <map>
#include <mutex>

#include "Hooking.h"
#include "Hooking.Stubs.h"

// A CDummyObject holds its registry row index at +0xE0, or -1 when unregistered. Registering one that
// already holds a row overwrites the index, orphaning the earlier row: removal is keyed on +0xE0 and
// can only free the newest one. Leads to pools overflowing and crashing.

static constexpr size_t kRegistrationRowIndexOffset = 0xE0;
static constexpr int32_t kUnregistered = -1;

static void (*g_origRegisterDummyObject)(void*);

// original tail-calls this to write the row's position and radius, so a skipped registration must
// still refresh the row or a dummy that has moved keeps a stale one
static void (*g_refreshRegistrationRow)(void*);

static constexpr size_t kTraceDepth = 6;

using TraceFrames = std::array<uint32_t, kTraceDepth>;

// enough for the handful of paths that reach the registration and a bound in case one of them
// varies its caller
static constexpr size_t kMaxTracedSites = 64;

static std::map<TraceFrames, uint64_t> g_duplicateSites;
static std::mutex g_duplicateSitesMutex;

static std::string FormatTraceFrames(const TraceFrames& key)
{
	std::string chain;

	for (uint32_t frame : key)
	{
		if (!frame)
		{
			break;
		}

		chain += fmt::sprintf("%s%07X", chain.empty() ? "" : " <- ", frame);
	}

	return chain;
}

static void RecordDuplicateRegistration(int32_t existingRow)
{
	void* frames[kTraceDepth] = { nullptr };
	const USHORT captured = RtlCaptureStackBackTrace(1, kTraceDepth, frames, nullptr);

	TraceFrames key = {};

	for (USHORT i = 0; i < captured; i++)
	{
		key[i] = (uint32_t)(hook::get_unadjusted(frames[i]) - 0x140000000);
	}

	std::lock_guard<std::mutex> lock(g_duplicateSitesMutex);

	auto it = g_duplicateSites.find(key);

	if (it != g_duplicateSites.end())
	{
		it->second++;
		return;
	}

	if (g_duplicateSites.size() >= kMaxTracedSites)
	{
		return;
	}

	g_duplicateSites.emplace(key, 1);

	trace("DummyObjectRegistration: new site, row %d, %s\n", existingRow, FormatTraceFrames(key));
}

static void RegisterDummyObject(void* dummy)
{
	const int32_t existingRow = *(int32_t*)((char*)dummy + kRegistrationRowIndexOffset);

	if (existingRow != kUnregistered)
	{
		RecordDuplicateRegistration(existingRow);

		g_refreshRegistrationRow(dummy);
		return;
	}

	g_origRegisterDummyObject(dummy);
}

static HookFunction hookFunction([]()
{
	auto location = hook::get_pattern<char>("40 53 48 83 EC 20 8A 05 ? ? ? ? 45 33 C0 48 8B D9");

	g_refreshRegistrationRow = (decltype(g_refreshRegistrationRow))hook::get_call(location + 0x92);
	g_origRegisterDummyObject = hook::trampoline(location, RegisterDummyObject);
});
