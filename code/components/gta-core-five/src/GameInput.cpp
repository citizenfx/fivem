#include <StdInc.h>
#include <Hooking.h>

#include <CoreConsole.h>

#include <RageParser.h>

#include <gameSkeleton.h>
#include <ICoreGameInit.h>

#include <atArray.h>
#include <MinHook.h>

#include <CustomText.h>

#include <concurrent_queue.h>

#include <array>
#include <unordered_set>

#include <CrossBuildRuntime.h>

#include <GameInput.h>

namespace rage
{
	struct ioInputSource
	{
		uint32_t source;
		uint32_t parameter;
		int unk;

		inline ioInputSource()
			: source(0), parameter(0), unk(-1)
		{

		}

		inline ioInputSource(uint32_t source, uint32_t parameter, int unk)
			: source(source), parameter(parameter), unk(unk)
		{

		}
	};

	class ioValue
	{
	public:
		using ReadOptions = uint64_t;

		ioValue()
		{
			memset(pad, 0, sizeof(pad));

			*(float*)(&pad[0]) = 1.0f;
			*(uint64_t*)(&pad[16]) = -1;
			*(int32_t*)(&pad[24]) = -3;
			*(uint64_t*)(&pad[28]) = -1;
			*(int32_t*)(&pad[36]) = -3;
			*(int32_t*)(&pad[40]) = -1;
			*(int32_t*)(&pad[44]) = -1;
			*(int32_t*)(&pad[48]) = -3;
			*(int32_t*)(&pad[52]) = -1;
			*(int8_t*)(&pad[58]) = 1;
		}

		void Update(uint8_t preFrameCounter, uint32_t time);

		void SetCurrentValue(float value, rage::ioInputSource& source);

		bool IsDown(float threshold, const ReadOptions& readOptions);

		static ReadOptions NO_DEAD_ZONE;

	private:
		char pad[9 * 8];
	};

	ioValue::ReadOptions ioValue::NO_DEAD_ZONE = 0x100000000;

	class ioMapper
	{
	public:
		uint32_t Map(uint32_t mapperSource, uint32_t key, ioValue& value, int a4);

		void UpdateMap(int max_k, const ioInputSource& info, ioValue& value);

		void RemoveDeviceMappings(ioValue& value, int a3);
	};
}

static hook::cdecl_stub<void(rage::ioValue* self, float value, rage::ioInputSource& source)> _ioValue_SetCurrentValue([]()
{
	return hook::get_pattern("8A 41 3A 4C 8B C9 A8 01");
});

void rage::ioValue::SetCurrentValue(float value, rage::ioInputSource& source)
{
	_ioValue_SetCurrentValue(this, value, source);
}

static hook::cdecl_stub<bool(rage::ioValue* self, float value, const rage::ioValue::ReadOptions&)> _ioValue_IsDown([]()
{
	return hook::get_pattern("0F 29 74 24 20 48 8B F9 48 8B D3 0F 28 F1 E8", -0xD);
});

bool rage::ioValue::IsDown(float threshold, const ReadOptions& readOptions)
{
	return _ioValue_IsDown(this, threshold, readOptions);
}

static hook::cdecl_stub<uint32_t(rage::ioMapper*, uint32_t, uint32_t, rage::ioValue&, int)> _ioMapper_Map([]()
{
	return hook::get_pattern("89 54 81 24 48 63", -0x30);
});

uint32_t rage::ioMapper::Map(uint32_t mapperSource, uint32_t key, ioValue& value, int a4)
{
	return _ioMapper_Map(this, mapperSource, key, value, a4);
}

static hook::cdecl_stub<void(rage::ioMapper*, int, const rage::ioInputSource&, rage::ioValue&)> _ioMapper_UpdateMap([]()
{
	return hook::get_pattern("33 C0 4D  8B D0 44 8B D8 8B D8 48", -0x0D);
});

void rage::ioMapper::UpdateMap(int max_k, const rage::ioInputSource& info, rage::ioValue& value)
{
	return _ioMapper_UpdateMap(this, max_k, info, value);
}

static hook::cdecl_stub<void(rage::ioMapper*, rage::ioValue&, int)> _ioMapper_RemoveDeviceMappings([]()
{
	return hook::get_pattern("45 33 E4 45 33 ED 89 94 24 80 00 00 00 39 11", -0x34);
});

void rage::ioMapper::RemoveDeviceMappings(rage::ioValue& value, int a3)
{
	return _ioMapper_RemoveDeviceMappings(this, value, a3);
}

static hook::cdecl_stub<void(rage::ioValue*, uint8_t, uint32_t)> _ioValue_Update([]()
{
	return hook::get_pattern("41 22 C3 74 06 F6 41 3A 02 74 6F", -0x18);
});

void rage::ioValue::Update(uint8_t preFrameCounter, uint32_t time)
{
	_ioValue_Update(this, preFrameCounter, time);
}

static hook::cdecl_stub<uint32_t(uint32_t)> _controlSourceToMapperSource([]()
{
	return hook::get_pattern("85 C9 78 20 83 F9 04");
});

uint32_t ControlSourceToMapperSource(uint32_t source)
{
	return _controlSourceToMapperSource(source);
}

static hook::cdecl_stub<uint32_t(uint32_t, uint32_t)> _getParameterIndex([]()
{
	return hook::get_pattern("83 F9 01 74 2C 0F 8E E8 00 00 00", -0xF);
});

//
// Given a source/parameter, gets the parameter to use when serializing a control.
//
uint32_t GetParameterIndex(uint32_t source, uint32_t parameter)
{
	return _getParameterIndex(source, parameter);
}

static hook::cdecl_stub<uint32_t(uint32_t, uint32_t)> _ungetParameterIndex([]()
{
	return hook::get_pattern("74 28 7E 36 83 F9", -0x12);
});

//
// Given a source/serialized parameter, gets the native parameter to use.
//
uint32_t UngetParameterIndex(uint32_t source, uint32_t parameter)
{
	return _ungetParameterIndex(source, parameter);
}

class Button
{
public:
	Button(const std::string& name);

	void UpdateIoValues(float value, rage::ioInputSource& source);

	void UpdateOnControl();

	void SetFromControl(void* control, int keyIndex);

private:
	std::string m_name;

	std::unique_ptr<ConsoleCommand> m_downCommand;
	std::unique_ptr<ConsoleCommand> m_upCommand;

	rage::ioValue m_ioValue;

	std::array<rage::ioValue*, 4> m_ioValueCopies;
};

Button::Button(const std::string& name)
	: m_name(name)
{
	for (auto& copy : m_ioValueCopies)
	{
		copy = nullptr;
	}

	m_downCommand = std::make_unique<ConsoleCommand>("+" + name, [this]()
	{
		rage::ioInputSource source{ 0, 1, -4 }; // keyboard?
		UpdateIoValues(1.0f, source);
	});

	m_upCommand = std::make_unique<ConsoleCommand>("-" + name, [this]()
	{
		rage::ioInputSource source{ 0, 1, -4 }; // keyboard?
		UpdateIoValues(0.0f, source);
	});
}

void Button::UpdateIoValues(float value, rage::ioInputSource& source)
{
	m_ioValue.SetCurrentValue(value, source);
}

void Button::UpdateOnControl()
{
	if (m_ioValue.IsDown(0.5f, rage::ioValue::NO_DEAD_ZONE))
	{
		for (auto copy : m_ioValueCopies)
		{
			if (copy)
			{
				rage::ioInputSource source{ 0, 1, -4 }; // keyboard?

				if (!copy->IsDown(0.5f, rage::ioValue::NO_DEAD_ZONE))
				{
					copy->SetCurrentValue(1.0f, source);
				}
			}
		}
	}
}

void Button::SetFromControl(void* control, int keyIndex)
{
	auto controlPtr = (uint8_t*)control;

	rage::ioValue* value = (rage::ioValue*) & controlPtr[33704 + (keyIndex * 72)];

	for (auto& copy : m_ioValueCopies)
	{
		if (copy == nullptr)
		{
			copy = value;
			break;
		}
	}
}

class Binding
{
public:
	Binding(const std::string& command);

	~Binding();

	void Update(rage::ioMapper* mapper);

	void UpdateMap(rage::ioMapper* mapper)
	{
		rage::ioInputSource src;
		GetBinding(src);
		mapper->UpdateMap(0, src, m_value);

		m_mappers.insert(mapper);
	} 

	rage::ioValue& GetValue()
	{
		return m_value;
	}

	inline void SetBinding(std::tuple<int, int> binding)
	{
		m_binding = binding;
	}

	inline const auto& GetCommand() const
	{
		return m_command;
	}

	inline const auto& GetTag() const
	{
		return m_tag;
	}

	inline void SetTag(const std::string& tag)
	{
		m_tag = tag;
	}

	inline void GetBinding(rage::ioInputSource& source) const
	{
		source.source = std::get<0>(m_binding);
		source.parameter = std::get<1>(m_binding);
		source.unk = -1;
	}

private:
	rage::ioValue m_value;

	bool m_wasDown;

	std::string m_command;

	std::string m_tag;

	std::tuple<int, int> m_binding;

	std::set<rage::ioMapper*> m_mappers;
};

Binding::Binding(const std::string& command)
	: m_command(command), m_wasDown(false)
{

}

Binding::~Binding()
{
	for (rage::ioMapper* mapper : m_mappers)
	{
		mapper->RemoveDeviceMappings(m_value, -4);
	}
}

static void* g_control;
static uint32_t g_mapperOffset;

static hook::cdecl_stub<bool(int)> _isMenuActive([]
{
	return hook::get_call(hook::get_pattern("33 C9 E8 ? ? ? ? 84 C0 74 04 84 DB 75 21", 2));
});

static hook::cdecl_stub<bool()> _isTextInputBoxActive([]
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 40 84 FF 74 15 E8", 10));
});

bool IsTagActive(const std::string& tag);

void Binding::Update(rage::ioMapper* mapper)
{
	if (!IsTagActive(m_tag))
	{
		return;
	}

	if (Instance<ICoreGameInit>::Get()->GetGameLoaded() && mapper == (rage::ioMapper*)((char*)g_control + g_mapperOffset) && !_isMenuActive(1))
	{
		UpdateMap(mapper);
	}

	bool down = m_value.IsDown(0.5f, rage::ioValue::NO_DEAD_ZONE);

	// text input should count as up
	if (Instance<ICoreGameInit>::Get()->GetGameLoaded() && _isTextInputBoxActive())
	{
		down = false;
	}

	bool isDownEvent = (!m_wasDown && down);
	bool isUpEvent = (m_wasDown && !down);

	m_wasDown = down;

	if (isDownEvent || isUpEvent)
	{
		const auto& commandString = m_command;

		// split the string by ';' (so button bindings get handled separately)
		int i = 0;
		std::stringstream thisCmd;

		bool hadButtonEvent = false;

		while (true)
		{
			if (i == commandString.length() || commandString[i] == ';')
			{
				std::string thisString = thisCmd.str();
				thisCmd.str("");

				// if this is a button binding
				if (thisString[0] == '+')
				{
					if (isDownEvent)
					{
						// TODO: add key code arguments
						console::GetDefaultContext()->AddToBuffer(thisString + "\n");
					}
					else
					{
						// up event is -[button cmd]
						// TODO: add key code arguments
						console::GetDefaultContext()->AddToBuffer("-" + thisString.substr(1) + "\n");
					}

					hadButtonEvent = true;
				}
				else
				{
					// if not, just execute the command on down
					if (isDownEvent || hadButtonEvent)
					{
						console::GetDefaultContext()->AddToBuffer(thisString + "\n");
					}
				}
			}

			if (i == commandString.length())
			{
				break;
			}

			thisCmd << commandString[i];

			i++;
		}
	}
}

class BindingManager
{
public:
	void Initialize();

	void OnGameInit();

	void CreateButtons();

	void UpdateButtons();

	void Update(rage::ioMapper* mapper, uint32_t time);

	std::shared_ptr<Binding> Bind(int source, int parameter, const std::string& command);

	inline auto& GetBindings()
	{
		return m_bindings;
	}

	inline void QueueOnFrame(const std::function<void()>& func)
	{
		m_queue.push(func);
	}

private:
	std::unique_ptr<ConsoleCommand> m_bindCommand;
	std::unique_ptr<ConsoleCommand> m_rbindCommand;
	std::unique_ptr<ConsoleCommand> m_unbindCommand;
	std::unique_ptr<ConsoleCommand> m_unbindAllCommand;
	std::unique_ptr<ConsoleCommand> m_listBindsCommand;

	std::multimap<std::tuple<int, int>, std::shared_ptr<Binding>> m_bindings;

	std::list<std::unique_ptr<Button>> m_buttons;

	concurrency::concurrent_queue<std::function<void()>> m_queue;
};

static std::map<std::string, int, console::IgnoreCaseLess> ioSourceMap;
static std::map<std::string, int, console::IgnoreCaseLess> ioParameterMap;

void BindingManager::Initialize()
{
	m_listBindsCommand = std::make_unique<ConsoleCommand>("bind", [this]()
	{
		for (auto& binding : m_bindings)
		{
			rage::ioInputSource source;
			binding.second->GetBinding(source);

			auto parameterIndex = _getParameterIndex(source.source, source.parameter);

			std::string ioSource = "";
			std::string ioParameter = "";

			for (auto& entry : ioSourceMap)
			{
				if (entry.second == source.source)
				{
					ioSource = entry.first;
				}
			}

			for (auto& entry : ioParameterMap)
			{
				if (entry.second == parameterIndex)
				{
					ioParameter = entry.first;
				}
			}

			auto sourceString = fmt::sprintf("%s %s", ioSource, ioParameter);

			console::Printf("IO", "%s %s -> %s\n", sourceString, binding.second->GetTag(), binding.second->GetCommand());
		}
	});

	auto bind = [=](const std::string& ioSourceName, const std::string& ioParameterName, const std::string& commandString, const std::string& tag)
	{
		int ioSource;

		{
			auto it = ioSourceMap.find(ioSourceName);

			if (it == ioSourceMap.end())
			{
				console::Printf("IO", "Invalid I/O source %s\n", ioSourceName);
				return;
			}

			ioSource = it->second;
		}

		int ioParameter;

		{
			auto it = ioParameterMap.find(ioParameterName);

			if (it == ioParameterMap.end())
			{
				console::Printf("IO", "Invalid key name %s\n", ioParameterName.c_str());
				return;
			}

			ioParameter = _ungetParameterIndex(ioSource, it->second);
		}

		auto binding = std::make_shared<Binding>(commandString);
		binding->SetBinding({ ioSource, ioParameter });
		binding->SetTag(tag);

		m_bindings.insert({ { ioSource, ioParameter }, binding });

		// TODO: implement when saving is added
		console::GetDefaultContext()->SetVariableModifiedFlags(ConVar_Archive);
	};

	m_bindCommand = std::make_unique<ConsoleCommand>("bind", [=](const std::string& ioSourceName, const std::string& ioParameterName, const std::string& commandString)
	{
		bind(ioSourceName, ioParameterName, commandString, "");
	});

	m_rbindCommand = std::make_unique<ConsoleCommand>("rbind", [=](const std::string& tag, const std::string& ioSourceName, const std::string& ioParameterName, const std::string& commandString)
	{
		bind(ioSourceName, ioParameterName, commandString, tag);
	});

	m_unbindCommand = std::make_unique<ConsoleCommand>("unbind", [=](const std::string& ioSourceName, const std::string& ioParameterName)
	{
		int ioSource;

		{
			auto it = ioSourceMap.find(ioSourceName);

			if (it == ioSourceMap.end())
			{
				console::Printf("IO", "Invalid I/O source %s\n", ioSourceName);
				return;
			}

			ioSource = it->second;
		}

		int ioParameter;

		{
			auto it = ioParameterMap.find(ioParameterName);

			if (it == ioParameterMap.end())
			{
				console::Printf("IO", "Invalid key name %s\n", ioParameterName.c_str());
				return;
			}

			ioParameter = _ungetParameterIndex(ioSource, it->second);
		}

		m_bindings.erase({ ioSource, ioParameter });

		console::GetDefaultContext()->SetVariableModifiedFlags(ConVar_Archive);
	});

	m_unbindAllCommand = std::make_unique<ConsoleCommand>("unbindall", [=]()
	{
		m_bindings.clear();
	});
}

std::shared_ptr<Binding> BindingManager::Bind(int ioSource, int ioParameter, const std::string& commandString)
{
	for (auto& binding : m_bindings)
	{
		if (binding.second->GetCommand() == commandString && IsTagActive(binding.second->GetTag()))
		{
			m_bindings.erase(binding.first);
			break;
		}
	}

	//m_bindings.erase({ ioSource, ioParameter });

	auto binding = std::make_shared<Binding>(commandString);
	binding->SetBinding({ ioSource, ioParameter });

	m_bindings.insert({ { ioSource, ioParameter }, binding });

	console::GetDefaultContext()->SetVariableModifiedFlags(ConVar_Archive);

	return binding;
}

void BindingManager::Update(rage::ioMapper* mapper, uint32_t time)
{
	static uint8_t counter = 0;
	counter++;

	for (auto& bindingPair : m_bindings)
	{
		auto [source, binding] = bindingPair;

		binding->Update(mapper);
	}

	std::function<void()> func;

	while (m_queue.try_pop(func))
	{
		func();
	}
}

void BindingManager::OnGameInit()
{
	{
		auto structureField = rage::GetStructureDefinition("rage__InputCalibration__Data");

		auto enumeration = structureField->m_members[0]->m_definition->enumData;
		auto name = enumeration->names;

		for (auto field = enumeration->fields; field->index != -1 || field->hash != 0; field++)
		{
			std::string_view thisName = *name;

			if (thisName.find("KEY_") == 0)
			{
				thisName = thisName.substr(thisName.find_first_of('_') + 1);
			}

			ioParameterMap[std::string(thisName)] = field->index;

			name++;
		}
	}

	{
		auto structureField = rage::GetStructureDefinition("rage__ControlInput__Mapping");

		auto enumeration = structureField->m_members[1]->m_definition->enumData;
		auto name = enumeration->names;

		for (auto field = enumeration->fields; field->index != -1 || field->hash != 0; field++)
		{
			std::string_view thisName = *name;
			thisName = thisName.substr(thisName.find_first_of('_') + 1);

			ioSourceMap[std::string(thisName)] = field->index;

			name++;
		}
	}
}

void BindingManager::CreateButtons()
{
	{
		auto structureField = rage::GetStructureDefinition("rage__ControlInput__Mapping");

		auto enumeration = structureField->m_members[0]->m_definition->enumData;
		auto name = enumeration->names;

		for (auto field = enumeration->fields; field->index != -1 || field->hash != 0; field++)
		{
			std::string_view thisName = *name;
			thisName = thisName.substr(thisName.find_first_of('_') + 1);

			std::string thisNameStr(thisName);
			std::transform(thisNameStr.begin(), thisNameStr.end(), thisNameStr.begin(), ToLower);

			auto button = std::make_unique<Button>(thisNameStr);
			button->SetFromControl(g_control, field->index);

			if (!Is2060())
			{
				button->SetFromControl((char*)g_control + 0x21A98, field->index); // 1604
			}

			m_buttons.push_back(std::move(button));

			name++;
		}
	}
}

void BindingManager::UpdateButtons()
{
	for (auto& button : m_buttons)
	{
		button->UpdateOnControl();
	}
}

static size_t g_controlSize;
static BindingManager bindingManager;

static std::unordered_set<std::string> g_activeTags;

bool IsTagActive(const std::string& tag)
{
	return (tag.empty() || g_activeTags.find(tag) != g_activeTags.end());
}

static void(*ioMapper_Update)(void*, uint32_t, bool);

static void ioMapper_UpdateStub(rage::ioMapper* mapper, uint32_t time, bool a3)
{
	bindingManager.Update(mapper, time);

	ioMapper_Update(mapper, time, a3);

	bindingManager.UpdateButtons();
}

void ProfileSettingsInit();

static void* (*origGetFunc)();
static void* GetFunc()
{
	bindingManager.OnGameInit();
	ProfileSettingsInit();

	return origGetFunc();
}

static std::map<std::string, std::tuple<std::string, std::string>> g_registeredBindings;

namespace game
{
	void SetBindingTagActive(const std::string& tag, bool active)
	{
		if (active)
		{
			g_activeTags.insert(tag);
		}
		else
		{
			for (auto it = g_registeredBindings.begin(); it != g_registeredBindings.end();)
			{
				if (std::get<0>(it->second) == tag)
				{
					it = g_registeredBindings.erase(it);
				}
				else
				{
					it++;
				}
			}

			g_activeTags.erase(tag);
		}
	}

	void RegisterBindingForTag(const std::string& tag, const std::string& command, const std::string& languageDesc, const std::string& ioms, const std::string& ioParam)
	{
		g_registeredBindings.insert({ command, { tag, languageDesc } });

		if (!ioms.empty() && !ioParam.empty())
		{
			int ioSource;

			{
				auto it = ioSourceMap.find(ioms);

				if (it == ioSourceMap.end())
				{
					console::Printf("IO", "Invalid I/O source %s\n", ioms);
					return;
				}

				ioSource = it->second;
			}

			int ioParameter;

			{
				auto it = ioParameterMap.find(ioParam);

				if (it == ioParameterMap.end())
				{
					console::Printf("IO", "Invalid key name %s\n", ioParam);
					return;
				}

				ioParameter = _ungetParameterIndex(ioSource, it->second);
			}

			for (auto& binding : bindingManager.GetBindings())
			{
				if (binding.second->GetCommand() == command && IsTagActive(binding.second->GetTag()))
				{
					return;
				}
			}

			bindingManager.Bind(ioSource, ioParameter, command)->SetTag(tag);
		}
	}
}

static uint32_t HashBinding(const std::string& key)
{
	return HashString(key.c_str()) | 0x80000000;
}

static auto GetRegisteredBindingByHash(uint32_t hash)
{
	for (auto& b : g_registeredBindings)
	{
		if (HashBinding(b.first) == hash)
		{
			return b;
		}
	}

	return std::pair<const std::string, std::tuple<std::string, std::string>>{};
}

static void(*g_origGetMappingCategories)(atArray<uint32_t>& mappingCategories);

static void GetMappingCategories(atArray<uint32_t>& mappingCategories)
{
	g_origGetMappingCategories(mappingCategories);

	mappingCategories.Set(mappingCategories.GetCount(), HashString("PM_PANE_CFX"));
}

static void(*g_origGetMappingCategoryInputs)(uint32_t* categoryId, atArray<uint32_t>& controlIds);

static void GetMappingCategoryInputs(uint32_t* categoryId, atArray<uint32_t>& controlIds)
{
	g_origGetMappingCategoryInputs(categoryId, controlIds);

	if (*categoryId == HashString("PM_PANE_CFX"))
	{
		for (auto& binding : g_registeredBindings)
		{
			controlIds.Set(controlIds.GetCount(), HashBinding(binding.first));
		}
	}
}

static void*(*g_origGetBindingForControl)(void* control, rage::ioInputSource* outBinding, uint32_t controlId, int source, uint8_t subIdx, bool unk);

static void* GetBindingForControl(void* control, rage::ioInputSource* outBinding, uint32_t controlId, int source, uint8_t subIdx, bool unk)
{
	if (controlId & 0x80000000)
	{
		outBinding->parameter = -1;
		outBinding->source = -1;
		outBinding->unk = -1;

		if (subIdx == 0)
		{
			auto controlRef = GetRegisteredBindingByHash(controlId);

			for (auto& bindingSet : bindingManager.GetBindings())
			{
				if (bindingSet.second->GetCommand() == controlRef.first && IsTagActive(bindingSet.second->GetTag()))
				{
					bindingSet.second->GetBinding(*outBinding);
					outBinding->unk = source;
					break;
				}
			}
		}

		return outBinding;
	}

	return g_origGetBindingForControl(control, outBinding, controlId, source, subIdx, unk);
}

static const char*(*g_origGetNameForControl)(uint32_t controlIdx);

static const char* GetNameForControl(uint32_t controlId)
{
	if (controlId & 0x80000000)
	{
		auto pair = GetRegisteredBindingByHash(controlId);

		static std::string str;
		str = fmt::sprintf("INPUT_%08X", HashString(pair.first.c_str()));

		// lame hack
		game::AddCustomText(HashString(str.c_str()), fmt::sprintf("%s (%s)", std::get<1>(pair.second), std::get<0>(pair.second)));

		return str.c_str();
	}

	return g_origGetNameForControl(controlId);
}

static uint32_t(*g_origGetControlForName)(const char* controlName);

static uint32_t GetControlForName(const char* controlName)
{
	auto control = g_origGetControlForName(controlName);

	if (control == -1)
	{
		for (auto& pair : g_registeredBindings)
		{
			auto str = fmt::sprintf("INPUT_%08X", HashString(pair.first.c_str()));

			if (str == controlName)
			{
				control = HashBinding(pair.first);
				break;
			}
		}
	}

	return control;
}

static void(*g_origMapControlInternal)(void* control, uint32_t controlIdx, int sourceIdx, rage::ioInputSource* input, uint32_t subIdx, bool persist);

static void MapControlInternal(void* control, uint32_t controlId, int sourceIdx, rage::ioInputSource* input, uint32_t subIdx, bool persist)
{
	if (controlId & 0x80000000)
	{
		auto pair = GetRegisteredBindingByHash(controlId);

		if (!pair.first.empty())
		{
			auto ioSource = input->source;
			auto ioParameter = input->parameter;

			bindingManager.QueueOnFrame([ioSource, ioParameter, pair]()
			{
				auto binding = bindingManager.Bind(ioSource, ioParameter, pair.first);

				if (binding)
				{
					auto tag = std::get<0>(pair.second);

					binding->SetTag(tag);
				}
			});
		}

		return;
	}

	return g_origMapControlInternal(control, controlId, sourceIdx, input, subIdx, persist);
}

static bool(*g_origHandleMappingConflicts)(void* control, uint32_t idx, rage::ioInputSource* input, void* a4, void* a5);

static bool HandleMappingConflicts(void* control, uint32_t idx, rage::ioInputSource* input, void* a4, void* a5)
{
	if (idx & 0x80000000)
	{
		return false;
	}

	return g_origHandleMappingConflicts(control, idx, input, a4, a5);
}

static void(*g_origMapFunc)(void*, uint32_t, void*, void*);

static void MapFuncHook(void* a1, uint32_t controlIdx, void* a3, void* a4)
{
	if (controlIdx < 512)
	{
		g_origMapFunc(a1, controlIdx, a3, a4);
	}
}

#include <boost/algorithm/string.hpp>

static HookFunction hookFunction([]()
{
	if (Is372())
	{
		return;
	}

	console::GetDefaultContext()->OnSaveConfiguration.Connect([](const std::function<void(const std::string&)>& writeLine)
	{
		writeLine("unbindall");

		for (auto& binding : bindingManager.GetBindings())
		{
			auto commandString = binding.second->GetCommand();
			boost::algorithm::replace_all(commandString, "\\", "\\\\");
			boost::algorithm::replace_all(commandString, "\"", "\\\"");

			rage::ioInputSource source;
			binding.second->GetBinding(source);

			auto parameterIndex = _getParameterIndex(source.source, source.parameter);

			std::string ioSource = "";
			std::string ioParameter = "";

			for (auto& entry : ioSourceMap)
			{
				if (entry.second == source.source)
				{
					ioSource = entry.first;
				}
			}

			for (auto& entry : ioParameterMap)
			{
				if (entry.second == parameterIndex)
				{
					ioParameter = entry.first;
				}
			}

			auto sourceString = fmt::sprintf("%s %s", ioSource, ioParameter);

			if (binding.second->GetTag().empty())
			{
				writeLine(fmt::sprintf("bind %s \"%s\"", sourceString, commandString));
			}
			else
			{
				writeLine(fmt::sprintf("rbind %s %s \"%s\"", binding.second->GetTag(), sourceString, commandString));
			}
		}
	});

	game::AddCustomText("PM_PANE_CFX", "FiveM");

	bindingManager.Initialize();

	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_CORE)
		{
			bindingManager.CreateButtons();
		}
	});

	{
		auto location = hook::get_pattern("45 33 C0 48 8B CF E8 ? ? ? ? 48 81 C7", 6);
		hook::set_call(&ioMapper_Update, location);
		hook::call(location, ioMapper_UpdateStub);
	}

	// coincidental call after first parser init
	{
		auto location = hook::get_pattern("48 8D 55 40 45 8A C6 89 45 44 E8 ? ? ? ? E8", 15);
		hook::set_call(&origGetFunc, location);
		hook::call(location, GetFunc);
	}

	// pause menu hooks
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("74 2A 48 63 CB 8D 56 10 48", -0x23), GetMappingCategories, (void**)&g_origGetMappingCategories);
	MH_CreateHook(hook::get_pattern("45 33 ED 4C 8B E2 41 8B C5 45 85 C0 0F 84", -0x24), GetMappingCategoryInputs, (void**)&g_origGetMappingCategoryInputs);
	MH_CreateHook(hook::get_call(hook::get_pattern("41 B9 FE FF FF FF C6 44 24 28 01 44 88 64 24 20 E8", 16)), GetBindingForControl, (void**)&g_origGetBindingForControl);
	MH_CreateHook(hook::get_call(hook::get_pattern("48 8B F9 45 8A E1 8B CE 45 8B F8 E8", 11)), GetNameForControl, (void**)&g_origGetNameForControl);
	MH_CreateHook(hook::get_pattern("4D 8B F9 45 8B E0 48 8B F9 83 FE FF 0F", -0x22), MapControlInternal, (void**)&g_origMapControlInternal);
	MH_CreateHook(hook::get_call(hook::get_pattern("48 8B DA E8 ? ? ? ? 83 F8 FF 74 27", 3)), GetControlForName, (void**)&g_origGetControlForName);
	MH_CreateHook(hook::get_pattern("33 DB 4C 8B D1 48 89 5E 50", -0x27), HandleMappingConflicts, (void**)&g_origHandleMappingConflicts);
	MH_EnableHook(MH_ALL_HOOKS);

	g_mapperOffset = *hook::get_pattern<uint32_t>("48 63 04 B0 4C 8D 89 ? ? 00 00 B9 BF", 7);

	// hacks for unknown array
	{
		auto location = hook::get_pattern<char>("44 38 34 08 0F 84");
		hook::nop(location, 4 + 6);
	}

	{
		auto location = hook::get_pattern<char>("49 8B D1 8A 1C 0E", 3);
		hook::nop(location, 3);
		hook::put<uint16_t>(location, 0x01B3);
	}

	{
		auto location = hook::get_pattern<char>("42 80 3C 08 00 74");
		hook::put<uint8_t>(location - 12, 0xEB);
		hook::nop(location, 7);
		//hook::put<uint8_t>(location + 5, 0xEB);
	}

	{
		auto location = hook::get_pattern("8A 9C 33 ? 1F 00 00 48 8D");
		hook::nop(location, 7);
		hook::put<uint16_t>(location, 0x01B3);
	}

	{
		auto location = hook::get_pattern<char>("80 BC 33 ? 1F 00 00 00 74 05");
		hook::nop(location, 10);
		//hook::put<uint8_t>(location + 8, 0xEB);
	}

	{
		auto location = hook::get_pattern("83 3C 81 06 77 25");
		hook::nop(location, 10);
		hook::put<uint16_t>(location, 0xC033);
	}

	// fix for ioValue pointer that will be out of bounds if a custom mapping exists
	{
		auto location = hook::get_pattern("48 C1 FA 02 48 8B C2 48 C1 E8 3F 48 03 D0 E8", 14);
		hook::set_call(&g_origMapFunc, location);
		hook::call(location, MapFuncHook);
	}

	// control
	g_control = hook::get_address<void*>(hook::get_pattern("74 09 48 8D 05 ? ? ? ? EB 07 48 8D 05", 5));
	g_controlSize = *hook::get_pattern<int>("E8 ? ? ? ? 48 81 C3 ? ? ? ? 48 FF CF 75 EA", 8);
});
