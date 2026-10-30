#include <StdInc.h>

#include <Streaming.h>
#include <CoreConsole.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

static hook::cdecl_stub<void*()> _getInstance([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 44 8B 44 24 ? 48 8B C8 33 C0"));
});

static bool (*_loadCatalogEntry)(void*, void*);
static void* (*_constructCatalogEntry)(void*, int, const CDataFileMgr::DataFile*);
static bool AddCatalogFile(CDataFileMgr::DataFile* catalogEntry)
{
	unsigned char storage[3120] = { 0 };
	_constructCatalogEntry(storage, catalogEntry->type, catalogEntry);
	if (_loadCatalogEntry(_getInstance(), storage))
	{
		console::DPrintf("streaming", "Loaded catalog entry '%s'\n", catalogEntry->name);
		return true;
	}

	console::PrintWarning("streaming", "Failed to load catalog entry %s, using default catalog\n", catalogEntry->name);
	return false;
}

static rage::sysMemAllocator* g_streamingAllocator;

// Catalog files are expected to be packaged in a packfile with the same name
// We can't easily support this in RedM so we need to handle this ourselves.
static void*(*g_openCatalogPackFile)(const char* filePath, int* length);
static void* OpenCatalogPackFile(const char* filePath, int* length)
{
	if (strnicmp(filePath, "resources:/", 11) != 0)
	{
		return g_openCatalogPackFile(filePath, length);
	}

	rage::fiDevice* device = rage::fiDevice::GetDevice(filePath, true);
	if (!device)
	{
		return nullptr;
	}

	uint64_t handle = device->Open(filePath, true);
	if (handle == uint64_t(-1))
	{
		return nullptr;
	}

	int fileLength = device->GetFileLength(handle);
	auto block = g_streamingAllocator->Allocate(fileLength, 16, 0);
	int read = device->Read(handle, block, fileLength);

	constexpr uint32_t kFileMagic = 0x4E495350;
	// Ensure the file is valid
	uint32_t* magic = (uint32_t*)block;
	if (read <= 24 || *magic != kFileMagic)
	{
		// If its compressed the file magic is shifted +8 bytes.
		// We can't currently parse this if its compressed so warn the user.
		if (read >= 12 && *(uint32_t*)((uint64_t)block + 8) == kFileMagic)
		{
			trace("Unable to read catalog entry, This file is compressed\n");
		}
		else
		{
			trace("Unable to use catalog entry, This file is not valid\n");
		}
		g_streamingAllocator->Free(block);
		device->Close(handle);
		return nullptr;
	}

	*length = read;
	device->Close(handle);

	// The game handles freeing this.
	return block;
}

constexpr char DEFAULT_CATALOG_SP_FILE[] = "platform:/data/itemdatabase/catalog_sp";
constexpr char DEFAULT_CATALOG_MP_FILE[] = "platform:/data/itemdatabase/catalog_mp";
constexpr char DEFAULT_CATALOG_AWARDS_FILE[] = "platform:/data/itemdatabase/catalog_awards_mp";

static std::string g_catalogMpOverridePath = "";
static std::string g_catalogAwardsPath = "";

namespace streaming
{
	void DLL_EXPORT SetCatalogOverridePath(const std::string& path)
	{
		g_catalogMpOverridePath = path;
	}

	void DLL_EXPORT SetAwardCatalogOverridePath(const std::string& path)
	{
		g_catalogAwardsPath = path;
	}
}

static void* (*_initCatalog)(char* catalog);
static uint32_t g_mpCatalogOffset = 0;

static void (*g_origLoadCatalogFile)(hook::FlexStruct*, int);
static void LoadCatalogFile(hook::FlexStruct* itemdbgMgr, int initState)
{
	// In order to allow weapon replacements in resources
	// Delay weapon loading until later.
	if (initState != 8)
	{
		return;
	}

	_initCatalog(&itemdbgMgr->At<char>(0));
	_initCatalog(&itemdbgMgr->At<char>(g_mpCatalogOffset));

    auto addCatalog = [](int type, const std::string& overridePath, const char* defaultPath)
	{
		CDataFileMgr::DataFile dataFile{};
		dataFile.type = type;
		const char* path = overridePath.empty() ? defaultPath : overridePath.c_str();
		strncpy_s(dataFile.name, sizeof(dataFile.name), path, _TRUNCATE);
		
		bool result = AddCatalogFile(&dataFile);
		// Override file failed to be loaded, use default otherwise the game will break
		if (!result && !overridePath.empty())
		{
			strncpy_s(dataFile.name, sizeof(dataFile.name), defaultPath, _TRUNCATE);
			AddCatalogFile(&dataFile);
		}
	};

	addCatalog(343, "", DEFAULT_CATALOG_SP_FILE);
	addCatalog(344, g_catalogMpOverridePath, DEFAULT_CATALOG_MP_FILE);
	addCatalog(345, g_catalogAwardsPath, DEFAULT_CATALOG_AWARDS_FILE);
	g_origLoadCatalogFile(itemdbgMgr, 8);
}

static HookFunction hookFunction([]()
{
	g_streamingAllocator = hook::get_address<rage::sysMemAllocator*>(hook::get_pattern("48 8D 0D ? ? ? ? 44 8D 78", 3));
	g_openCatalogPackFile = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 4C 8B F2"), OpenCatalogPackFile);
	
	// Patch catalog loading to allow resources to override catalogs
	{
		auto location = (uintptr_t)hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B F1 83 FA");
		g_origLoadCatalogFile = hook::trampoline((void*)location, LoadCatalogFile);

		hook::set_call(&_initCatalog, location + 0x1E);
		hook::set_call(&_constructCatalogEntry, location + 0x5A);
		hook::set_call(&_loadCatalogEntry, location + 0x67);
		g_mpCatalogOffset = *(uint32_t*)(location + 38);
	}
});
