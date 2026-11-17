#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Patterns.h>
#include <GameInit.h>

#include <AmvZones.h>

#include <algorithm>
#include <unordered_set>

//
// The game collects the streamed ambient mask volume zones by probing "amv_zone_0", "amv_zone_1", ... and ends the
// enumeration at the first name that is not a valid streaming entry - with the shipped data that is amv_zone_183.
// Map creators could therefore only replace the shipped zone files, or fight over the same free numbers.
//
// Here the first name the map does not ship (and each one after it) is handed over to a zone file a resource streams
// under an "amv_zone_" name of its own, which lets every map have its own zones instead of sharing the shipped
// numbering. A zone file that replaces a shipped one needs none of this - it overrides that streaming entry, and the
// probe finds it under its number.
//
// Resource zone files are collected by name when they are registered, which happens before the level is loaded. Once
// the probe runs out of names, each collected name is put through the game's own lookup, so a file that is gone by
// then, or is not in the zone store, is passed over the same way a missing number is.
//
// The enumeration only runs when a session starts: a resource started later gets its zones on the next connect.
//

// names (without extension) of the zone files streamed by resources
static std::vector<std::string> g_amvZoneFiles;

// rage::atHashStringNamespaceSupport::ComputeHash, the hash the probe looks names up by
static uint32_t (*g_computeHash)(uint32_t hashNamespace, const char* string);

// state of the current enumeration
static std::unordered_set<int32_t> g_amvZonesRequested;
static size_t g_amvZoneFileCursor;
static bool g_amvZoneFileOffered;
static uint32_t g_amvZoneLookupHash;
static uint32_t g_amvZonesByName;
static uint32_t g_amvZonesByFile;

// what the probe does next; the stub tells them apart with a single compare, so the order matters
enum AmvZoneProbeAction : uint32_t
{
	// request the zone it found
	AMV_ZONE_REQUEST = 0,

	// look up g_amvZoneLookupHash in place of the next name
	AMV_ZONE_LOOKUP = 1,

	// nothing is left, load the requested zones
	AMV_ZONE_FINISH = 2,
};

namespace streaming
{
	void AddAmvZoneFile(const std::string& baseName)
	{
		// this sees every streamed file, so nothing is copied until the prefix matches
		if (_strnicmp(baseName.c_str(), "amv_zone_", 9) != 0)
		{
			return;
		}

		// the zone and its texture dictionary share a name, and so does a file that is registered again
		if (std::find(g_amvZoneFiles.begin(), g_amvZoneFiles.end(), baseName) == g_amvZoneFiles.end())
		{
			g_amvZoneFiles.push_back(baseName);
		}
	}
}

// Runs for every probe result: `slot` is the number in the probed name, `zoneIndex` the zone store index the lookup
// resolved, or -1.
static AmvZoneProbeAction OnAmvZoneProbe(uint32_t slot, int32_t zoneIndex)
{
	// set when this is the result of a lookup the probe was sent to for a resource file
	const bool fromFile = std::exchange(g_amvZoneFileOffered, false);

	// every enumeration starts with amv_zone_0
	if (slot == 0 && !fromFile)
	{
		g_amvZonesRequested.clear();
		g_amvZoneFileCursor = 0;
		g_amvZonesByName = 0;
		g_amvZonesByFile = 0;
	}

	// a numbered name can resolve to a resource file that already took an earlier slot
	if (zoneIndex != -1 && g_amvZonesRequested.insert(zoneIndex).second)
	{
		if (fromFile)
		{
			++g_amvZonesByFile;
			trace("amv_zone: %s -> probe slot #%u\n", g_amvZoneFiles[g_amvZoneFileCursor - 1].c_str(), slot);
		}
		else
		{
			++g_amvZonesByName;
		}

		return AMV_ZONE_REQUEST;
	}

	// no new zone under this number, the slot goes to the next resource file
	if (g_amvZoneFileCursor < g_amvZoneFiles.size())
	{
		g_amvZoneLookupHash = g_computeHash(0, g_amvZoneFiles[g_amvZoneFileCursor++].c_str());
		g_amvZoneFileOffered = true;

		return AMV_ZONE_LOOKUP;
	}

	// any numbered name past this point would be a resource file, and those have all been offered
	trace("amv_zone: %u zones by name, %u from resource files in free slots\n", g_amvZonesByName, g_amvZonesByFile);

	return AMV_ZONE_FINISH;
}

static HookFunction hookFunction([]()
{
	static struct : jitasm::Frontend
	{
		intptr_t requestZone;
		intptr_t lookupZone;
		intptr_t loadRequested;
		intptr_t lookupHash;
		intptr_t onProbe;

		void Init(uint8_t* request, uint8_t* lookup, uint8_t* finish)
		{
			this->requestZone = reinterpret_cast<intptr_t>(request);
			this->lookupZone = reinterpret_cast<intptr_t>(lookup);
			this->loadRequested = reinterpret_cast<intptr_t>(finish);
			this->lookupHash = reinterpret_cast<intptr_t>(&g_amvZoneLookupHash);
			this->onProbe = reinterpret_cast<intptr_t>(OnAmvZoneProbe);
		}

		void InternalMain() override
		{
			// ebx holds the zone store index the probed name resolved to, or -1; the name counter in r15d is
			// already incremented for the next name, so the probed slot is r15d - 1. Both are non-volatile, as is
			// everything else the loop keeps across its own calls.
			mov(ecx, r15d);
			sub(ecx, 1);
			mov(edx, ebx);
			mov(rax, onProbe);
			sub(rsp, 0x20);
			call(rax);
			add(rsp, 0x20);

			cmp(eax, static_cast<uint32_t>(AMV_ZONE_LOOKUP));
			je("lookup");
			ja("finish");

			// RequestObject for ebx, then the next name
			mov(rax, requestZone);
			jmp(rax);

			L("lookup");

			// the game's lookup takes the hash in eax, validates the slot it finds and comes back here with ebx
			// set; the name counter is left alone, so the file is probed in place of the missing number
			mov(rax, lookupHash);
			mov(eax, dword_ptr[rax]);
			mov(rcx, lookupZone);
			jmp(rcx);

			L("finish");

			mov(rax, loadRequested);
			jmp(rax);
		}
	} probeStub;

	// the loop exit: <cmp ebx, -1> <jnz request> <xor edx, edx>, the point the enumeration gives up on a probe
	auto location = hook::get_pattern<uint8_t>("83 FB ? 0F 85 ? ? ? ? 33 D2");

	// the probe of the next name: <formatf "amv_zone_%d"> <inc counter> <call ComputeHash> <mov rcx, zone store map>;
	// a lookup for a hash of our own enters after the call, where rcx is loaded anew
	auto nextName = hook::get_pattern<uint8_t>("45 8B C7 48 8D 15 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 48 8D 54 24 ? 33 C9 45 03 FD E8 ? ? ? ? 48 8B 0D");

	hook::set_call(&g_computeHash, nextName + 0x1E);

	// the request path is where the original conditional jump went, the load of the requested zones follows it
	probeStub.Init(hook::get_address<uint8_t*>(location + 5), nextName + 0x23, location + 9);

	hook::nop(location, 9);
	hook::jump(location, probeStub.GetCode());

	OnKillNetworkDone.Connect([]()
	{
		g_amvZoneFiles.clear();
	});
});
