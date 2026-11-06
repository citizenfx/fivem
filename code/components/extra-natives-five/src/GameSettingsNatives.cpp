/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <ScriptEngine.h>
#include <ScriptSerialization.h>

#include <Hooking.h>
#include <RageParser.h>

#include <ConsoleHost.h>
#include <GameInit.h>
#include <GamePrimitives.h>
#include <NetLibrary.h>
#include <nutsnbolts.h>

#include <atArray.h>
#include <imgui.h>
#include <msgpack.hpp>

//
// Natives that expose the game's graphics/video settings (CSettingsManager) so resources can build their own
// settings menus instead of relying on the native pause menu.
//
// Values are read and written through the game's own parser metadata for CGraphicsSettings/CVideoSettings, so
// field offsets are not hardcoded. Changes are staged and then handed to CSettingsManager::RequestNewSettings,
// the same path the pause menu uses; the game applies them (including any device reset) on the next frame.
//
// Since these natives are callable by any server's scripts:
// - changes applied live are reverted when leaving the server, unless the player changed them since, and
// - changes are only saved to settings.xml after the player allows it for that server in an in-game prompt.
//

// resolution entry as stored in CSettingsManager's per-monitor mode lists
struct VideoMode
{
	uint32_t width;
	uint32_t height;
	uint32_t refreshNumerator;
	uint32_t refreshDenominator;
	uint32_t pad[2];
};

static_assert(sizeof(VideoMode) == 0x18);

// sizeof(CSettings) is 0x130 as of game build 3570; leave headroom for later builds.
static constexpr size_t kSettingsBufferSize = 0x200;

// the active CSettings lives directly after the CSettingsManager vtable
static constexpr size_t kSettingsOffset = 8;

// CSettings::graphics (CGraphicsSettings) and CSettings::video (CVideoSettings)
static constexpr size_t kGraphicsSettingsOffset = 0x10;
static constexpr size_t kVideoSettingsOffset = 0xE0;

static char* g_settingsManager;
static uint32_t g_monitorListCountOffset;

static void (*g_getSettingsCopy)(void* manager, void* outSettings);
static void (*g_requestNewSettings)(void* manager, void* settingsByValue);
static void* (*g_getResolutionList)(void* manager, int monitorIndex);
static void (*g_monitorToAdapterOutput)(int monitorIndex, int* adapterIndex, int* outputIndex);
static bool (*g_saveSettings)(void* manager, void* settings);
static bool (*g_settingsRequireRestart)(void* manager, void* newSettings);
static void (*g_destroySettings)(void* settings);

// APPLY_GAME_SETTINGS results
enum class ApplyResult : int
{
	Rejected = 0,
	Applied = 1,
	RestartRequired = 2,
};

// member names as they appear in settings.xml; the parser only stores their hashes
static const char* const g_graphicsSettingNames[] = {
	"Tessellation", "LodScale", "PedLodBias", "VehicleLodBias", "ShadowQuality", "ReflectionQuality",
	"ReflectionMSAA", "SSAO", "AnisotropicFiltering", "MSAA", "MSAAFragments", "MSAAQuality", "SamplingMode",
	"TextureQuality", "ParticleQuality", "WaterQuality", "GrassQuality", "ShaderQuality", "Shadow_SoftShadows",
	"UltraShadows_Enabled", "Shadow_ParticleShadows", "Shadow_Distance", "Shadow_LongShadows", "Shadow_SplitZStart",
	"Shadow_SplitZEnd", "Shadow_aircraftExpWeight", "Shadow_DisableScreenSizeCheck", "Reflection_MipBlur",
	"FXAA_Enabled", "TXAA_Enabled", "Lighting_FogVolumes", "Shader_SSA", "DX_Version", "CityDensity",
	"PedVarietyMultiplier", "VehicleVarietyMultiplier", "PostFX", "DoF", "HdStreamingInFlight", "MaxLodScale",
	"MotionBlurStrength",
};

static const char* const g_videoSettingNames[] = {
	"AdapterIndex", "OutputIndex", "ScreenWidth", "ScreenHeight", "RefreshRate", "Windowed", "VSync", "Stereo",
	"Convergence", "Separation", "PauseOnFocusLoss", "AspectRatio",
};

struct SettingField
{
	std::string name;
	size_t offset; // relative to CSettings
	rage::parMemberType type;
};

// keyed by HashString (case-insensitive) of the setting name
static std::map<uint32_t, SettingField> g_settingFields;

struct SettingValue
{
	rage::parMemberType type;

	union
	{
		int32_t intValue;
		float floatValue;
		bool boolValue;
	};

	bool operator==(const SettingValue& right) const
	{
		switch (type)
		{
			case rage::parMemberType::Bool:
				return boolValue == right.boolValue;
			case rage::parMemberType::Float:
				return floatValue == right.floatValue;
			default:
				return intValue == right.intValue;
		}
	}

	template<typename Packer>
	void msgpack_pack(Packer& packer) const
	{
		switch (type)
		{
			case rage::parMemberType::Bool:
				packer.pack(boolValue);
				break;
			case rage::parMemberType::Float:
				packer.pack(floatValue);
				break;
			default:
				packer.pack(intValue);
				break;
		}
	}
};

struct ScreenResolution
{
	uint32_t width;
	uint32_t height;

	MSGPACK_DEFINE_MAP(width, height);
};

// staged changes, applied on APPLY_GAME_SETTINGS
static std::map<uint32_t, float> g_pendingSettings;

// settings changed live by scripts during this server session, to be reverted when leaving
struct LiveChange
{
	SettingValue original;
	SettingValue applied;
};

static std::map<uint32_t, LiveChange> g_liveChanges;

// whether the player allowed a server (by URL) to save settings, remembered for the game session
static std::unordered_map<std::string, bool> g_saveConsent;

static size_t GetMemberOffset(rage::parStructure* structure, const char* name)
{
	for (auto member : structure->m_members)
	{
		if (member->m_definition && member->m_definition->hash == HashRageString(name))
		{
			return member->m_definition->offset;
		}
	}

	return SIZE_MAX;
}

static void AddSettingFields(const char* parentMember, size_t defaultBaseOffset, const char* structName, const char* const* names, size_t count)
{
	auto structure = rage::GetStructureDefinition(structName);

	if (!structure)
	{
		trace("Couldn't find parser structure %s for game settings natives.\n", structName);
		return;
	}

	// CSettings' own parser structure has no members registered at runtime (as of 3570), so fall back to the
	// known location of the substructure when the parser can't tell us
	size_t baseOffset = SIZE_MAX;

	if (auto settingsStructure = rage::GetStructureDefinition("CSettings"))
	{
		baseOffset = GetMemberOffset(settingsStructure, parentMember);
	}

	if (baseOffset == SIZE_MAX)
	{
		baseOffset = defaultBaseOffset;
	}

	for (size_t i = 0; i < count; i++)
	{
		for (auto member : structure->m_members)
		{
			auto definition = member->m_definition;

			if (!definition || definition->hash != HashRageString(names[i]))
			{
				continue;
			}

			switch (definition->type)
			{
				case rage::parMemberType::Bool:
				case rage::parMemberType::Int32:
				case rage::parMemberType::UInt32:
				case rage::parMemberType::Float:
					g_settingFields[HashString(names[i])] = { names[i], baseOffset + definition->offset, definition->type };
					break;
				default:
					break;
			}

			break;
		}
	}
}

static void EnsureSettingFields()
{
	if (!g_settingFields.empty())
	{
		return;
	}

	AddSettingFields("graphics", kGraphicsSettingsOffset, "CGraphicsSettings", g_graphicsSettingNames, std::size(g_graphicsSettingNames));
	AddSettingFields("video", kVideoSettingsOffset, "CVideoSettings", g_videoSettingNames, std::size(g_videoSettingNames));

	if (g_settingFields.empty())
	{
		trace("No game settings fields could be resolved for game settings natives.\n");
	}
}

static const SettingField& GetSettingField(const char* name)
{
	EnsureSettingFields();

	auto it = g_settingFields.find(HashString(name));

	if (it == g_settingFields.end())
	{
		throw std::runtime_error(va("Unknown game setting '%s'.", name));
	}

	return it->second;
}

static SettingValue ReadSetting(const char* settings, const SettingField& field)
{
	SettingValue value;
	value.type = field.type;
	value.intValue = 0;

	switch (field.type)
	{
		case rage::parMemberType::Bool:
			value.boolValue = *(bool*)(settings + field.offset);
			break;
		case rage::parMemberType::Float:
			value.floatValue = *(float*)(settings + field.offset);
			break;
		default:
			value.intValue = *(int32_t*)(settings + field.offset);
			break;
	}

	return value;
}

static void WriteSetting(char* settings, const SettingField& field, float value)
{
	switch (field.type)
	{
		case rage::parMemberType::Bool:
			*(bool*)(settings + field.offset) = (value != 0.0f);
			break;
		case rage::parMemberType::Float:
			*(float*)(settings + field.offset) = value;
			break;
		default:
			*(int32_t*)(settings + field.offset) = (int32_t)value;
			break;
	}
}

static void WriteSetting(char* settings, const SettingField& field, const SettingValue& value)
{
	switch (field.type)
	{
		case rage::parMemberType::Bool:
			*(bool*)(settings + field.offset) = value.boolValue;
			break;
		case rage::parMemberType::Float:
			*(float*)(settings + field.offset) = value.floatValue;
			break;
		default:
			*(int32_t*)(settings + field.offset) = value.intValue;
			break;
	}
}

static bool IsAvailable()
{
	return g_settingsManager && g_getSettingsCopy && g_requestNewSettings && g_getResolutionList && g_monitorToAdapterOutput &&
		g_settingsRequireRestart && g_destroySettings;
}

static void EnsureAvailable()
{
	if (!IsAvailable())
	{
		throw std::runtime_error("Game settings natives are not supported on this game build.");
	}
}

static const char* GetActiveSettings()
{
	return g_settingsManager + kSettingsOffset;
}

static atArray<VideoMode>* GetResolutionsForAdapterOutput(int adapterIndex, int outputIndex)
{
	// the per-monitor lists are built lazily by the game, so make sure they exist before reading them
	g_getResolutionList(g_settingsManager, 0);

	// atArray<atArray<VideoMode>>, one list per monitor; the getter references its count field
	auto monitorLists = (atArray<atArray<VideoMode>>*)(g_settingsManager + g_monitorListCountOffset - sizeof(void*));

	for (int monitor = 0; monitor < monitorLists->GetCount(); monitor++)
	{
		int adapter = 0;
		int output = 0;
		g_monitorToAdapterOutput(monitor, &adapter, &output);

		if (adapter == adapterIndex && output == outputIndex)
		{
			return &monitorLists->Get(monitor);
		}
	}

	return nullptr;
}

static std::vector<ScreenResolution> GetAvailableResolutions(int adapterIndex, int outputIndex)
{
	std::vector<ScreenResolution> resolutions;

	if (auto modes = GetResolutionsForAdapterOutput(adapterIndex, outputIndex))
	{
		for (int i = 0; i < modes->GetCount(); i++)
		{
			resolutions.push_back({ modes->Get(i).width, modes->Get(i).height });
		}
	}

	return resolutions;
}

static int GetPendingOrActiveInt(const char* name)
{
	auto& field = GetSettingField(name);

	if (auto it = g_pendingSettings.find(HashString(name)); it != g_pendingSettings.end())
	{
		return (int)it->second;
	}

	return ReadSetting(GetActiveSettings(), field).intValue;
}

// A copy of the active settings built with the game's own copy routine, so it can be passed by value to
// CSettingsManager::RequestNewSettings (which then owns it), or destroyed with the game's destructor.
class SettingsCopy
{
public:
	SettingsCopy()
	{
		g_getSettingsCopy(g_settingsManager, m_data);
	}

	~SettingsCopy()
	{
		if (m_owned)
		{
			g_destroySettings(m_data);
		}
	}

	SettingsCopy(const SettingsCopy&) = delete;
	SettingsCopy& operator=(const SettingsCopy&) = delete;

	char* Get()
	{
		return m_data;
	}

	// whether the pause menu would refuse to apply these settings without a game restart
	bool RequiresRestart()
	{
		return g_settingsRequireRestart(g_settingsManager, m_data);
	}

	void Request()
	{
		m_owned = false;
		g_requestNewSettings(g_settingsManager, m_data);
	}

private:
	alignas(16) char m_data[kSettingsBufferSize];
	bool m_owned = true;
};

static std::string GetCurrentServer()
{
	return Instance<NetLibrary>::Get()->GetCurrentServerUrl();
}

// undo live changes made during this server session, skipping any the player changed again since
static void RevertLiveChanges()
{
	if (g_liveChanges.empty())
	{
		return;
	}

	SettingsCopy settings;
	bool changed = false;

	for (auto& [hash, change] : g_liveChanges)
	{
		auto& field = g_settingFields[hash];

		if (ReadSetting(GetActiveSettings(), field) == change.applied)
		{
			WriteSetting(settings.Get(), field, change.original);
			changed = true;
		}
	}

	g_liveChanges.clear();

	if (changed && !settings.RequiresRestart())
	{
		settings.Request();
	}
}

//
// save consent prompt
//
enum class ConsentDecision : int
{
	None,
	Allow,
	Deny,
};

static std::unique_ptr<SettingsCopy> g_pendingSave;
static std::string g_pendingSaveServer;

static std::atomic<bool> g_consentPromptVisible;
static std::atomic<ConsentDecision> g_consentDecision{ ConsentDecision::None };
static std::mutex g_consentPromptMutex;
static std::string g_consentPromptServer;

static void ShowConsentPrompt(const std::string& server)
{
	{
		std::lock_guard _(g_consentPromptMutex);
		g_consentPromptServer = server;
	}

	if (!g_consentPromptVisible.exchange(true))
	{
		AcquireCursorMode();
	}
}

static void HideConsentPrompt()
{
	if (g_consentPromptVisible.exchange(false))
	{
		ReleaseCursorMode();
	}

	g_consentDecision = ConsentDecision::None;
}

static void SaveSettings(SettingsCopy& settings)
{
	if (g_saveSettings)
	{
		g_saveSettings(g_settingsManager, settings.Get());
	}

	// the player chose to keep these, so don't revert them when leaving
	g_liveChanges.clear();
}

// save once the player allowed this server to, asking them first if needed
static void RequestSave(std::unique_ptr<SettingsCopy> settings)
{
	auto server = GetCurrentServer();

	if (auto it = g_saveConsent.find(server); it != g_saveConsent.end())
	{
		if (it->second)
		{
			SaveSettings(*settings);
		}

		return;
	}

	// a newer request while the prompt is open replaces the pending one
	g_pendingSave = std::move(settings);
	g_pendingSaveServer = server;

	ShowConsentPrompt(server);
}

// runs on the main thread, as the prompt itself is drawn on the render thread
static void ProcessConsentDecision()
{
	auto decision = g_consentDecision.load();

	if (decision == ConsentDecision::None)
	{
		return;
	}

	bool allowed = (decision == ConsentDecision::Allow);
	g_saveConsent[g_pendingSaveServer] = allowed;

	if (allowed && g_pendingSave)
	{
		SaveSettings(*g_pendingSave);
	}

	g_pendingSave.reset();
	HideConsentPrompt();
}

static void DrawConsentPrompt()
{
	if (!g_consentPromptVisible)
	{
		return;
	}

	std::string server;

	{
		std::lock_guard _(g_consentPromptMutex);
		server = g_consentPromptServer;
	}

	ImGui::OpenPopup("##GameSettingsConsent");

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(640.0f, 0.0f), ImGuiCond_Always);

	if (ImGui::BeginPopupModal("##GameSettingsConsent", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("This server wants to save your graphics settings");
		ImGui::Separator();

		ImGui::TextWrapped("%s is asking to permanently change your game's graphics and display settings. They will stay changed after you leave this server.", server.c_str());
		ImGui::TextWrapped("Only allow this if you trust this server. Changes that aren't saved are undone when you leave.");

		ImGui::Dummy(ImVec2(0.0f, 8.0f));

		float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

		if (ImGui::Button("Allow", ImVec2(buttonWidth, 0.0f)))
		{
			g_consentDecision = ConsentDecision::Allow;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Don't allow", ImVec2(buttonWidth, 0.0f)))
		{
			g_consentDecision = ConsentDecision::Deny;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

static HookFunction hookFunction([]()
{
	// CSettingsManager::GetInstance() is inlined everywhere; take the instance from the main update's
	// lazy-init guard right before it calls CSettingsManager::Update.
	if (auto pattern = hook::pattern("8B 05 ? ? ? ? A8 01 75 21 83 C8 01 48 8D 0D ? ? ? ? 89 05 ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 84 C0 74 05 E8"); !pattern.empty())
	{
		g_settingsManager = hook::get_address<char*>(pattern.get_first(16));
	}

	if (auto pattern = hook::pattern("48 89 5C 24 08 57 48 83 EC 20 48 8B FA 48 8B D9 E8 ? ? ? ? 48 8D 53 08"); !pattern.empty())
	{
		g_getSettingsCopy = (decltype(g_getSettingsCopy))pattern.get_first();
	}

	if (auto pattern = hook::pattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B D9 48 81 C1 58"); !pattern.empty())
	{
		g_requestNewSettings = (decltype(g_requestNewSettings))pattern.get_first();
	}

	if (auto pattern = hook::pattern("48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 55 48 8B EC 48 83 EC 40 33 F6 8B FA 48 8B D9 66 39 B1"); !pattern.empty())
	{
		g_getResolutionList = (decltype(g_getResolutionList))pattern.get_first();
		g_monitorListCountOffset = *pattern.get_first<uint32_t>(0x21);
	}

	if (auto pattern = hook::pattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 20 33 DB 4D 8B F8"); !pattern.empty())
	{
		g_monitorToAdapterOutput = (decltype(g_monitorToAdapterOutput))pattern.get_first();
	}

	if (auto pattern = hook::pattern("66 39 34 48 75 F7 8D 41 01 48 8B CE"); !pattern.empty())
	{
		g_saveSettings = (decltype(g_saveSettings))pattern.get_first(-0x55);
	}

	// the check the pause menu uses to decide that a change needs a restart instead of applying it live
	if (auto pattern = hook::pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 8B 81 0C 06 00 00 33 DB 48 8B FA 48 8B F1 39 82 E8"); !pattern.empty())
	{
		g_settingsRequireRestart = (decltype(g_settingsRequireRestart))pattern.get_first();
	}

	if (auto pattern = hook::pattern("48 89 5C 24 08 57 48 83 EC 20 48 8B D9 48 8B 89 18 01 00 00 33 FF 48 85 C9 74 05 E8"); !pattern.empty())
	{
		g_destroySettings = (decltype(g_destroySettings))pattern.get_first();
	}

	if (!IsAvailable())
	{
		trace("Game settings natives are unavailable on this game build.\n");
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_GAME_SETTINGS", [](fx::ScriptContext& context)
	{
		EnsureAvailable();
		EnsureSettingFields();

		std::map<std::string, SettingValue> settings;

		for (auto& [hash, field] : g_settingFields)
		{
			settings[field.name] = ReadSetting(GetActiveSettings(), field);
		}

		context.SetResult(fx::SerializeObject(settings));
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_GAME_SETTING", [](fx::ScriptContext& context)
	{
		EnsureAvailable();

		const char* name = context.CheckArgument<const char*>(0);
		float value = context.GetArgument<float>(1);

		GetSettingField(name);
		g_pendingSettings[HashString(name)] = value;
	});

	fx::ScriptEngine::RegisterNativeHandler("CLEAR_PENDING_GAME_SETTINGS", [](fx::ScriptContext& context)
	{
		g_pendingSettings.clear();
	});

	fx::ScriptEngine::RegisterNativeHandler("APPLY_GAME_SETTINGS", [](fx::ScriptContext& context)
	{
		EnsureAvailable();

		bool save = context.GetArgument<bool>(0);

		if (g_pendingSettings.empty())
		{
			context.SetResult<int>((int)ApplyResult::Applied);
			return;
		}

		// only allow resolutions the game itself would offer for the target monitor
		int adapterIndex = GetPendingOrActiveInt("AdapterIndex");
		int outputIndex = GetPendingOrActiveInt("OutputIndex");
		int width = GetPendingOrActiveInt("ScreenWidth");
		int height = GetPendingOrActiveInt("ScreenHeight");

		auto resolutions = GetAvailableResolutions(adapterIndex, outputIndex);
		bool validResolution = std::any_of(resolutions.begin(), resolutions.end(), [width, height](const ScreenResolution& resolution)
		{
			return resolution.width == (uint32_t)width && resolution.height == (uint32_t)height;
		});

		if (!validResolution)
		{
			g_pendingSettings.clear();

			context.SetResult<int>((int)ApplyResult::Rejected);
			return;
		}

		auto buildSettings = [](SettingsCopy& settings)
		{
			for (auto& [hash, value] : g_pendingSettings)
			{
				WriteSetting(settings.Get(), g_settingFields[hash], value);
			}
		};

		SettingsCopy settings;
		buildSettings(settings);

		// saving is subject to player consent, so keep a separate copy for it
		std::unique_ptr<SettingsCopy> saveSettings;

		if (save)
		{
			saveSettings = std::make_unique<SettingsCopy>();
			buildSettings(*saveSettings);
		}

		// like the pause menu, don't apply changes live that the game only picks up on restart
		bool restartRequired = settings.RequiresRestart();

		if (!restartRequired)
		{
			for (auto& [hash, value] : g_pendingSettings)
			{
				auto& field = g_settingFields[hash];
				auto applied = ReadSetting(settings.Get(), field);

				// keep the value from before the first change this session, so leaving restores that
				if (auto it = g_liveChanges.find(hash); it != g_liveChanges.end())
				{
					it->second.applied = applied;
				}
				else
				{
					g_liveChanges[hash] = { ReadSetting(GetActiveSettings(), field), applied };
				}
			}

			settings.Request();
		}

		g_pendingSettings.clear();

		if (saveSettings)
		{
			RequestSave(std::move(saveSettings));
		}

		context.SetResult<int>((int)(restartRequired ? ApplyResult::RestartRequired : ApplyResult::Applied));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_GAME_SETTING_REQUIRES_RESTART", [](fx::ScriptContext& context)
	{
		EnsureAvailable();

		auto& field = GetSettingField(context.CheckArgument<const char*>(0));

		// ask the game's own check whether changing just this setting would need a restart
		SettingsCopy settings;
		auto current = ReadSetting(settings.Get(), field);

		switch (field.type)
		{
			case rage::parMemberType::Bool:
				WriteSetting(settings.Get(), field, current.boolValue ? 0.0f : 1.0f);
				break;
			case rage::parMemberType::Float:
				WriteSetting(settings.Get(), field, current.floatValue + 1.0f);
				break;
			default:
				WriteSetting(settings.Get(), field, (float)(current.intValue + 1));
				break;
		}

		context.SetResult<bool>(settings.RequiresRestart());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_AVAILABLE_SCREEN_RESOLUTIONS", [](fx::ScriptContext& context)
	{
		EnsureAvailable();

		auto settings = GetActiveSettings();
		int adapterIndex = ReadSetting(settings, GetSettingField("AdapterIndex")).intValue;
		int outputIndex = ReadSetting(settings, GetSettingField("OutputIndex")).intValue;

		context.SetResult(fx::SerializeObject(GetAvailableResolutions(adapterIndex, outputIndex)));
	});

	OnMainGameFrame.Connect([]()
	{
		ProcessConsentDecision();
	});

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || g_consentPromptVisible;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		DrawConsentPrompt();
	});

	// changes from scripts only last for the server session, unless the player allowed saving them
	OnKillNetworkDone.Connect([]()
	{
		g_pendingSettings.clear();
		g_pendingSave.reset();
		HideConsentPrompt();

		if (IsAvailable())
		{
			RevertLiveChanges();
		}
	});
});
