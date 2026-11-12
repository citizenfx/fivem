#include <StdInc.h>

#include <Hooking.h>
#include <MinHook.h>

#include <EntitySystem.h>
#include <ScriptEngine.h>
#include <Streaming.h>
#include <VFSManager.h>
#include <grcTexture.h>
#include <nutsnbolts.h>
#include <sysAllocator.h>

#include <tinyxml2.h>

#include <atomic>
#include <map>
#include <set>

//
// Add-on vehicle number plate styles.
//
// The game keeps its plate styles in `CVehicleModelInfoVarGlobal::VehiclePlates::Textures` (from carcols.ymt),
// and every consumer (SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX, network sync, GET_NUMBER_OF_VEHICLE_NUMBER_PLATES,
// the renderer) only bounds the plate index by that array's count. So instead of special-casing indices, we
// append real entries to that array: the game then renders custom plates (including font color/placement) the
// exact same way it renders vanilla ones. The only thing the game can't do itself is find the textures, as it
// only looks in `vehshare`, so we fill in the texture pointers for our entries after it applies a plate style.
//
// Servers ship these as `data_file 'VEHICLE_PLATES_FILE' 'plates.meta'`, using the carcols.ymt item format
// plus a `TextureDictionary` (and optionally an `Index`).
//

void ForAllVehicles(const std::function<void(fwEntity*)>& fn);
rage::grcTexture* LookupTexture(const std::string& txd, const std::string& txn);

namespace
{
// one item in VehiclePlates.Textures, offsets are verified against the game's parser metadata
struct PlateTextureSet
{
	float fontExtents[4]; // +0x00
	float maxLettersOnPlate[2]; // +0x10
	float fontOutlineMinMaxDepth[2]; // +0x18
	uint32_t textureSetName; // +0x20
	uint32_t diffuseMapName; // +0x24
	uint32_t normalMapName; // +0x28
	uint32_t fontColor; // +0x2C
	uint32_t fontOutlineColor; // +0x30
	bool isFontOutlineEnabled; // +0x34
	uint8_t pad[11];
};

static_assert(sizeof(PlateTextureSet) == 0x40);

// atArray<PlateTextureSet> as laid out in game memory
struct PlateArray
{
	PlateTextureSet* entries;
	uint16_t count;
	uint16_t capacity;
};

// which fields a plate file specified, anything else is taken from the default plate
enum PlateField : uint32_t
{
	Field_FontExtents = 1 << 0,
	Field_MaxLettersOnPlate = 1 << 1,
	Field_FontOutlineMinMaxDepth = 1 << 2,
	Field_NormalMapName = 1 << 3,
	Field_FontColor = 1 << 4,
	Field_FontOutlineColor = 1 << 5,
	Field_IsFontOutlineEnabled = 1 << 6,
};

struct CustomPlate
{
	std::string sourceFile;
	std::string name;
	std::string txd;
	std::string diffuseName;
	std::string normalName;

	PlateTextureSet values{};
	uint32_t fields = 0;
	int requestedIndex = -1;

	uint32_t txdSlot = -1;
	bool holdsTxdRef = false;
	bool texturesResolved = false;
	bool warnedMissingTxd = false;

	rage::grcTexture* diffuse = nullptr;
	rage::grcTexture* normal = nullptr;
};

// upper bound for the plate array, the game's count is a 16-bit value
constexpr int kMaxPlateCount = 4096;

// CVehicleDrawHandler -> CCustomShaderEffectVehicle
constexpr int kShaderEffectOffset = 0x20;

// how long to wait before shrinking the array or freeing an old one, as draw lists built in previous frames
// may still reference plate indices/entries on the render thread
constexpr int kDeferFrames = 10;
}

static void** g_vehicleModelInfoVarGlobal;
static uint32_t g_plateArrayOffset;
static uint32_t g_defaultPlateIndexOffset;
static uint32_t g_effectPlateIndexOffset;
static uint32_t g_effectDiffuseOffset;
static uint32_t g_effectNormalOffset;
static uint8_t g_drawHandlerOffset;

static void (*g_origSetLicensePlateTexture)(char* effect, int index);

static std::map<int, CustomPlate> g_customPlates;

// the game's own plate array, which we never free
static PlateTextureSet* g_gameEntries;
static uint16_t g_gameCount;
static uint16_t g_gameCapacity;

// our extended copy of it, if installed
static PlateTextureSet* g_ownEntries;
static uint16_t g_ownCapacity;

static std::vector<std::pair<PlateTextureSet*, int>> g_retiredEntries;
static int g_frameCounter;
static int g_trimAtFrame = -1;

static PlateArray* GetPlateArray()
{
	auto varGlobal = static_cast<char*>(*g_vehicleModelInfoVarGlobal);

	if (!varGlobal)
	{
		return nullptr;
	}

	auto array = reinterpret_cast<PlateArray*>(varGlobal + g_plateArrayOffset);
	return (array->entries) ? array : nullptr;
}

// re-captures the game's array if it isn't ours, e.g. when carcols got (re)loaded
static PlateArray* SyncWithGame()
{
	auto array = GetPlateArray();

	if (array && array->entries != g_ownEntries)
	{
		// if we had our own array installed, the game dropped it, so it isn't ours to free anymore
		g_ownEntries = nullptr;
		g_ownCapacity = 0;

		g_gameEntries = array->entries;
		g_gameCount = array->count;
		g_gameCapacity = array->capacity;
	}

	return array;
}

static int GetDefaultPlateIndex()
{
	auto varGlobal = static_cast<char*>(*g_vehicleModelInfoVarGlobal);
	int index = (varGlobal) ? *reinterpret_cast<int*>(varGlobal + g_defaultPlateIndexOffset) : 0;

	return (index >= 0 && index < g_gameCount) ? index : 0;
}

static PlateTextureSet BuildEntry(const CustomPlate& plate)
{
	// start from the default plate so anything not specified looks vanilla
	PlateTextureSet entry = g_gameEntries[GetDefaultPlateIndex()];
	const auto& v = plate.values;

	entry.textureSetName = v.textureSetName;
	entry.diffuseMapName = v.diffuseMapName;

	if (plate.fields & Field_FontExtents)
	{
		memcpy(entry.fontExtents, v.fontExtents, sizeof(entry.fontExtents));
	}

	if (plate.fields & Field_MaxLettersOnPlate)
	{
		memcpy(entry.maxLettersOnPlate, v.maxLettersOnPlate, sizeof(entry.maxLettersOnPlate));
	}

	if (plate.fields & Field_FontOutlineMinMaxDepth)
	{
		memcpy(entry.fontOutlineMinMaxDepth, v.fontOutlineMinMaxDepth, sizeof(entry.fontOutlineMinMaxDepth));
	}

	if (plate.fields & Field_NormalMapName)
	{
		entry.normalMapName = v.normalMapName;
	}

	if (plate.fields & Field_FontColor)
	{
		entry.fontColor = v.fontColor;
	}

	if (plate.fields & Field_FontOutlineColor)
	{
		entry.fontOutlineColor = v.fontOutlineColor;
	}

	if (plate.fields & Field_IsFontOutlineEnabled)
	{
		entry.isFontOutlineEnabled = v.isFontOutlineEnabled;
	}

	return entry;
}

static int GetWantedCount()
{
	return (g_customPlates.empty()) ? g_gameCount : std::max<int>(g_gameCount, g_customPlates.rbegin()->first + 1);
}

static void RetireEntries(PlateTextureSet* entries)
{
	if (entries)
	{
		g_retiredEntries.emplace_back(entries, g_frameCounter);
	}
}

//
// Writes the registered plates into the game's plate array, growing it when needed. Unregistered indices in
// the custom range become copies of the default plate. The array is only shrunk when `allowShrink` is set,
// which callers only do once no vehicle uses an index past the new end.
//
static void ApplyPlates(bool allowShrink)
{
	auto array = SyncWithGame();

	if (!array)
	{
		return;
	}

	int wanted = GetWantedCount();
	int newCount = (allowShrink) ? wanted : std::max<int>(wanted, array->count);

	if (newCount <= g_gameCount)
	{
		if (g_ownEntries)
		{
			array->count = g_gameCount;
			array->entries = g_gameEntries;
			array->capacity = g_gameCapacity;

			RetireEntries(g_ownEntries);
			g_ownEntries = nullptr;
			g_ownCapacity = 0;
		}

		return;
	}

	PlateTextureSet* entries = g_ownEntries;

	if (!entries || g_ownCapacity < newCount)
	{
		int capacity = std::min(kMaxPlateCount, (newCount + 31) & ~31);
		entries = static_cast<PlateTextureSet*>(rage::GetAllocator()->Allocate(sizeof(PlateTextureSet) * capacity, 16, 0));

		memcpy(entries, g_gameEntries, sizeof(PlateTextureSet) * g_gameCount);

		RetireEntries(g_ownEntries);

		g_ownEntries = entries;
		g_ownCapacity = capacity;
	}

	auto& defaultEntry = g_gameEntries[GetDefaultPlateIndex()];

	for (int i = g_gameCount; i < newCount; i++)
	{
		auto it = g_customPlates.find(i);
		entries[i] = (it != g_customPlates.end()) ? BuildEntry(it->second) : defaultEntry;
	}

	// the render thread reads entries through this pointer, so publish it only once they're written
	std::atomic_thread_fence(std::memory_order_release);

	array->entries = entries;
	array->capacity = g_ownCapacity;
	array->count = newCount;
}

static void ForEachPlateEffect(const std::function<void(char* effect)>& fn)
{
	ForAllVehicles([&fn](fwEntity* vehicle)
	{
		auto drawHandler = *reinterpret_cast<char**>(reinterpret_cast<char*>(vehicle) + g_drawHandlerOffset);

		if (!drawHandler)
		{
			return;
		}

		if (auto effect = *reinterpret_cast<char**>(drawHandler + kShaderEffectOffset))
		{
			fn(effect);
		}
	});
}

static int& GetEffectPlateIndex(char* effect)
{
	return *reinterpret_cast<int*>(effect + g_effectPlateIndexOffset);
}

static void SetLicensePlateTextureHook(char* effect, int index)
{
	g_origSetLicensePlateTexture(effect, index);

	// rejected by the game, or not ours
	if (index < 0 || GetEffectPlateIndex(effect) != index)
	{
		return;
	}

	auto it = g_customPlates.find(index);

	if (it == g_customPlates.end())
	{
		return;
	}

	if (auto diffuse = it->second.diffuse)
	{
		*reinterpret_cast<rage::grcTexture**>(effect + g_effectDiffuseOffset) = diffuse;
	}

	if (auto normal = it->second.normal)
	{
		*reinterpret_cast<rage::grcTexture**>(effect + g_effectNormalOffset) = normal;
	}
}

static void ReapplyPlate(char* effect, int index)
{
	// the game skips applying a plate style when the index didn't change
	GetEffectPlateIndex(effect) = -1;
	SetLicensePlateTextureHook(effect, index);
}

static streaming::strStreamingModule* GetTxdStore()
{
	return streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ytd");
}

// returns true once the plate's textures became available
static bool ResolveTextures(CustomPlate& plate)
{
	if (plate.texturesResolved)
	{
		return false;
	}

	auto txdStore = GetTxdStore();

	if (plate.txdSlot == -1)
	{
		txdStore->FindSlot(&plate.txdSlot, plate.txd.c_str());

		if (plate.txdSlot == -1)
		{
			if (!plate.warnedMissingTxd)
			{
				trace("Vehicle plate '%s' (%s): texture dictionary '%s' is not streamed.\n", plate.name, plate.sourceFile, plate.txd);
				plate.warnedMissingTxd = true;
			}

			return false;
		}
	}

	if (!txdStore->GetPtr(plate.txdSlot))
	{
		streaming::Manager::GetInstance()->RequestObject(txdStore->baseIdx + plate.txdSlot, 0);
		return false;
	}

	// keep the dictionary loaded for as long as vehicles may point at its textures
	txdStore->AddRef(plate.txdSlot);
	plate.holdsTxdRef = true;
	plate.texturesResolved = true;

	plate.diffuse = LookupTexture(plate.txd, plate.diffuseName);

	if (!plate.diffuse)
	{
		trace("Vehicle plate '%s' (%s): texture '%s' was not found in '%s'.\n", plate.name, plate.sourceFile, plate.diffuseName, plate.txd);
	}

	if (!plate.normalName.empty())
	{
		plate.normal = LookupTexture(plate.txd, plate.normalName);

		if (!plate.normal)
		{
			trace("Vehicle plate '%s' (%s): texture '%s' was not found in '%s'.\n", plate.name, plate.sourceFile, plate.normalName, plate.txd);
		}
	}

	return true;
}

static void ReleaseTextures(CustomPlate& plate)
{
	if (plate.holdsTxdRef)
	{
		GetTxdStore()->RemoveRef(plate.txdSlot);
		plate.holdsTxdRef = false;
	}

	plate.diffuse = nullptr;
	plate.normal = nullptr;
}

static void UpdatePendingTextures()
{
	for (auto& [index, plate] : g_customPlates)
	{
		if (ResolveTextures(plate))
		{
			// vehicles that already got this style were missing its textures
			ForEachPlateEffect([index = index](char* effect)
			{
				if (GetEffectPlateIndex(effect) == index)
				{
					ReapplyPlate(effect, index);
				}
			});
		}
	}
}

static std::string GetChildText(const tinyxml2::XMLElement* element, const char* name)
{
	auto child = element->FirstChildElement(name);
	auto text = (child) ? child->GetText() : nullptr;

	return (text) ? text : "";
}

static bool ReadVector(const tinyxml2::XMLElement* element, const char* name, float* out, std::initializer_list<const char*> components)
{
	auto child = element->FirstChildElement(name);

	if (!child)
	{
		return false;
	}

	for (auto component : components)
	{
		child->QueryFloatAttribute(component, out++);
	}

	return true;
}

static bool ReadColor(const tinyxml2::XMLElement* element, const char* name, uint32_t* out)
{
	auto child = element->FirstChildElement(name);
	auto value = (child) ? child->Attribute("value") : nullptr;

	if (!value)
	{
		return false;
	}

	*out = strtoul(value, nullptr, 0);
	return true;
}

static bool ParsePlateFile(const char* fileName, std::vector<CustomPlate>& plates)
{
	fwRefContainer<vfs::Stream> stream = vfs::OpenRead(fileName);

	if (!stream.GetRef())
	{
		trace("Could not open vehicle plates file %s.\n", fileName);
		return false;
	}

	auto data = stream->ReadToEnd();

	tinyxml2::XMLDocument document;

	if (document.Parse(reinterpret_cast<const char*>(data.data()), data.size()) != tinyxml2::XML_SUCCESS)
	{
		trace("Could not parse vehicle plates file %s: %s at line %d\n", fileName, document.ErrorName(), document.GetErrorLineNum());
		return false;
	}

	// accept a full carcols-style document, a bare VehiclePlates node or a bare Textures node
	const tinyxml2::XMLElement* textures = document.RootElement();

	if (textures && strcmp(textures->Name(), "Textures") != 0)
	{
		auto vehiclePlates = (strcmp(textures->Name(), "VehiclePlates") == 0) ? textures : textures->FirstChildElement("VehiclePlates");
		textures = (vehiclePlates) ? vehiclePlates->FirstChildElement("Textures") : nullptr;
	}

	if (!textures)
	{
		trace("Vehicle plates file %s has no VehiclePlates/Textures node.\n", fileName);
		return false;
	}

	for (auto item = textures->FirstChildElement("Item"); item; item = item->NextSiblingElement("Item"))
	{
		CustomPlate plate;
		plate.sourceFile = fileName;
		plate.name = GetChildText(item, "TextureSetName");
		plate.txd = GetChildText(item, "TextureDictionary");
		plate.diffuseName = GetChildText(item, "DiffuseMapName");
		plate.normalName = GetChildText(item, "NormalMapName");

		if (plate.name.empty() || plate.txd.empty() || plate.diffuseName.empty())
		{
			trace("Vehicle plates file %s: skipping an item without TextureSetName, TextureDictionary or DiffuseMapName.\n", fileName);
			continue;
		}

		auto& v = plate.values;
		v.textureSetName = HashString(plate.name);
		v.diffuseMapName = HashString(plate.diffuseName);

		if (!plate.normalName.empty())
		{
			v.normalMapName = HashString(plate.normalName);
			plate.fields |= Field_NormalMapName;
		}

		if (ReadVector(item, "FontExtents", v.fontExtents, { "x", "y", "z", "w" }))
		{
			plate.fields |= Field_FontExtents;
		}

		if (ReadVector(item, "MaxLettersOnPlate", v.maxLettersOnPlate, { "x", "y" }))
		{
			plate.fields |= Field_MaxLettersOnPlate;
		}

		if (ReadVector(item, "FontOutlineMinMaxDepth", v.fontOutlineMinMaxDepth, { "x", "y" }))
		{
			plate.fields |= Field_FontOutlineMinMaxDepth;
		}

		if (ReadColor(item, "FontColor", &v.fontColor))
		{
			plate.fields |= Field_FontColor;
		}

		if (ReadColor(item, "FontOutlineColor", &v.fontOutlineColor))
		{
			plate.fields |= Field_FontOutlineColor;
		}

		if (auto outline = item->FirstChildElement("IsFontOutlineEnabled"))
		{
			outline->QueryBoolAttribute("value", &v.isFontOutlineEnabled);
			plate.fields |= Field_IsFontOutlineEnabled;
		}

		if (auto index = item->FirstChildElement("Index"))
		{
			index->QueryIntAttribute("value", &plate.requestedIndex);
		}

		plates.push_back(std::move(plate));
	}

	return true;
}

static int FindPlateIndexByName(uint32_t nameHash)
{
	auto array = SyncWithGame();

	if (!array)
	{
		return -1;
	}

	for (int i = 0; i < array->count; i++)
	{
		if (array->entries[i].textureSetName == nameHash)
		{
			return i;
		}
	}

	return -1;
}

static int AllocatePlateIndex(const CustomPlate& plate)
{
	if (plate.requestedIndex >= 0)
	{
		if (plate.requestedIndex < g_gameCount || plate.requestedIndex >= kMaxPlateCount)
		{
			trace("Vehicle plate '%s' (%s): Index %d is out of range (%d-%d).\n", plate.name, plate.sourceFile, plate.requestedIndex, g_gameCount, kMaxPlateCount - 1);
			return -1;
		}

		if (auto it = g_customPlates.find(plate.requestedIndex); it != g_customPlates.end())
		{
			trace("Vehicle plate '%s' (%s): Index %d is already used by '%s' (%s).\n", plate.name, plate.sourceFile, plate.requestedIndex, it->second.name, it->second.sourceFile);
			return -1;
		}

		return plate.requestedIndex;
	}

	for (int i = g_gameCount; i < kMaxPlateCount; i++)
	{
		if (g_customPlates.find(i) == g_customPlates.end())
		{
			return i;
		}
	}

	trace("Vehicle plate '%s' (%s): no free plate index left.\n", plate.name, plate.sourceFile);
	return -1;
}

static bool LoadPlateFile(const char* fileName)
{
	std::vector<CustomPlate> plates;

	if (!ParsePlateFile(fileName, plates))
	{
		return false;
	}

	if (!SyncWithGame())
	{
		trace("Could not load vehicle plates file %s: the game's plate data isn't loaded.\n", fileName);
		return false;
	}

	for (auto& plate : plates)
	{
		if (FindPlateIndexByName(plate.values.textureSetName) >= 0)
		{
			trace("Vehicle plate '%s' (%s): a plate with this TextureSetName already exists.\n", plate.name, plate.sourceFile);
			continue;
		}

		int index = AllocatePlateIndex(plate);

		if (index < 0)
		{
			continue;
		}

		auto& registered = g_customPlates.emplace(index, std::move(plate)).first->second;
		ApplyPlates(false);
		ResolveTextures(registered);

		trace("Registered vehicle plate '%s' as index %d.\n", registered.name, index);
	}

	return true;
}

static void UnloadPlateFile(const char* fileName)
{
	std::vector<CustomPlate> removed;
	std::set<int> removedIndices;

	for (auto it = g_customPlates.begin(); it != g_customPlates.end();)
	{
		if (it->second.sourceFile == fileName)
		{
			removedIndices.insert(it->first);
			removed.push_back(std::move(it->second));
			it = g_customPlates.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (removed.empty())
	{
		return;
	}

	// removed indices turn into copies of the default plate until the array can be shrunk
	ApplyPlates(false);

	int defaultIndex = GetDefaultPlateIndex();

	ForEachPlateEffect([&removedIndices, defaultIndex](char* effect)
	{
		if (removedIndices.find(GetEffectPlateIndex(effect)) != removedIndices.end())
		{
			ReapplyPlate(effect, defaultIndex);
		}
	});

	// no vehicle points at these textures anymore
	for (auto& plate : removed)
	{
		ReleaseTextures(plate);
	}

	g_trimAtFrame = g_frameCounter + kDeferFrames;
}

static void TrimPlates()
{
	int wanted = GetWantedCount();
	int defaultIndex = GetDefaultPlateIndex();
	bool inUse = false;

	ForEachPlateEffect([wanted, defaultIndex, &inUse](char* effect)
	{
		if (GetEffectPlateIndex(effect) >= wanted)
		{
			ReapplyPlate(effect, defaultIndex);
			inUse = true;
		}
	});

	// vehicles were still using indices past the end, give their draw lists time to catch up
	if (inUse)
	{
		g_trimAtFrame = g_frameCounter + kDeferFrames;
		return;
	}

	ApplyPlates(true);
	g_trimAtFrame = -1;
}

static void OnFrame()
{
	++g_frameCounter;

	for (auto it = g_retiredEntries.begin(); it != g_retiredEntries.end();)
	{
		if (g_frameCounter - it->second >= kDeferFrames)
		{
			rage::GetAllocator()->Free(it->first);
			it = g_retiredEntries.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (g_customPlates.empty() && !g_ownEntries)
	{
		return;
	}

	// the game reloaded its plate data, put ours back in
	if (SyncWithGame() && !g_ownEntries && !g_customPlates.empty())
	{
		ApplyPlates(false);
	}

	UpdatePendingTextures();

	if (g_trimAtFrame >= 0 && g_frameCounter >= g_trimAtFrame)
	{
		TrimPlates();
	}
}

static InitFunction initFunction([]()
{
	streaming::RegisterDataFileHandler("VEHICLE_PLATES_FILE", LoadPlateFile, UnloadPlateFile);

	OnMainGameFrame.Connect(OnFrame);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX_BY_NAME", [](fx::ScriptContext& context)
	{
		const char* name = context.GetArgument<const char*>(0);
		context.SetResult<int>((name) ? FindPlateIndexByName(HashString(name)) : -1);
	});
});

static HookFunction hookFunction([]()
{
	// CCustomShaderEffectVehicle::SetLicensePlateTexture(int index)
	auto setLicensePlateTexture = hook::get_pattern<char>("83 FA FF 0F 8C ? ? ? ? 48 8B C4 48 89 58 08");
	auto begin = reinterpret_cast<uintptr_t>(setLicensePlateTexture);
	auto end = begin + 0x180;

	{
		// mov rcx, CVehicleModelInfo::ms_VehicleColours; movzx eax, word ptr [rcx + Textures.count]
		auto location = hook::range_pattern(begin, end, "48 8B 0D ? ? ? ? 0F B7 81 ? ? ? ? 3B D0").get_one().get<char>(0);
		g_vehicleModelInfoVarGlobal = hook::get_address<void**>(location + 3);
		g_plateArrayOffset = *reinterpret_cast<uint32_t*>(location + 10) - offsetof(PlateArray, count);

		// mov rdi, [rcx + Textures.entries]
		auto entries = hook::range_pattern(begin, end, "48 8B B9 ? ? ? ?").get_one().get<uint32_t>(3);
		assert(*entries == g_plateArrayOffset);

		// shl r10, 6: the entry stride is sizeof(PlateTextureSet)
		hook::range_pattern(begin, end, "49 C1 E2 06").get_one();
	}

	g_effectPlateIndexOffset = *hook::range_pattern(begin, end, "41 89 93 ? ? ? ?").get_one().get<uint32_t>(3);

	{
		auto location = hook::range_pattern(begin, end, "49 89 9B ? ? ? ? 4D 89 8B ? ? ? ?").get_one().get<char>(0);
		g_effectDiffuseOffset = *reinterpret_cast<uint32_t*>(location + 3);
		g_effectNormalOffset = *reinterpret_cast<uint32_t*>(location + 10);
	}

	// VehiclePlates.DefaultTexureIndex, from the function that picks a vehicle's initial plate
	g_defaultPlateIndexOffset = *hook::get_pattern<uint32_t>("48 8B 05 ? ? ? ? 4C 8B C2 48 8B D9 49 8B 48 20 8B 90", 19);

	g_drawHandlerOffset = *hook::get_pattern<uint8_t>("44 0F 2F 43 48 45 8D", 4);

	MH_Initialize();
	MH_CreateHook(setLicensePlateTexture, SetLicensePlateTextureHook, (void**)&g_origSetLicensePlateTexture);
	MH_EnableHook(setLicensePlateTexture);
});