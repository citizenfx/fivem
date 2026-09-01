#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Patterns.h>

#include <Error.h>


// Grow the CMetaPedUpdateRequest array past its 128 entries

static constexpr size_t kEntrySize = 0x18;
static constexpr size_t kOrigEntries = 128;
static constexpr uint32_t kOrigCounterDisp = kOrigEntries * kEntrySize;

static constexpr size_t kEntries = 150;
static constexpr size_t kBufferSize = kEntries * kEntrySize;

static_assert(kEntries > kOrigEntries, "the array would shrink below its stock size");

static constexpr int kDrainToRemoveCall = 0x44;
static constexpr int kRemoveDispA = 13;
static constexpr int kRemoveDispB = 23;

struct ArraySite
{
	const char* pattern;
	int dispOffset;
	int leaEnd;
	uint32_t entryOffset;
	const char* what;
};

struct CapacitySite
{
	const char* pattern;
	int immOffset;
	const char* what;
};

static const ArraySite kArraySites[] = {
	{ "8B 15 ? ? ? ? 48 8D 3D ? ? ? ? 41 8B DF 85 D2", 9, 13, 0x00, "drain" },
	{ "8B 15 ? ? ? ? 48 63 C2 4C 8D 05 ? ? ? ? FF C2 89 15", 12, 16, 0x00, "append, new entry" },
	{ "EB ? 4C 8D 05 ? ? ? ? 8B C0 48 8D 14 40 0F B6 44 24", 5, 9, 0x00, "append, existing entry" },
	{ "4B 8D 14 40 48 8D 05 ? ? ? ? 48 39 0C D0", 7, 11, 0x08, "find" },
	{ "B9 80 00 00 00 48 8D 05 ? ? ? ? 33 D2 48 89 50 F8 89 10 66 89 50 04 88 50 06", 8, 12, 0x10, "clear on init" },
};

static const CapacitySite kCapacitySites[] = {
	{ "8B 15 ? ? ? ? B8 80 00 00 00 2B C2 85 C0 7F", 7, "append capacity check" },
	{ "B9 80 00 00 00 48 8D 05 ? ? ? ? 33 D2 48 89 50 F8 89 10 66 89 50 04 88 50 06", 1, "clear on init count" },
};

static constexpr size_t kArraySiteCount = std::size(kArraySites);
static constexpr size_t kCapacitySiteCount = std::size(kCapacitySites);

static HookFunction metaPedUpdateRequestResizeHook([]()
{
	auto resolveUnique = [](const char* pattern) -> uint8_t*
	{
		auto matches = hook::pattern(pattern);

		if (matches.size() != 1)
		{
			FatalError("MetaPedUpdateRequests: pattern '%s' matched %d time(s), expected exactly 1.", pattern, (int)matches.size());
		}

		return matches.get(0).get<uint8_t>(0);
	};

	uint8_t* arrayBases[kArraySiteCount] = {};
	uint8_t* capacityBases[kCapacitySiteCount] = {};

	for (size_t i = 0; i < kArraySiteCount; i++)
	{
		arrayBases[i] = resolveUnique(kArraySites[i].pattern);
	}

	for (size_t i = 0; i < kCapacitySiteCount; i++)
	{
		capacityBases[i] = resolveUnique(kCapacitySites[i].pattern);
	}

	auto targetOf = [&](size_t i) -> uint8_t*
	{
		int32_t disp = *reinterpret_cast<int32_t*>(arrayBases[i] + kArraySites[i].dispOffset);

		return arrayBases[i] + kArraySites[i].leaEnd + disp;
	};

	uint8_t* array = targetOf(0) - kArraySites[0].entryOffset;
	uint8_t* counter = array + kOrigCounterDisp;

	for (size_t i = 0; i < kArraySiteCount; i++)
	{
		uint8_t* expected = array + kArraySites[i].entryOffset;

		if (targetOf(i) != expected)
		{
			FatalError("MetaPedUpdateRequests: site '%s' points at %p, expected %p.", kArraySites[i].what, (void*)targetOf(i), (void*)expected);
		}
	}

	for (size_t i = 0; i < kCapacitySiteCount; i++)
	{
		uint32_t value = *reinterpret_cast<uint32_t*>(capacityBases[i] + kCapacitySites[i].immOffset);

		if (value != kOrigEntries)
		{
			FatalError("MetaPedUpdateRequests: site '%s' holds %u, expected %u.", kCapacitySites[i].what, value, (unsigned)kOrigEntries);
		}
	}

	uint8_t* removeCall = arrayBases[0] + kDrainToRemoveCall;

	if (*removeCall != 0xE8)
	{
		FatalError("MetaPedUpdateRequests: expected a call to the remove helper at %p, found %02X.", (void*)removeCall, *removeCall);
	}

	uint8_t* removeFn = hook::get_call(removeCall);

	for (int off : { kRemoveDispA, kRemoveDispB })
	{
		uint32_t disp = *reinterpret_cast<uint32_t*>(removeFn + off);

		if (disp != kOrigCounterDisp)
		{
			FatalError("MetaPedUpdateRequests: the remove helper at %p holds %08X, expected %08X.", (void*)removeFn, disp, kOrigCounterDisp);
		}
	}


	uint8_t* buffer = (uint8_t*)hook::AllocateStubMemory(kBufferSize);

	if (!buffer)
	{
		FatalError("MetaPedUpdateRequests: could not reserve %llu bytes within reach of the array.", (unsigned long long)kBufferSize);
		return;
	}

	memset(buffer, 0, kBufferSize);

	intptr_t counterDisp = counter - buffer;

	if (counterDisp != int32_t(counterDisp))
	{
		FatalError("MetaPedUpdateRequests: the counter is %lld bytes from the buffer.", (long long)counterDisp);
		return;
	}

	for (size_t i = 0; i < kArraySiteCount; i++)
	{
		intptr_t disp = (buffer + kArraySites[i].entryOffset) - (arrayBases[i] + kArraySites[i].leaEnd);

		if (disp != int32_t(disp))
		{
			FatalError("MetaPedUpdateRequests: the buffer is %lld bytes from site '%s'.", (long long)disp, kArraySites[i].what);
			return;
		}
	}

	for (size_t i = 0; i < kArraySiteCount; i++)
	{
		uint8_t* target = buffer + kArraySites[i].entryOffset;
		uint8_t* ripBase = arrayBases[i] + kArraySites[i].leaEnd;

		hook::put<int32_t>(arrayBases[i] + kArraySites[i].dispOffset, int32_t(target - ripBase));
	}

	for (size_t i = 0; i < kCapacitySiteCount; i++)
	{
		hook::put<uint32_t>(capacityBases[i] + kCapacitySites[i].immOffset, (uint32_t)kEntries);
	}

	hook::put<int32_t>(removeFn + kRemoveDispA, int32_t(counterDisp));
	hook::put<int32_t>(removeFn + kRemoveDispB, int32_t(counterDisp));
});
