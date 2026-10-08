/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Error.h>
#include <Hooking.h>
#include <MinHook.h>
#include <VFSManager.h>
#include <gameSkeleton.h>
#include <nutsnbolts.h>

#include <atomic>
#include <mutex>
#include <unordered_map>

namespace
{
struct WavePack
{
	std::string path;
	std::string name;
	size_t references = 1;
};

struct WavePackChange
{
	std::string path;
	std::string name;
	bool added;
};

// partial b1491 audWaveSlot layout; only fields used for live reload are mapped
struct AudioBankState
{
	uint8_t pad10[0x10];
	uint32_t pendingArgument;
	uint32_t activeArgument;
	uint8_t pad18[4];
	uint32_t requestTimestamp;
	uint8_t padA4[0x84];
	uint32_t pendingName;
	uint32_t activeName;
	uint8_t padB2[6];
	uint8_t loaderType;
	uint8_t pendingPriority;
	uint8_t activePriority;
	uint8_t state;
};

static_assert(offsetof(AudioBankState, pendingName) == 0xA4);
static_assert(offsetof(AudioBankState, state) == 0xB5);

std::mutex g_wavePackMutex;
std::vector<WavePack> g_wavePacks;
std::vector<WavePackChange> g_wavePackChanges;
std::unordered_map<std::string, std::string> g_resourceBankSources;
std::atomic<bool> g_audioInitialized{ false };

bool (*g_origMountWavePack)(void* self, const char* entry);
void (*g_origUnmountWavePack)(void* self, const char* entry);
void (*g_origBuildWavePath)(char* out, uint32_t size, const char* name);

uint32_t* g_audioBankCount;
uint8_t** g_audioBankPool;
void* g_audioRequestLock;
void (*g_lockAudioRequests)(void* lock);
void (*g_unlockAudioRequests)(void* lock);
uint32_t (*g_getAudioRequestTimestamp)();
void* g_audioNameRegistry;
const char* (*g_getAudioName)(void* registry, uint32_t name);

std::string NormalizeWavePath(const char* value)
{
	std::string path = value ? value : "";
	std::replace(path.begin(), path.end(), '\\', '/');
	while (!path.empty() && path.back() == '/')
	{
		path.pop_back();
	}
	return path;
}

std::string NormalizeWaveKey(const std::string& value)
{
	auto key = NormalizeWavePath(value.c_str());
	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char character)
	{
		return static_cast<char>(tolower(character));
	});
	return key;
}

bool IsResourceWavePack(const std::string& path)
{
	return path.size() >= 11 && _strnicmp(path.c_str(), "resources:/", 11) == 0 &&
		_stricmp(path.c_str() + path.size() - 4, ".rpf") != 0;
}

std::string FindResourceWave(const std::string& path, const std::string& packName, const std::string& waveName)
{
	if (waveName.size() <= packName.size() || waveName[packName.size()] != '/' ||
		_strnicmp(waveName.c_str(), packName.c_str(), packName.size()) != 0)
	{
		return {};
	}

	auto result = path + waveName.substr(packName.size()) + ".awc";
	auto device = vfs::GetDevice(result);
	if (!device.GetRef())
	{
		return {};
	}

	auto attributes = device->GetAttributes(result);
	return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY)) ? result : std::string();
}

bool MountWavePack(void* self, const char* entry)
{
	auto path = NormalizeWavePath(entry);
	if (!IsResourceWavePack(path))
	{
		return g_origMountWavePack(self, entry);
	}

	std::lock_guard<std::mutex> lock(g_wavePackMutex);
	for (auto& pack : g_wavePacks)
	{
		if (_stricmp(pack.path.c_str(), path.c_str()) == 0)
		{
			++pack.references;
			return true;
		}
	}

	auto name = path.substr(path.find_last_of('/') + 1);
	g_wavePacks.push_back({ path, name });
	if (g_audioInitialized.load(std::memory_order_acquire))
	{
		g_wavePackChanges.push_back({ path, name, true });
	}
	return true;
}

void UnmountWavePack(void* self, const char* entry)
{
	auto path = NormalizeWavePath(entry);
	if (!IsResourceWavePack(path))
	{
		g_origUnmountWavePack(self, entry);
		return;
	}

	std::lock_guard<std::mutex> lock(g_wavePackMutex);
	for (auto it = g_wavePacks.begin(); it != g_wavePacks.end(); ++it)
	{
		if (_stricmp(it->path.c_str(), path.c_str()) != 0)
		{
			continue;
		}

		if (--it->references == 0)
		{
			if (g_audioInitialized.load(std::memory_order_acquire))
			{
				g_wavePackChanges.push_back({ it->path, it->name, false });
			}
			g_wavePacks.erase(it);
		}
		break;
	}
}

void BuildWavePath(char* out, uint32_t size, const char* name)
{
	g_origBuildWavePath(out, size, name);

	// keep the game's path as fallback, then check resource packs newest first
	std::vector<WavePack> packs;
	{
		std::lock_guard<std::mutex> lock(g_wavePackMutex);
		packs = g_wavePacks;
	}

	auto waveName = NormalizeWavePath(name);
	for (auto it = packs.rbegin(); it != packs.rend(); ++it)
	{
		auto path = FindResourceWave(it->path, it->name, waveName);
		if (path.empty() || path.size() >= size)
		{
			continue;
		}

		memcpy(out, path.c_str(), path.size() + 1);
		// remember the winning pack so removing it can reload this bank
		std::lock_guard<std::mutex> lock(g_wavePackMutex);
		g_resourceBankSources[NormalizeWaveKey(waveName)] = it->path;
		return;
	}
}

// couldn't find the original names for these flags,
// had to learn their purpose the hard way from audWaveSlot::RequestLoad and UpdateSlots
constexpr uint8_t kBankRequestPending = 1 << 0;
constexpr uint8_t kBankLoadInProgress = 1 << 1;
constexpr uint8_t kBankRequestChanged = 1 << 3;

void ReloadChangedWaveBanks()
{
	if (!g_audioInitialized.load(std::memory_order_acquire) || !*g_audioBankPool)
	{
		return;
	}

	std::vector<WavePackChange> changes;
	std::unordered_map<std::string, std::string> loadedSources;
	{
		std::lock_guard<std::mutex> lock(g_wavePackMutex);
		changes.swap(g_wavePackChanges);
		loadedSources = g_resourceBankSources;
	}

	if (changes.empty())
	{
		return;
	}

	struct LoadedBank
	{
		AudioBankState* state;
		uint32_t name;
		uint8_t flags;
	};

	// copy slot state under the audio lock; VFS lookups below should not hold it
	std::vector<LoadedBank> loadedBanks;
	g_lockAudioRequests(g_audioRequestLock);
	auto bankCount = *g_audioBankCount;
	auto bankPool = *g_audioBankPool;
	loadedBanks.reserve(bankCount);
	// audWaveSlot pool entries are 0x100 bytes apart
	for (uint32_t i = 0; i < bankCount; ++i)
	{
		auto bank = reinterpret_cast<AudioBankState*>(bankPool + (i * 0x100));
		if (bank->activeName != UINT32_MAX)
		{
			loadedBanks.push_back({ bank, bank->activeName, bank->state });
		}
	}
	g_unlockAudioRequests(g_audioRequestLock);

	struct ReloadBank
	{
		AudioBankState* state;
		uint32_t name;
		std::string text;
		std::string key;
	};

	bool ready = true;
	std::vector<ReloadBank> reloadBanks;
	for (const auto& loaded : loadedBanks)
	{
		auto name = g_getAudioName(g_audioNameRegistry, loaded.name);
		if (!name)
		{
			continue;
		}

		auto waveName = NormalizeWavePath(name);
		auto key = NormalizeWaveKey(waveName);
		auto source = loadedSources.find(key);
		bool affected = false;
		for (const auto& change : changes)
		{
			if ((change.added && !FindResourceWave(change.path, change.name, waveName).empty()) ||
				(!change.added && source != loadedSources.end() &&
				_stricmp(source->second.c_str(), change.path.c_str()) == 0))
			{
				affected = true;
				break;
			}
		}

		if (!affected)
		{
			continue;
		}
		// retry next frame if an affected bank already has a pending request or active load
		if (loaded.flags & (kBankRequestPending | kBankLoadInProgress))
		{
			ready = false;
			break;
		}
		reloadBanks.push_back({ loaded.state, loaded.name, std::move(waveName), std::move(key) });
	}

	if (ready)
	{
		// the audio thread may have changed these slots while paths were checked
		g_lockAudioRequests(g_audioRequestLock);
		for (const auto& reload : reloadBanks)
		{
			if (reload.state->activeName != reload.name ||
				(reload.state->state & (kBankRequestPending | kBankLoadInProgress)))
			{
				ready = false;
				break;
			}
		}

		if (ready)
		{
			// clear only the source we observed; BuildWavePath records whoever wins the reload
			std::lock_guard<std::mutex> lock(g_wavePackMutex);
			for (const auto& reload : reloadBanks)
			{
				auto previous = loadedSources.find(reload.key);
				auto current = g_resourceBankSources.find(reload.key);
				if (previous != loadedSources.end() && current != g_resourceBankSources.end() &&
					_stricmp(previous->second.c_str(), current->second.c_str()) == 0)
				{
					g_resourceBankSources.erase(current);
				}
			}

			// mirror audWaveSlot's request state and let UpdateSlots perform the reload
			for (const auto& reload : reloadBanks)
			{
				auto bank = reload.state;
				bank->state |= kBankRequestChanged;
				bank->pendingName = bank->activeName;
				bank->pendingArgument = (bank->loaderType == 1) ? 0 : bank->activeArgument;
				bank->pendingPriority = bank->activePriority;
				bank->state |= kBankRequestPending;
				bank->requestTimestamp = g_getAudioRequestTimestamp();
			}
		}
		g_unlockAudioRequests(g_audioRequestLock);
	}

	if (!ready)
	{
		std::lock_guard<std::mutex> lock(g_wavePackMutex);
		g_wavePackChanges.insert(g_wavePackChanges.end(), changes.begin(), changes.end());
		return;
	}

	for (const auto& reload : reloadBanks)
	{
		trace("[rdr3-wavepack] reloading %s after resource wavepack change\n", reload.text.c_str());
	}
}
}

static HookFunction hookFunction([]()
{
	// b1491 wavepack functions
	// audWavePackDataFileMounter::mount
	auto mountWavePack = hook::get_pattern("48 89 5C 24 08 48 89 74 24 20 55 57 41 56 48 8D 6C 24 A0 48 81 EC 60 01 00 00 48 8B F2");
	// audWavePackDataFileMounter::unmount
	auto unmountWavePack = hook::get_pattern("48 89 5C 24 08 48 89 7C 24 18 55 48 8B EC 48 83 EC 50 48 8B C2 48 8B F9 48 8B C8 BA 2F 00 00 00");
	// rage::audWaveSlot::ComputeBankFilePath
	auto computeBankFilePath = hook::get_pattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 81 EC 40 01 00 00 49 8B F0 8B EA 48 8B D9");
	// rage::audWaveSlot::UpdateSlots, location of the slot count
	auto bankCount = hook::get_pattern<uint8_t>("8B 05 ? ? ? ? 33 DB 89 5D 30 44 8A EB 48 89 5C 24 20");
	// rage::audWaveSlot::UpdateSlots, location of the slot pool
	auto bankPool = hook::get_pattern<uint8_t>("48 03 3D ? ? ? ? 74 7E 8A 87 B5 00 00 00");
	// Unnamed rage::audWaveSlot bank request path
	auto requestBank = hook::get_pattern<uint8_t>("83 FA FF 0F 84 ? ? ? ? 48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B F9 41 8B E9 48 8D 0D ? ? ? ? 41 8B F0 8B DA E8 ? ? ? ?");
	// Unnamed rage::audWaveSlot load path, location of the bank-name registry
	auto audioName = hook::get_pattern<uint8_t>("8B 93 A4 00 00 00 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 85 C0 75 0B");

	g_audioBankCount = hook::get_address<uint32_t*>(bankCount + 2);
	g_audioBankPool = hook::get_address<uint8_t**>(bankPool + 3);
	g_audioRequestLock = hook::get_address<void*>(requestBank + 0x26);
	g_lockAudioRequests = reinterpret_cast<decltype(g_lockAudioRequests)>(hook::get_call(requestBank + 0x2F));
	g_getAudioRequestTimestamp = reinterpret_cast<decltype(g_getAudioRequestTimestamp)>(hook::get_call(requestBank + 0x86));
	g_unlockAudioRequests = reinterpret_cast<decltype(g_unlockAudioRequests)>(hook::get_call(requestBank + 0x95));
	g_audioNameRegistry = hook::get_address<void*>(audioName + 9);
	g_getAudioName = reinterpret_cast<decltype(g_getAudioName)>(hook::get_call(audioName + 13));

	MH_Initialize();
	if (MH_CreateHook(mountWavePack, MountWavePack, reinterpret_cast<void**>(&g_origMountWavePack)) != MH_OK ||
		MH_CreateHook(unmountWavePack, UnmountWavePack, reinterpret_cast<void**>(&g_origUnmountWavePack)) != MH_OK ||
		MH_CreateHook(computeBankFilePath, BuildWavePath, reinterpret_cast<void**>(&g_origBuildWavePath)) != MH_OK ||
		MH_EnableHook(mountWavePack) != MH_OK ||
		MH_EnableHook(unmountWavePack) != MH_OK ||
		MH_EnableHook(computeBankFilePath) != MH_OK)
	{
		FatalError("Could not install the RDR3 audio wavepack hooks.");
	}

	// resources can mount before or after audio init;
	// only late mounts need banks requeued
	rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_CORE)
		{
			g_audioInitialized.store(false, std::memory_order_release);
			std::lock_guard<std::mutex> lock(g_wavePackMutex);
			g_wavePackChanges.clear();
			g_resourceBankSources.clear();
		}
	});

	rage::OnInitFunctionInvoked.Connect([](rage::InitFunctionType type, const rage::InitFunctionData& data)
	{
		auto name = data.GetName();
		if (type == rage::INIT_CORE && name && strcmp(name, "audNorthAudioEngine") == 0)
		{
			g_audioInitialized.store(true, std::memory_order_release);
		}
	});

	OnMainGameFrame.Connect(ReloadChangedWaveBanks);
});
