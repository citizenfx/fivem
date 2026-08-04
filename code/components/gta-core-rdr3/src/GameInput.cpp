#include "StdInc.h"
#include "GameInput.h"
#include "atArray.h"
#include "CustomText.h"
#include "Hooking.h"
#include "Hooking.Stubs.h"
#include <array>
#include <unordered_set>
#include "RageParser.h"
#include "console/Console.CommandHelpers.h"
#include <CoreConsole.h>
#include <set>

constexpr int UNBINDED_KEY = 0xFF000;
constexpr int MAPPING_COUNT = 2; // One for the first bind entry and another for the second bind
constexpr int MAX_CUSTOM_BINDINGS = 128;

static char* g_gameKeyArray = nullptr;

class CustomBinding
{
private:
	bool m_dummy = true;          // True if this slot is available for use
	bool m_wasDown = false;       // Internal state for processing updates
	bool m_suppressUntilRelease = false; // Set while the controls menu is capturing a rebind; blocks the command from firing until the physical key is released again
	std::string m_inputName;      // Input name for the game input system (e.g., "INPUT_CUSTOM1")
	std::string m_command;        // Command name for scripts
	std::string m_tag;            // Script name
	std::string m_description;    // Description for UI
	uint32_t m_hash;              // HashBinding(inputName) hash of the INPUT name
	
	// Bind entries for each mapping (primary and secondary)
	std::array<void*, MAPPING_COUNT> m_bindEntries = { nullptr, nullptr };
	
	// Binded keys for each mapping(including modifiers)
	std::array<std::array<int, 2>, MAPPING_COUNT> m_bindedKeys = {{
		{UNBINDED_KEY, UNBINDED_KEY},
		{UNBINDED_KEY, UNBINDED_KEY}
	}};

public:
	// Getters
	bool IsDummy() const { return m_dummy; }
	bool HasBindedKeys() const
	{
		for (const auto& keys : m_bindedKeys)
		{
			if (keys[0] != UNBINDED_KEY || keys[1] != UNBINDED_KEY)
			{
				return true;
			}
		}
		return false;
	}
	const std::string& GetInputName() const { return m_inputName; }
	const std::string& GetCommand() const { return m_command; }
	const std::string& GetTag() const { return m_tag; }
	const std::string& GetDescription() const { return m_description; }
	uint32_t GetHash() const { return m_hash; }
	void* GetBindEntry(int index) const { return m_bindEntries[index]; }
	const std::array<int, 2>& GetBindedKeys(int mappingIndex) const { return m_bindedKeys[mappingIndex]; }
	
	// Setters
	void SetDummy(bool dummy) { m_dummy = dummy; }
	void SetInputName(const std::string& inputName) { m_inputName = inputName; }
	void SetCommand(const std::string& command) { m_command = command; }
	void SetTag(const std::string& tag) { m_tag = tag; }
	void SetDescription(const std::string& description) { m_description = description; }
	void SetHash(uint32_t hash) { m_hash = hash; }
	void SetBindEntry(int index, void* entry) { m_bindEntries[index] = entry; }
	void SetBindedKeys(int mappingIndex, const std::array<int, 2>& keys) { m_bindedKeys[mappingIndex] = keys; }
	void SetBindedKey(int mappingIndex, int keyIndex, int value) { m_bindedKeys[mappingIndex][keyIndex] = value; }

	void Update(bool suppress);
};

static std::multiset<std::string, std::less<>> g_downSet;

static void DispatchBindingCommand(const std::string& command, bool isDownEvent)
{
	std::string_view commandString = command;

	// '~!' is a UI-only prefix (fivem#2084); strip it for execution.
	if (commandString.rfind("~!", 0) == 0)
	{
		commandString = commandString.substr(2);
	}

	// A binding may hold several ';'-separated commands; button ('+x') commands are handled apart.
	bool hadButtonEvent = false;
	size_t start = 0;

	while (start <= commandString.length())
	{
		size_t semi = commandString.find(';', start);
		size_t end = (semi == std::string_view::npos) ? commandString.length() : semi;
		std::string part(commandString.substr(start, end - start));

		if (!part.empty())
		{
			if (part[0] == '+')
			{
				if (isDownEvent)
				{
					// only submit the '+' (down) command on the first press of this combo
					if (g_downSet.find(part) == g_downSet.end())
					{
						console::GetDefaultContext()->AddToBuffer(part + "\n");
					}
					g_downSet.insert(part);
				}
				else
				{
					if (auto it = g_downSet.find(part); it != g_downSet.end())
					{
						g_downSet.erase(it);
					}

					// only submit the '-' (up) command once the last reference is released
					if (g_downSet.find(part) == g_downSet.end())
					{
						console::GetDefaultContext()->AddToBuffer("-" + part.substr(1) + "\n");
					}
				}

				hadButtonEvent = true;
			}
			else if (isDownEvent || hadButtonEvent)
			{
				// plain commands only run on the down edge
				console::GetDefaultContext()->AddToBuffer(part + "\n");
			}
		}

		if (semi == std::string_view::npos)
		{
			break;
		}

		start = semi + 1;
	}
}

void CustomBinding::Update(bool suppress)
{
	bool down = false;

	for (int mapping = 0; mapping < MAPPING_COUNT && !down; mapping++)
	{
		for (int key : m_bindedKeys[mapping])
		{
			if (key != UNBINDED_KEY && key > 0 && key < 256 && g_gameKeyArray && (g_gameKeyArray[key] & 0x80))
			{
				down = true;
				break;
			}
		}
	}

	if (suppress)
	{
		m_suppressUntilRelease = true;
	}

	if (m_suppressUntilRelease)
	{
		if (!down)
		{
			m_suppressUntilRelease = false; // key released: safe to resume normal edge detection
		}

		down = false; // treat as up while latched (also releases any held '+' command via the up edge)
	}

	bool isDownEvent = (!m_wasDown && down);
	bool isUpEvent = (m_wasDown && !down);
	m_wasDown = down;

	if (isDownEvent || isUpEvent)
	{
		DispatchBindingCommand(m_command, isDownEvent);
	}
}

static std::unordered_map<uint32_t, CustomBinding> g_customBindings;
static std::unordered_set<void*> g_customBindEntryPointers;
static std::map<std::string, int, console::IgnoreCaseLess> ioParameterMap;
static std::recursive_mutex g_customBindingsMutex;

static std::map<std::string, std::array<int, MAPPING_COUNT>> g_persistedKeys;

static uintptr_t* BindEntryVTable = nullptr;
static uint32_t* pendingBindId = nullptr;
static bool BindingInitiated = false;
static void* InitializeCategoryInputsCallAddr = nullptr;

static void EnsureInputMappings();

static std::array<int, 2> GetBindedKey(uint32_t hash, int mappingIndex)
{
	std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
	
	auto it = g_customBindings.find(hash);
	if (it != g_customBindings.end())
	{
		return it->second.GetBindedKeys(mappingIndex);
	}
	return { UNBINDED_KEY, UNBINDED_KEY };
}

static void UpdateBindedKey(uint32_t hash, int key, int mappingIndex)
{
	std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
	
	auto it = g_customBindings.find(hash);
	if (it != g_customBindings.end())
	{
		it->second.SetBindedKey(mappingIndex, 0, key);
		it->second.SetBindedKey(mappingIndex, 1, UNBINDED_KEY);

		auto& persisted = g_persistedKeys[it->second.GetCommand()];
		for (int m = 0; m < MAPPING_COUNT; m++)
		{
			persisted[m] = it->second.GetBindedKeys(m)[0];
		}
		console::GetDefaultContext()->SetVariableModifiedFlags(ConVar_Archive);
	}
}

static uint32_t HashBinding(const std::string& key)
{
	return HashString(key.c_str()) | 0x80000000;
}

static void* g_inputTypeData = nullptr;

static void (*g_origGetMappingCategories)(atArray<unsigned int>& outCategories);
static void GetMappingCategories(atArray<unsigned int>& outCategories)
{
	g_origGetMappingCategories(outCategories);

	outCategories.Set(outCategories.GetCount(), 0);

	for (int i = outCategories.GetCount() - 1; i > 0; i--)
	{
		outCategories[i] = outCategories[i - 1];
	}

	outCategories[0] = HashString("PM_PANE_CFX");
}

static void* (*g_origGetCategoryInputs)(uint32_t* categoryHash, atArray<uint32_t>& controlIds);
static void* GetCategoryInputs(uint32_t* categoryHash, atArray<uint32_t>& controlIds)
{
	bool includeDummy = (_ReturnAddress() == InitializeCategoryInputsCallAddr);
	
	auto result = g_origGetCategoryInputs(categoryHash, controlIds);
	if (*categoryHash == HashString("PM_PANE_CFX"))
	{
		std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
		for (const auto& [hash, binding] : g_customBindings)
		{
			// Only show bindings that are actually in use, but during page initialization
			// include every slot so the game builds a 472-byte descriptor for each hash up-front
			if (!binding.IsDummy() || includeDummy)
			{
				controlIds.Set(controlIds.GetCount(), hash);
			}
		}
	}
	return result;
}

static hook::cdecl_stub<char*(void*, uint32_t)> getNameFromValueUnsafe([]()
{
	return hook::get_pattern("4C 8B 41 ? 33 C0 4C 8B CA");
});

static hook::cdecl_stub<void*()> ctorBindEntry([]()
{
	return hook::get_pattern("48 83 EC ? B9 ? ? ? ? E8 ? ? ? ? 45 33 C9 48 8B D0 48 85 C0 74 ? 65 48 8B 0C 25 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 02 48 8D 05");
});

static hook::cdecl_stub<bool()> _isPauseMenuActive([]()
{
	return hook::get_pattern("48 83 EC ? E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B C8 33 C0");
});

static hook::cdecl_stub<void*(char)> getMainFrontendControl([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 41 B8 ? ? ? ? 48 8D 54 24 ? 48 8B C8 E8 ? ? ? ? F3 0F 10 35 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 4C 8D 05 ? ? ? ? 0F 28 CE 48 8B C8 E8 ? ? ? ? 84 C0 75"));
});

static char* GetNameFromInput(void* data, uint32_t index)
{
	if (index & 0x80000000)
	{
		std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
		thread_local char buf[64];
		
		auto it = g_customBindings.find(index);
		if (it != g_customBindings.end())
		{
			strcpy_s(buf, it->second.GetInputName().c_str());
			return buf;
		}
		
		buf[0] = '\0';
		return buf;
	}

	return getNameFromValueUnsafe(data, index);
}

static const char* (*g_origGetActionName)(uint32_t controlId);
static const char* GetActionName(uint32_t controlId)
{
	return GetNameFromInput(g_inputTypeData, controlId);
}

static uint32_t (*g_origGetInputByName)(char*);
static uint32_t GetInputByName(char* name)
{
	auto control = g_origGetInputByName(name);
	
	if(control <= 0)
	{
		std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
		for (const auto& [hash, binding] : g_customBindings)
		{
			if (_stricmp(name, binding.GetInputName().c_str()) == 0)
			{
				control = hash;
				break;
			}
		}
	}

	return control;
}

static void* CreateBindEntry(uint32_t hash, bool isFirst)
{
	auto bind = ctorBindEntry();
	auto* bindStruct = reinterpret_cast<hook::FlexStruct*>(bind);

	// Set vtable
	bindStruct->Set<uintptr_t*>(0x0, BindEntryVTable);

	// Pin the ref-count (at +0x8) high. The menu borrows the entry while drawing input icons
	// (FindBindEntryByInputId/BuildInputIcon do a matching inc/dec and release it via vtable[0]
	// when it reaches 0). Our entries are mod-owned and permanent, so keep the count away from 0
	// and the game can never invoke the destructor on them.
	bindStruct->Set<int>(0x8, 0x40000000);

	// Set bind entry fields
	bindStruct->Set<bool>(0x11, isFirst);
	bindStruct->Set<uint32_t>(0x2C, hash);

	return bind;
}

static void* GetBindEntry(uint32_t hash, int mappingIndex)
{
	std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
	
	if (mappingIndex > MAPPING_COUNT - 1)
	{
		return nullptr;
	}

	auto it = g_customBindings.find(hash);
	if (it == g_customBindings.end())
	{
		// Binding not found, should not happen
		return nullptr;
	}

	auto& binding = it->second;
	
	// Create bind entries if they don't exist yet
	if (binding.GetBindEntry(0) == nullptr)
	{
		binding.SetBindEntry(0, CreateBindEntry(hash, true));
		binding.SetBindEntry(1, CreateBindEntry(hash, false));
		
		// Add to fast lookup set
		g_customBindEntryPointers.insert(binding.GetBindEntry(0));
		g_customBindEntryPointers.insert(binding.GetBindEntry(1));
	}

	return binding.GetBindEntry(mappingIndex);
}

static bool IsCustomBindEntry(void* bindEntry)
{
	std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
	return g_customBindEntryPointers.find(bindEntry) != g_customBindEntryPointers.end();
}

static void* (*g_origFindDeviceForInput)(void* thisPtr, void** outDevice, uint32_t inputId, uint8_t inputType, uint16_t deviceIndex, bool isAnalog);
static void* FindDeviceForInput(void* thisPtr, void** outDevice, uint32_t inputId, uint8_t inputType, uint16_t deviceIndex, bool isAnalog)
{
	if (inputId & 0x80000000)
	{
		std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
		*outDevice = GetBindEntry(inputId, deviceIndex);
		return outDevice;
	}
	
	return g_origFindDeviceForInput(thisPtr, outDevice, inputId, inputType, deviceIndex, isAnalog);
}

struct PatternPair
{
	std::string_view pattern;
	int offset;
};

static void RegisterBinding(const std::string& tag, const std::string& command, const std::string& description, bool isDummy = false)
{
	uint32_t hash = HashBinding(command);
	
	CustomBinding binding;
	binding.SetDummy(isDummy);
	binding.SetInputName(command);  // For dummy slots, inputName and command are the same
	binding.SetCommand(command);
	binding.SetTag(tag);
	binding.SetDescription(description);
	binding.SetHash(hash);
	
	std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
	g_customBindings[hash] = std::move(binding);
	game::AddCustomText("KMS_" + command, description);
}

static bool RegisterDynamicBinding(const std::string& tag, const std::string& command, const std::string& description, const std::string& ioParamName)
{
	std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);

	// Resolve key names now in case a resource registered before the controls page was ever opened
	// (which is otherwise what triggers ioParameterMap population).
	EnsureInputMappings();

	// Find first available dummy slot
	for (auto it = g_customBindings.begin(); it != g_customBindings.end(); ++it)
	{
		auto& [oldHash, binding] = *it;
		
		if (binding.IsDummy())
		{
			// Found a free slot - keep the original inputName (e.g., INPUT_CUSTOM1)
			std::string originalInputName = binding.GetInputName();
			uint32_t originalHash = binding.GetHash();
			
			// Update the binding in place
			binding.SetDummy(false);
			binding.SetCommand(command);  // New command name (e.g., "dani")
			// binding.inputName stays the same (e.g., "INPUT_CUSTOM1")
			// binding.hash stays the same (hash of INPUT_CUSTOM1)

			// Apply the requested default key (3rd arg) if it maps to a known IO parameter
			int ioParam = UNBINDED_KEY;
			auto ioIt = ioParameterMap.find(ioParamName);
			if (ioIt != ioParameterMap.end())
			{
				ioParam = ioIt->second;
			}

			binding.SetBindedKeys(0, { ioParam, UNBINDED_KEY });
			binding.SetBindedKeys(1, { UNBINDED_KEY, UNBINDED_KEY });

			// Override the default with the user's persisted rebind for this command, if any.
			auto persistedIt = g_persistedKeys.find(command);
			if (persistedIt != g_persistedKeys.end())
			{
				for (int m = 0; m < MAPPING_COUNT; m++)
				{
					binding.SetBindedKey(m, 0, persistedIt->second[m]);
					binding.SetBindedKey(m, 1, UNBINDED_KEY);
				}
			}

			binding.SetTag(tag);
			binding.SetDescription(description);
			
			game::AddCustomText("KMS_" + originalInputName, description);
			return true;
		}
	}
	
	return false;
}

static bool UnregisterDynamicBinding(const std::string& command)
{
	std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
	
	// Find the binding by command name (not hash, since we're searching by command)
	for (auto& [hash, binding] : g_customBindings)
	{
		if (!binding.IsDummy() && binding.GetCommand() == command)
		{
			// Reset to dummy state
			binding.SetDummy(true);
			binding.SetCommand(binding.GetInputName());  // Reset command back to inputName
			binding.SetTag("redm_custom_input");
			binding.SetDescription("Available slot");
			
			// Reset binded keys
			binding.SetBindedKeys(0, {UNBINDED_KEY, UNBINDED_KEY});
			binding.SetBindedKeys(1, {UNBINDED_KEY, UNBINDED_KEY});
			
			return true;
		}
	}
	
	return false;
}

// Release every slot registered under a resource tag; used when that resource stops.
static void UnregisterBindingsForTag(const std::string& tag)
{
	std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);

	for (auto& [hash, binding] : g_customBindings)
	{
		if (!binding.IsDummy() && binding.GetTag() == tag)
		{
			binding.SetDummy(true);
			binding.SetCommand(binding.GetInputName());
			binding.SetTag("redm_custom_input");
			binding.SetDescription("Available slot");
			binding.SetBindedKeys(0, { UNBINDED_KEY, UNBINDED_KEY });
			binding.SetBindedKeys(1, { UNBINDED_KEY, UNBINDED_KEY });
		}
	}
}

static bool g_hideResourceNames = false;

namespace game
{
	void RegisterBindingForTag(const std::string& tag, const std::string& command, const std::string& languageDesc, const std::string& ioms, const std::string& ioParam)
	{
		{
			std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);

			for (auto& [hash, binding] : g_customBindings)
			{
				if (!binding.IsDummy() && binding.GetCommand() == command)
				{
					// already registered (e.g. the resource re-ran its script) - don't burn a slot
					return;
				}
			}
		}

		RegisterDynamicBinding(tag, command, languageDesc, ioParam);
	}

	void SetBindingTagActive(const std::string& tag, bool active)
	{
		if (!active)
		{
			UnregisterBindingsForTag(tag);
		}
	}

	void SetKeyMappingHideResources(bool hide)
	{
		g_hideResourceNames = hide;
	}
}

static void* (*g_origGetMappedInputs)(void* a1, void** a2, uint16_t a3, uint16_t a4);
static void* GetMappedInputs(void* a1, void** a2, uint16_t a3, uint16_t a4)
{
	auto bind = (hook::FlexStruct*)*a2;
	auto isFirst = bind->At<bool>(0x11);
	auto inputId = bind->At<uint32_t>(0x2C);

	if(IsCustomBindEntry(bind))
	{
		auto* data = reinterpret_cast<int*>(a1);
		auto keys = GetBindedKey(inputId, isFirst ? 0 : 1);
		
		data[0] = keys[0];
		data[1] = keys[1];
		data[2] = 0x3; // Unk

		return data;
	}
	
	return g_origGetMappedInputs(a1, a2, a3, a4);
}

static int64_t (__fastcall *g_origGetBindingCount)(hook::FlexStruct* self);
static int64_t __fastcall GetBindingCount(hook::FlexStruct* self)
{
	auto result = g_origGetBindingCount(self);
	
	if (IsCustomBindEntry(self))
	{
		result = 1;
	}
	
	return result;
}

static int (*g_origGetMappingCount)(void* self);
static int GetMappingCount(void* self)
{
	auto result = g_origGetMappingCount(self);

	if (IsCustomBindEntry(self))
	{
		result = 1;
	}
	
	return result;
}

static int64_t (*g_origFindRebindConflicts)(void* self, uint64_t* out, uint32_t inputId, int64_t a4, char a5);
static int64_t FindRebindConflicts(void* self, uint64_t* out, uint32_t inputId, int64_t a4, char a5)
{
	if (inputId & 0x80000000)
	{
		return 0;
	}

	return g_origFindRebindConflicts(self, out, inputId, a4, a5);
}

static bool (*g_origUpdateBindInput)(hook::FlexStruct* self, int keyId, uint16_t mappingIndex);
static bool UpdateBindInput(hook::FlexStruct* self, int keyId, uint16_t mappingIndex)
{
	auto result = g_origUpdateBindInput(self, keyId, mappingIndex);
	
	auto isFirst = self->At<bool>(0x11);
	auto inputId = self->At<uint32_t>(0x2C);

	if (inputId & 0x80000000)
	{
		UpdateBindedKey(inputId, keyId, isFirst ? 0 : 1);
	}
	
	return result;
}

static void* (*g_origProcessBindingRequestStep)(hook::FlexStruct* self, uint32_t checkingInputId);
static void* ProcessBindingRequestStep(hook::FlexStruct* self, uint32_t checkingInputId)
{
	if (*pendingBindId & 0x80000000)
	{
		return 0;
	}
	
	return g_origProcessBindingRequestStep(self, checkingInputId);
}

static int GetBindedKey_Vtable(void* self, uint16_t mappingIndex)
{
	auto* bind = reinterpret_cast<hook::FlexStruct*>(self);
	auto inputId = bind->At<uint32_t>(0x2C);
	auto isFirst = bind->At<bool>(0x11);

	if (IsCustomBindEntry(self))
	{
		auto keys = GetBindedKey(inputId, isFirst ? 0 : 1);
		return keys[mappingIndex];
	}

	// Check if bind entry has valid data
	if (bind->At<uint32_t>(0x18) != 0 && mappingIndex < bind->At<uint16_t>(0x28))
	{
		uint32_t* array = bind->At<uint32_t*>(0x20);
		return array[mappingIndex];
	}

	return UNBINDED_KEY;
}

static bool InitializeInputMappings()
{
	auto structureField = rage::GetStructureDefinition("rage__InputCalibration__Data");

	// This parser structure isn't registered until the game core has initialized. If a resource
	// calls RegisterKeyMapping before that, bail out so we retry on a later call (no crash).
	if (!structureField)
	{
		return false;
	}

	auto enumeration = structureField->m_members[0]->m_definition->enumData;
	auto name = enumeration->names;
	auto field = enumeration->fields;

	// The data is interleaved: [hash0, value0, hash1, value1, ...]
	while (name && *name != nullptr)
	{
		field++; // skip the key hash; the next field's "hash" is actually the key value
		uint32_t keyValue = field->hash;

		std::string_view thisName = *name;
		if (thisName.find("KEY_") == 0)
		{
			thisName = thisName.substr(thisName.find_first_of('_') + 1);
		}

		ioParameterMap[std::string(thisName)] = keyValue;

		name++;
		field++;
	}

	return true;
}

static void EnsureInputMappings()
{
	std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);

	if (BindingInitiated)
	{
		return;
	}

	if (InitializeInputMappings())
	{
		BindingInitiated = true;
	}
}

static void (*g_origInitializeCategoryInputs)(void* self, void** a2);
static void InitializeCategoryInputs(void* self, void** a2)
{
	g_origInitializeCategoryInputs(self, a2);

	EnsureInputMappings();
}

static void ProcessCustomBindings()
{
	std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
	
	bool suppress = _isPauseMenuActive() || (pendingBindId && (*pendingBindId != 0));
	
	// Iterate through all bindings and update non-dummy ones
	for (auto& [hash, binding] : g_customBindings)
	{
		if (!binding.IsDummy() && (binding.HasBindedKeys() || suppress))
		{
			binding.Update(suppress);
		}
	}
}

static void (*g_origIoMapperUpdate)(void* self, uint32_t time, bool forceKeyboardMouse, float formal);
static void IoMapperUpdate(void* self, uint32_t time, bool forceKeyboardMouse, float formal)
{
	ProcessCustomBindings();
	g_origIoMapperUpdate(self, time, forceKeyboardMouse, formal);
}

static HookFunction hookFunction([]()
{
	g_origIoMapperUpdate = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 0F 57 D2 48 8D 05 ? ? ? ? F3 0F 6F 88")), IoMapperUpdate);

	// Resolve the game's raw VK key-state array so CustomBinding::Update can detect presses on our custom bindings.
	{
		auto location = hook::get_pattern<char>("48 3B 05 ? ? ? ? 48 8D 35", 0xA);
		g_gameKeyArray = location + *(int32_t*)location + 4;
	}
	
	// Call to handle the initialize of binding system
	g_origInitializeCategoryInputs = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 54 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC ? 48 8B DA 4C 8B E9"), InitializeCategoryInputs);

	// The address of the return addr of GetCategoryInputs when initializing inputs
	InitializeCategoryInputsCallAddr = hook::get_pattern("E8 ? ? ? ? 48 8B 75 ? 44 8B F7", 0x5);
	
	// Function of UI to get the mapped inputs for a bind entry
	g_origGetMappedInputs = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? B8"), GetMappedInputs);
	
	BindEntryVTable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 03 4C 89 63 ? 44 89 63 ? EB ? 49 8B DC 49 8B D6", 0x3));
	// Override the GetBindedKey function in the BindEntry Vtable
	hook::put<uintptr_t>(&BindEntryVTable[14], (uintptr_t)GetBindedKey_Vtable);

	// Hook binding and mapping count getter functions to return 1 for custom bindings
	{
		g_origGetBindingCount = hook::trampoline(hook::get_pattern("48 83 ec 28 48 8b 49 18 33 c0 48 85 c9 74 06 48 8b 01 ff 50 38 48 83 c4 28"), GetBindingCount);
		g_origGetMappingCount = hook::trampoline(hook::get_pattern("48 83 ec 28 48 8b 49 18 33 c0 48 85 c9 74 06 48 8b 01 ff 50 68 48 83 c4 28 c3"), GetMappingCount);
	}

	// Skip rebind conflicts for custom bindings
	{
		g_origFindRebindConflicts = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B DA 41 8B F8"), FindRebindConflicts);
		g_origProcessBindingRequestStep = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 44 8A 89"), ProcessBindingRequestStep);
	}

	// Update the binded key when rebinding
	g_origUpdateBindInput = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 33 C0 48 8B 49"), UpdateBindInput);

	// Global bind entry that stores the bind that is being modified
	pendingBindId = hook::get_address<uint32_t*>(hook::get_pattern("8B 05 ? ? ? ? 45 33 FF 85 C0", 0x2));

	// Function that retrieve the BindEntry for an input, we create BindEntries for our custom bindings
	g_origFindDeviceForInput = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B 5C 24 ? 48 85 DB 74 ? 8B 05")), FindDeviceForInput);

	// Restores a persisted user rebind from redm.cfg
	static auto setKeyCmd = std::make_unique<ConsoleCommand>("bind_setkey", [](const std::string& command, const std::string& mappingStr, const std::string& keyStr)
	{
		int mapping = atoi(mappingStr.c_str());
		int keyValue = atoi(keyStr.c_str());

		if (mapping < 0 || mapping >= MAPPING_COUNT)
		{
			return;
		}

		std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);

		auto emplaced = g_persistedKeys.emplace(command, std::array<int, MAPPING_COUNT>{});
		if (emplaced.second)
		{
			emplaced.first->second.fill(UNBINDED_KEY);
		}
		emplaced.first->second[mapping] = keyValue;

		for (auto& [hash, binding] : g_customBindings)
		{
			if (!binding.IsDummy() && binding.GetCommand() == command)
			{
				binding.SetBindedKey(mapping, 0, keyValue);
				binding.SetBindedKey(mapping, 1, UNBINDED_KEY);
			}
		}
	});

	// Persist user rebinds into redm.cfg
	console::GetDefaultContext()->OnSaveConfiguration.Connect([](const std::function<void(const std::string&)>& writeLine)
	{
		auto escapeArg = [](const std::string& s)
		{
			std::string out;
			for (char c : s)
			{
				if (c == '\\' || c == '"')
				{
					out += '\\';
				}
				out += c;
			}
			return out;
		};

		std::lock_guard<std::recursive_mutex> lock(g_customBindingsMutex);
		for (const auto& [command, keys] : g_persistedKeys)
		{
			for (int m = 0; m < MAPPING_COUNT; m++)
			{
				if (keys[m] != UNBINDED_KEY)
				{
					writeLine(fmt::sprintf("bind_setkey \"%s\" %d %d", escapeArg(command), m, keys[m]));
				}
			}
		}
	});

	// Initialize all dummy slots
	for (int i = 0; i < MAX_CUSTOM_BINDINGS; i++)
	{
		RegisterBinding("redm_custom_input", fmt::sprintf("INPUT_CUSTOM%d", i + 1), fmt::sprintf("RedM Custom Input %d", i + 1), true);
	}
	
	// Hook functions to retrieve the name from input id
	{
		std::initializer_list<PatternPair> patterns = {
			{ "E8 ? ? ? ? 48 8B D0 B9 ? ? ? ? 48 8B F8 E8 ? ? ? ? 4C 8B C7 8B D0 B9 ? ? ? ? 8B D8 E8 ? ? ? ? 48 8B 4D", 0x0 },
			{ "48 8D 15 ? ? ? ? 48 8D 4D ? E8 ? ? ? ? 48 8D 55 ? 33 C9 E8 ? ? ? ? 39 43", -0x18 },
			{ "E8 ? ? ? ? 4C 8D 0D ? ? ? ? 4C 8B C0 48 8D 15 ? ? ? ? 48 8D 4D", 0x0 },
			{ "E8 ? ? ? ? 33 D2 48 8B C8 E8 ? ? ? ? 48 8B 4F", 0x0 },
			{ "E8 ? ? ? ? 33 D2 48 89 45 ? 48 85 C0 74 ? 45 84 E4", 0x0 },
			{ "E8 ? ? ? ? 48 8B D0 B9 ? ? ? ? 48 8B F8 E8 ? ? ? ? 4C 8B C7 8B D0 B9 ? ? ? ? 8B D8 E8 ? ? ? ? 48 8D 54 24 ? 89 5C 24 ? 49 8B CF", 0x0 }
		};

		for (const auto& patternPair : patterns)
		{
			hook::call(hook::get_pattern(patternPair.pattern, patternPair.offset), GetNameFromInput);
		}
		
		g_origGetActionName = hook::trampoline(hook::get_pattern("48 63 D1 48 8D 0D ? ? ? ? E9"), GetActionName);
		g_inputTypeData = hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B D0 B9 ? ? ? ? 48 8B F8 E8 ? ? ? ? 4C 8B C7 8B D0 B9 ? ? ? ? 8B D8 E8 ? ? ? ? 48 8B 4D", 0x3));
		g_origGetInputByName = hook::trampoline(hook::get_pattern("48 83 EC ? 48 8B D1 45 33 C9"), GetInputByName);
	}

	// Mapping categories
	{
		game::AddCustomText("PM_PANE_CFX", "RedM");
		g_origGetMappingCategories = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B 7C 24 ? 45 8B F4")), GetMappingCategories);
	}
	
	g_origGetCategoryInputs = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B 5D ? 41 8B F5")), GetCategoryInputs);

	// Allow duplicated
	hook::nop(hook::get_pattern("E8 ? ? ? ? 48 8D 4D ? BA ? ? ? ? 48 2B CB"), 5);
	hook::nop(hook::get_pattern("74 ? 41 8D 57 ? 44 89 7C 24"), 0x2);

	// UpdateBindingState
	// Add a stub to return the custom binding name if the 8 bit is set
	{
		auto location = hook::get_pattern("4C 8B F8 48 85 C0 0F 84 ? ? ? ? 4C 8B C0 C6 45");

		static struct : jitasm::Frontend
		{
			uintptr_t originalLocation;
			uintptr_t customLocation;

			void Init(uintptr_t original)
			{
				originalLocation = original;
			}

			virtual void InternalMain() override
			{
				mov(r15, rax);
				test(edi, 0x80000000);
				jz("NotCustom");

				mov(rcx, 0);
				movsxd(rdx, edi);
				mov(rax, (uintptr_t)GetNameFromInput);
				call(rax);
				mov(r15, rax);

				L("NotCustom");
				test(rax, rax);
				mov(rcx, originalLocation);
				jmp(rcx);
			}
		} stub;
		
		hook::nop(location, 0x6);
		stub.Init((uintptr_t)location + 0x6);
		hook::jump_rcx(location, stub.GetCode());
	}

	// UpdateBindingState
	// Remove the bitmask use for custom bindings, this bitmask determines if a input is already registered, valid, etc.
	{
		auto location = hook::get_pattern("0F B6 C8 48 8B C7 48 C1 E8");
		auto skipBitmask = hook::get_pattern("48 8b 4c 24 38 83 cf ff 48 85 c9");
		
		static struct : jitasm::Frontend
		{
			uintptr_t originalLocation;
			uintptr_t customLocation;

			void Init(uintptr_t original, uintptr_t custom)
			{
				originalLocation = original;
				customLocation = custom;
			}

			virtual void InternalMain() override
			{
				movzx(ecx, al);
				mov(rax, rdi);
				
				test(edi, 0x80000000);
				jz("NotCustom");
				mov(rbx, customLocation);
				jmp(rbx);

				L("NotCustom");
				mov(rbx, originalLocation);
				jmp(rbx);
			}
		} stub;
		
		hook::nop(location, 0x6);
		stub.Init((uintptr_t)location + 0x6, (uintptr_t)skipBitmask);
		hook::jump_rcx(location, stub.GetCode());
	}
});
