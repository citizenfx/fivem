#include <StdInc.h>
#include <Hooking.h>

#include <fiDevice.h>
#include <fiCustomDevice.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <MinHook.h>

static std::map<std::tuple<std::string, std::string>, rage::fiDevice*> g_pathsToDevices;
static std::unordered_map<std::string, size_t> g_nameRefCounts;

static const std::string_view g_basePacks[] =
{
	"ANIMALS",
	"ANIMALS_FAR",
	"ANIMALS_NEAR",
	"CUTSCENE_MASTERED_ONLY",
	"DLC_GTAO",
	"INTERACTIVE_MUSIC",
	"ONESHOT_AMBIENCE",
	"PAIN",
	"POLICE_SCANNER",
	"PROLOGUE",
	"RADIO_01_CLASS_ROCK",
	"RADIO_02_POP",
	"RADIO_03_HIPHOP_NEW",
	"RADIO_04_PUNK",
	"RADIO_05_TALK_01",
	"RADIO_06_COUNTRY",
	"RADIO_07_DANCE_01",
	"RADIO_08_MEXICAN",
	"RADIO_09_HIPHOP_OLD",
	"RADIO_11_TALK_02",
	"RADIO_12_REGGAE",
	"RADIO_13_JAZZ",
	"RADIO_14_DANCE_02",
	"RADIO_15_MOTOWN",
	"RADIO_16_SILVERLAKE",
	"RADIO_17_FUNK",
	"RADIO_18_90S_ROCK",
	"RADIO_ADVERTS",
	"RADIO_NEWS",
	"RESIDENT",
	"SCRIPT",
	"SS_AC",
	"SS_DE",
	"SS_FF",
	"SS_GM",
	"SS_NP",
	"SS_QR",
	"SS_ST",
	"SS_UZ",
	"STREAMED_AMBIENCE",
	"STREAMED_VEHICLES",
	"STREAMED_VEHICLES_GRANULAR",
	"STREAMED_VEHICLES_GRANULAR_NPC",
	"STREAMED_VEHICLES_LOW_LATENCY",
	"STREAMS",
	"S_FULL_AMB_F",
	"S_FULL_AMB_M",
	"S_FULL_GAN",
	"S_FULL_SER",
	"S_MINI_AMB",
	"S_MINI_GAN",
	"S_MINI_SER",
	"S_MISC",
	"WEAPONS_PLAYER",
};

static void (*g_origAddWavePack)(const char* name, const char* path);

// equivalent of fiDeviceRelative, but it resolves the device on open, not on init
class MultiRelativeDevice : public rage::fiCustomDevice
{
private:
	struct HandleData
	{
		rage::fiDevice* device = nullptr;
		uint64_t handle = -1;
	};

public:
	auto Get(const std::string& fileName)
	{
		auto fileNameRel = m_relativeTo + fileName.substr(m_stripLength);

		return std::make_tuple(rage::fiDevice::GetDevice(fileNameRel.c_str(), true), fileNameRel);
	}

	auto Get(uint64_t handle) -> HandleData*
	{
		if (m_handles[handle].device)
		{
			return &m_handles[handle];
		}

		return nullptr;
	}

	auto Allocate()
	{
		for (int i = 0; i < std::size(m_handles); i++)
		{
			if (m_handles[i].device == nullptr)
			{
				return i;
			}
		}

		return -1;
	}

	virtual uint64_t Open(const char* fileName, bool readOnly) override
	{
		auto [d, n] = Get(fileName);

		if (d)
		{
			auto h = d->Open(n.c_str(), readOnly);

			if (h != -1)
			{
				uint64_t rh = Allocate();
				auto dt = &m_handles[rh];
				dt->device = d;
				dt->handle = h;

				return rh;
			}
		}

		return -1;
	}

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr) override
	{
		auto [d, n] = Get(fileName);

		if (d)
		{
			auto h = d->OpenBulk(n.c_str(), ptr);

			if (h != -1)
			{
				uint64_t rh = Allocate();
				auto dt = &m_handles[rh];
				dt->device = d;
				dt->handle = h;

				return rh;
			}
		}

		return -1;
	}

	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*) override
	{
		return OpenBulk(fileName, ptr);
	}

	virtual uint64_t Create(const char* fileName) override
	{
		return -1;
	}

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead) override
	{
		auto dt = Get(handle);
		return dt->device->Read(dt->handle, buffer, toRead);
	}

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead) override
	{
		auto dt = Get(handle);
		return dt->device->ReadBulk(dt->handle, ptr, buffer, toRead);
	}

	virtual int32_t GetCollectionId() override
	{
		return 0;
	}

	virtual uint32_t Write(uint64_t, void*, int) override
	{
		return -1;
	}

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int) override
	{
		return -1;
	}

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method) override
	{
		auto dt = Get(handle);
		return dt->device->Seek(dt->handle, distance, method);
	}

	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method) override
	{
		auto dt = Get(handle);
		return dt->device->SeekLong(dt->handle, distance, method);
	}

	virtual int32_t Close(uint64_t handle) override
	{
		auto dt = Get(handle);
		auto r = dt->device->Close(dt->handle);

		dt->device = nullptr;
		dt->handle = -1;

		return r;
	}

	virtual int32_t CloseBulk(uint64_t handle) override
	{
		auto dt = Get(handle);
		auto r = dt->device->CloseBulk(dt->handle);

		dt->device = nullptr;
		dt->handle = -1;

		return r;
	}

	virtual int GetFileLength(uint64_t handle) override
	{
		auto dt = Get(handle);
		return dt->device->GetFileLength(dt->handle);
	}

	virtual uint64_t GetFileLengthLong(const char* fileName) override
	{
		auto [d, f] = Get(fileName);

		if (d)
		{
			return d->GetFileLengthLong(f.c_str());
		}

		return -1;
	}

	virtual uint64_t GetFileLengthUInt64(uint64_t handle) override
	{
		auto dt = Get(handle);
		return dt->device->GetFileLengthUInt64(dt->handle);
	}

	virtual bool RemoveFile(const char* file) override
	{
		return false;
	}

	virtual int RenameFile(const char* from, const char* to) override
	{
		return false;
	}

	virtual int CreateDirectory(const char* dir) override
	{
		return false;
	}

	virtual int RemoveDirectory(const char* dir) override
	{
		return false;
	}

	virtual uint64_t GetFileTime(const char* file) override
	{
		auto [d, f] = Get(file);

		if (d)
		{
			return d->GetFileTime(f.c_str());
		}

		return -1;
	}

	virtual bool SetFileTime(const char* file, FILETIME fileTime) override
	{
		return false;
	}

	virtual uint32_t GetFileAttributes(const char* path) override
	{
		auto [d, f] = Get(path);

		if (d)
		{
			return d->GetFileAttributes(f.c_str());
		}

		return -1;
	}

	virtual int m_yx() override
	{
		return 0;
	}

	virtual bool IsCollection() override
	{
		return false;
	}

	virtual const char* GetName() override
	{
		return "AWP";
	}

	virtual int GetResourceVersion(const char* fileName, rage::ResourceFlags* version) override
	{
		auto [d, f] = Get(fileName);

		if (d)
		{
			return d->GetResourceVersion(f.c_str(), version);
		}

		return -1;
	}

	virtual uint64_t CreateLocal(const char* fileName) override
	{
		return -1;
	}

	virtual void* m_xy(void* a, int len, void* c) override
	{
		return nullptr;
	}

	void SetPath(const std::string& path, bool)
	{
		m_relativeTo = path;
	}

	void Mount(const char* at)
	{	
		m_stripLength = strlen(at);
		rage::fiDevice::MountGlobal(at, this, true);
	}

private:
	size_t m_stripLength = 0;
	std::string m_relativeTo;

	HandleData m_handles[512];
};

static void AddWavePack(const char* name, const char* path)
{
	auto wavePath = fmt::sprintf("awc:/%s/", name);
	
	auto addPath = [&wavePath](const char* path)
	{
		std::string pathStr = path;
		if (!boost::algorithm::ends_with(pathStr, "/"))
		{
			pathStr += "/";
		}

		if (g_pathsToDevices.find({ pathStr, wavePath }) == g_pathsToDevices.end())
		{
			auto device = new MultiRelativeDevice();
			device->SetPath(pathStr, true);
			device->Mount(wavePath.c_str());

			g_pathsToDevices.insert({ { pathStr, wavePath }, device });
		}
	};

	addPath(path);

	if (g_nameRefCounts[name] == 0)
	{
		// add any pack that would match an original pack (as 8-character limit)
		// as audio:/sfx/ would not match a packfile subdevice, we add the actual packfile mounts for each as a submount here
		for (auto pack : g_basePacks)
		{
			if (boost::algorithm::iequals(pack.substr(0, std::min(size_t(8), pack.length())), name))
			{
				addPath("audio:/sfx/");
			}
		}

		g_origAddWavePack(name, wavePath.c_str());
	}

	g_nameRefCounts[name]++;
}

static thread_local std::string g_lastWavePackPath;

static void (*g_origRemoveWavePack)(const char* name);

static void RemoveWavePack(const char* name)
{
	auto wavePath = fmt::sprintf("awc:/%s/", name);

	if (auto it = g_pathsToDevices.find({ g_lastWavePackPath, wavePath }); it != g_pathsToDevices.end())
	{
		rage::fiDevice::Unmount(*it->second);
		g_pathsToDevices.erase(it);
	}

	// remove underlying wavepack, if needed
	g_nameRefCounts[name]--;

	if (g_nameRefCounts[name] == 0)
	{
		g_origRemoveWavePack(name);
	}
}

static bool (*g_origUnmountWavePack)(void* self, const void* entry);

static bool UnmountWavePack(void* self, const char* entry)
{
	// memorize this for RemoveWavePack to use
	g_lastWavePackPath = entry;

	return g_origUnmountWavePack(self, entry);
}

static HookFunction hookFunction([]
{
	{
		auto location = hook::get_pattern("C6 04 02 00 48 8D 4C 24 20 48 8D", 14);
		hook::set_call(&g_origAddWavePack, location);
		hook::call(location, AddWavePack);
	}

	MH_Initialize();

	{
		auto location = hook::get_pattern<char>("48 8D 4C 24 20 C6 44 24 28 00 E8", 10);
		MH_CreateHook(location - 0x3F, UnmountWavePack, (void**)&g_origUnmountWavePack);
		hook::set_call(&g_origRemoveWavePack, location);
		hook::call(location, RemoveWavePack);
	}

	MH_EnableHook(MH_ALL_HOOKS);
});
