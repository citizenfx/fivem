#include "StdInc.h"
#include "Hooking.h"

#include <GameInit.h>
#include <ICoreGameInit.h>
#include <nutsnbolts.h>

#include <NetLibrary.h>
#include <MumbleClient.h>

#include <sstream>
#include <regex>

#include <json.hpp>

#include <scrEngine.h>
#include <MinHook.h>

#include <mmsystem.h>
#include <dsound.h>
#include <ScriptEngine.h>
#include <scrEngine.h>

#include <CoreConsole.h>

#include <sysAllocator.h>

#include <NetworkPlayerMgr.h>
#include <netObject.h>

#include <CrossBuildRuntime.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include "ScriptWarnings.h"

#if __has_include(<GameInput.h>)
#include <GameInput.h>
#endif

#if __has_include(<GameAudioState.h>)
#include <GameAudioState.h>
#endif

using json = nlohmann::json;

static NetLibrary* g_netLibrary;

#ifdef GTA_FIVE
static uint32_t g_gamerInfoGamerIdOffset;
static uint32_t g_playerInfoPedOffset;
#endif

#ifdef GTA_FIVE
static hook::cdecl_stub<void()> _initVoiceChatConfig([]()
{
	return hook::get_pattern("89 44 24 58 0F 29 44 24 40 E8", -0x12E);
});
#elif IS_RDR3
static hook::cdecl_stub<void(void*)> _initVoiceChatConfig([]()
{
	return hook::get_pattern("8B 83 ? ? ? ? F2 0F 10 8B ? ? ? ? 48", -0xDD);
});

static hook::cdecl_stub<int(void*, uint64_t, uint32_t)> rage__atDataHash([]()
{
	return hook::get_call(hook::get_pattern("7E 7A BB DD 78 22 A8 4C 8D", -12));
});
#endif

class VoiceChatPrefs
{
public:
	static void InitConfig();
	static bool IsEnabled();
	static bool IsMicEnabled();
	static int GetOutputDevice();
	static int GetInputDevice();
	static int GetOutputVolume();
	static int GetMicSensitivity();
	static int GetChatMode();
};

#ifdef GTA_FIVE
static uint32_t* g_preferenceArray;

// Virtual mapping - incomplete
enum eMenuPref
{
	PREF_VOICE_ENABLE = 0,
	PREF_VOICE_OUTPUT = 1, // NOT A VOICE PREF - needed for index to match
	PREF_VOICE_OUTPUT_DEVICE = 2,
	PREF_VOICE_OUTPUT_VOLUME = 3,
	PREF_VOICE_SOUND_VOLUME = 9,
	PREF_VOICE_MUSIC_VOLUME = 10,
	PREF_VOICE_TALK_ENABLED = 4,
	//PREF_VOICE_FEEDBACK,
	PREF_VOICE_INPUT_DEVICE = 5,
	PREF_VOICE_CHAT_MODE = 6,
	PREF_VOICE_MIC_VOLUME = 7,
	PREF_VOICE_MIC_SENSITIVITY = 8,
};

static std::array<uint8_t, 11> voicePrefEnums;

static int MapPrefsEnum(const int index)
{
	return voicePrefEnums[index];
}

static void GetDynamicVoicePrefEnums()
{
	uint8_t inserted = 0;
	uint8_t offset = 0xFF;

	auto instructionPtr = hook::get_pattern<uint8_t>("40 56 48 83 EC ? BE");

	while (inserted < 11)
	{
		if (instructionPtr[0] == 0xBE && offset == 0xFF)
		{
			assert(inserted == 0);
			offset = instructionPtr[1];
		}

		if (instructionPtr[0] == 0x8D && instructionPtr[1] == 0x4E)
		{
			assert(offset != 0xFF && inserted < 11);

			const uint8_t enumVal = offset + instructionPtr[2];

			voicePrefEnums[inserted++] = enumVal;
		}

		++instructionPtr;
	}

	assert(inserted == 11);
}

void VoiceChatPrefs::InitConfig()
{
	_initVoiceChatConfig();
}

bool VoiceChatPrefs::IsEnabled()
{
	return g_preferenceArray[MapPrefsEnum(PREF_VOICE_ENABLE)];
}

bool VoiceChatPrefs::IsMicEnabled()
{
	return g_preferenceArray[MapPrefsEnum(PREF_VOICE_TALK_ENABLED)];
}

int VoiceChatPrefs::GetOutputDevice()
{
	return g_preferenceArray[MapPrefsEnum(PREF_VOICE_OUTPUT_DEVICE)];
}

int VoiceChatPrefs::GetInputDevice()
{
	return g_preferenceArray[MapPrefsEnum(PREF_VOICE_INPUT_DEVICE)];
}

int VoiceChatPrefs::GetOutputVolume()
{
	return g_preferenceArray[MapPrefsEnum(PREF_VOICE_OUTPUT_VOLUME)];
}

int VoiceChatPrefs::GetMicSensitivity()
{
	return g_preferenceArray[MapPrefsEnum(PREF_VOICE_MIC_SENSITIVITY)];
}

int VoiceChatPrefs::GetChatMode()
{
	return g_preferenceArray[MapPrefsEnum(PREF_VOICE_CHAT_MODE)];
}
#elif IS_RDR3
struct VoiceChatMgrPrefs
{
	bool m_voiceEnabled;
	bool m_voiceOutput;
	bool m_talkEnabled;
	bool m_unk1;
	int m_volume;
	int m_micVolume;
	uint16_t m_micSensitivity;
	uint16_t m_unk2;
	int m_inputDevice;
	int m_outputDevice;
	int m_voiceChatMode;
};

static VoiceChatMgrPrefs* g_voiceChatMgrPrefs;
static bool g_voiceChatPrefEnabled;
static void* g_voiceChatMgr;

void VoiceChatPrefs::InitConfig()
{
	_initVoiceChatConfig(g_voiceChatMgr);
}

bool VoiceChatPrefs::IsEnabled()
{
	return g_voiceChatPrefEnabled;
}

bool VoiceChatPrefs::IsMicEnabled()
{
	return g_voiceChatMgrPrefs->m_talkEnabled;
}

int VoiceChatPrefs::GetOutputDevice()
{
	return g_voiceChatMgrPrefs->m_outputDevice;
}

int VoiceChatPrefs::GetInputDevice()
{
	return g_voiceChatMgrPrefs->m_inputDevice;
}

int VoiceChatPrefs::GetOutputVolume()
{
	return g_voiceChatMgrPrefs->m_volume;
}

int VoiceChatPrefs::GetMicSensitivity()
{
	return g_voiceChatMgrPrefs->m_micSensitivity;
}

int VoiceChatPrefs::GetChatMode()
{
	return g_voiceChatMgrPrefs->m_voiceChatMode;
}
#endif

void MumbleVoice_BindNetLibrary(NetLibrary* library)
{
	g_netLibrary = library;
}

#ifdef GTA_FIVE
void Policy_BindNetLibrary(NetLibrary* library)
{
	g_netLibrary = library;
}
#endif

static IMumbleClient* g_mumbleClient;

static struct  
{
	volatile bool connecting;
	volatile bool connected;
	volatile bool errored;

	volatile MumbleConnectionInfo* connectionInfo;

	volatile int nextConnectDelay;
	volatile uint64_t nextConnectAt;

	boost::optional<net::PeerAddress> overridePeer;

	concurrency::concurrent_queue<std::function<void()>> mainFrameExecQueue;
} g_mumble;

static bool g_voiceActiveByScript = true;

static bool Mumble_ShouldConnect()
{
	return VoiceChatPrefs::IsEnabled() && Instance<ICoreGameInit>::Get()->OneSyncEnabled && g_voiceActiveByScript;
}

static void Mumble_Connect(bool isReconnect = false)
{
	g_mumble.connected = false;
	g_mumble.errored = false;
	g_mumble.connecting = true;

	VoiceChatPrefs::InitConfig();

	g_mumbleClient->ConnectAsync(g_mumble.overridePeer ? *g_mumble.overridePeer : g_netLibrary->GetCurrentPeer(), fmt::sprintf("[%d] %s", g_netLibrary->GetServerNetID(), g_netLibrary->GetPlayerName())).then([isReconnect](concurrency::task<MumbleConnectionInfo*> task)
	{
		try
		{
			auto info = task.get();

			auto eventManager = Instance<fx::ResourceManager>::Get()->GetComponent<fx::ResourceEventManagerComponent>();

			/*NETEV mumbleConnected CLIENT
			/#*
			 * An event triggered when the game completes (re)connecting to a Mumble server.
			 *
			 * @param address - The address of the Mumble server connected to.
			 * @param reconnecting - Is this a reconnection to a Mumble server.
			 #/
			declare function mumbleConnected(address: string, reconnecting: boolean): void;
			*/
			eventManager->QueueEvent2("mumbleConnected", {}, info->address.ToString(), isReconnect);

			g_mumble.connectionInfo = g_mumbleClient->GetConnectionInfo();

			g_mumble.connected = true;
			g_mumble.nextConnectDelay = 4 * 1000;

			g_mumble.mainFrameExecQueue.push([]()
			{
				VoiceChatPrefs::InitConfig();
			});
		}
		catch (std::exception& e)
		{
			trace("Exception: %s\n", e.what());

			g_mumble.errored = true;
		}

		g_mumble.connecting = false;
	});
}

static void Mumble_Disconnect(bool reconnect = false)
{
	auto mumbleConnectionInfo = g_mumbleClient->GetConnectionInfo();

	g_mumble.connected = false;
	g_mumble.errored = false;
	g_mumble.connecting = false;
	g_mumble.nextConnectDelay = 4 * 1000;

	g_mumbleClient->DisconnectAsync().then([=]()
	{
		if (!reconnect && mumbleConnectionInfo)
		{
			auto eventManager = Instance<fx::ResourceManager>::Get()->GetComponent<fx::ResourceEventManagerComponent>();

			/*NETEV mumbleDisconnected CLIENT
			/#*
			 * An event triggered when the game disconnects from a Mumble server without being reconnected.
			 *
			 * @param address - The address of the Mumble server disconnected from.
			 #/
			declare function mumbleDisconnected(address: string): void;
			*/
			eventManager->QueueEvent2("mumbleDisconnected", {}, mumbleConnectionInfo ? mumbleConnectionInfo->address.ToString() : NULL);
		}
		
		if (reconnect && Mumble_ShouldConnect())
		{
			Mumble_Connect(true);
		}
	});

	VoiceChatPrefs::InitConfig();
}

struct grcViewport
{
	float m_mat1[16];
	float m_mat2[16];
	float m_mat3[16];
	float m_camMatrix[16];
};

struct CViewportGame
{
public:
	virtual ~CViewportGame() = 0;

private:
	char m_pad[8];

public:
	grcViewport viewport;
};

static CViewportGame** g_viewportGame;
static float* g_actorPos;

#pragma comment(lib, "dsound.lib")

bool IsMumbleConnected()
{
	if (!g_mumble.connectionInfo)
	{
		return false;
	}
	
	return g_mumble.connected && g_mumble.connectionInfo->isConnected;
}

static void Mumble_RunFrame()
{
	if (!Instance<ICoreGameInit>::Get()->HasVariable("gameSettled"))
	{
		return;
	}

	if (g_netLibrary->GetConnectionState() != NetLibrary::CS_ACTIVE)
	{
		return;
	}

	if (!IsMumbleConnected())
	{
		if (Mumble_ShouldConnect() && !g_mumble.connecting && !g_mumble.errored)
		{
			if (GetTickCount64() > g_mumble.nextConnectAt)
			{
				Mumble_Connect();
				
				g_mumble.nextConnectDelay = std::min(g_mumble.nextConnectDelay * 2, 30'000);

				g_mumble.nextConnectAt = GetTickCount64() + g_mumble.nextConnectDelay;
			}
		}
	}
	else
	{
		if (!Mumble_ShouldConnect())
		{
			Mumble_Disconnect();
		}
	}

	MumbleActivationMode activationMode;

	if (VoiceChatPrefs::IsMicEnabled())
	{
		if (VoiceChatPrefs::GetChatMode() == 1)
		{
			activationMode = MumbleActivationMode::PushToTalk;
		}
		else
		{
			activationMode = MumbleActivationMode::VoiceActivity;
		}
	}
	else
	{
		activationMode = MumbleActivationMode::Disabled;
	}

	g_mumbleClient->SetActivationMode(activationMode);

#if __has_include(<GameAudioState.h>)
	if (ShouldMuteGameAudio())
	{
		g_mumbleClient->SetOutputVolume(0.0f);
	}
	else
#endif
	{
		g_mumbleClient->SetOutputVolume(VoiceChatPrefs::GetOutputVolume() * 0.1f);
	}

	float cameraFront[3];
	float cameraTop[3];
	float cameraPos[3];
	float actorPos[3];

	cameraFront[0] = -(*g_viewportGame)->viewport.m_camMatrix[8];
	cameraFront[1] = -(*g_viewportGame)->viewport.m_camMatrix[10];
	cameraFront[2] = -(*g_viewportGame)->viewport.m_camMatrix[9];

	cameraTop[0] = (*g_viewportGame)->viewport.m_camMatrix[4];
	cameraTop[1] = (*g_viewportGame)->viewport.m_camMatrix[6];
	cameraTop[2] = (*g_viewportGame)->viewport.m_camMatrix[5];

	cameraPos[0] = (*g_viewportGame)->viewport.m_camMatrix[12];
	cameraPos[1] = (*g_viewportGame)->viewport.m_camMatrix[14];
	cameraPos[2] = (*g_viewportGame)->viewport.m_camMatrix[13];

	actorPos[0] = g_actorPos[0];
	actorPos[1] = g_actorPos[2];
	actorPos[2] = g_actorPos[1];

#ifdef GTA_FIVE
	static auto getCam1 = fx::ScriptEngine::GetNativeHandler(0x19CAFA3C87F7C2FF);
	static auto getCam2 = fx::ScriptEngine::GetNativeHandler(0xEE778F8C7E1142E2);
	bool isInFirstPerson = FxNativeInvoke::Invoke<int>(getCam2, FxNativeInvoke::Invoke<int>(getCam1)) == 4;
#elif IS_RDR3
	static auto isInFullFirstPersonMode = fx::ScriptEngine::GetNativeHandler(0xD1BA66940E94C547);
	static auto isFirstPersonCameraActive = fx::ScriptEngine::GetNativeHandler(0xA24C1D341C6E0D53);
	bool isInFirstPerson = FxNativeInvoke::Invoke<bool>(isInFullFirstPersonMode) && FxNativeInvoke::Invoke<bool>(isFirstPersonCameraActive, 1, 0, 0);
#endif

	if (isInFirstPerson)
	{
		actorPos[0] = cameraPos[0];
		actorPos[1] = cameraPos[1];
		actorPos[2] = cameraPos[2];
	}

	g_mumbleClient->SetListenerMatrix(actorPos, cameraFront, cameraTop);
	g_mumbleClient->SetActorPosition(actorPos);

	auto likelihoodValue = VoiceChatPrefs::GetMicSensitivity();

	if (likelihoodValue >= 0 && likelihoodValue < 3)
	{
		g_mumbleClient->SetActivationLikelihood(MumbleVoiceLikelihood::VeryLowLikelihood);
	}
	else if (likelihoodValue >= 3 && likelihoodValue < 6)
	{
		g_mumbleClient->SetActivationLikelihood(MumbleVoiceLikelihood::LowLikelihood);
	}
	else if (likelihoodValue >= 6 && likelihoodValue < 9)
	{
		g_mumbleClient->SetActivationLikelihood(MumbleVoiceLikelihood::ModerateLikelihood);
	}
	else
	{
		g_mumbleClient->SetActivationLikelihood(MumbleVoiceLikelihood::HighLikelihood);
	}

	{
		// handle PTT
#ifdef GTA_FIVE
		constexpr const uint32_t INPUT_PUSH_TO_TALK = 249;
#elif IS_RDR3
		constexpr const uint32_t INPUT_PUSH_TO_TALK = HashString("INPUT_PUSH_TO_TALK");
#endif

		constexpr const uint32_t PLAYER_CONTROL = 0;
		static auto isControlPressed = fx::ScriptEngine::GetNativeHandler(0xF3A21BCD95725A4A);
		static auto isControlEnabled = fx::ScriptEngine::GetNativeHandler(0x1CEA6BFDF248E5D9);
		bool pushToTalkPressed = FxNativeInvoke::Invoke<bool>(isControlPressed, PLAYER_CONTROL, INPUT_PUSH_TO_TALK);
		bool pushToTalkEnabled = FxNativeInvoke::Invoke<bool>(isControlEnabled, PLAYER_CONTROL, INPUT_PUSH_TO_TALK);

#if __has_include(<GameInput.h>)
		// game::IsControlKeyDown doesn't take enabled/disabled state into account, so we manually check enabled state
		g_mumbleClient->SetPTTButtonState(pushToTalkEnabled && (pushToTalkPressed || game::IsControlKeyDown(249 /* INPUT_PUSH_TO_TALK */)));
#else
		g_mumbleClient->SetPTTButtonState(pushToTalkEnabled && pushToTalkPressed);
#endif
	}

	// handle device changes
	static int curInDevice = -1;
	static int curOutDevice = -1;

	int inDevice = VoiceChatPrefs::GetInputDevice();
	int outDevice = VoiceChatPrefs::GetOutputDevice();

	struct EnumCtx
	{
		int target;
		int cur;
		GUID guid;
		std::string guidStr;
	} enumCtx;

#ifdef GTA_FIVE
	LPDSENUMCALLBACKW enumCb = [](LPGUID guid, LPCWSTR desc, LPCWSTR, void* cxt) -> BOOL
	{
		auto ctx = (EnumCtx*)cxt;

		if (ctx->cur == ctx->target)
		{
			if (!guid)
			{
				ctx->guidStr = "";
			}
			else
			{
				ctx->guid = *guid;
				ctx->guidStr = fmt::sprintf("{%08lX-%04hX-%04hX-%02hX%02hX-%02hX%02hX%02hX%02hX%02hX%02hX}", guid->Data1, guid->Data2, guid->Data3,
				guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
				guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
			}

			return FALSE;
		}

		ctx->cur++;
		return TRUE;
	};
#elif IS_RDR3
	LPDSENUMCALLBACKW enumCb = [](LPGUID guid, LPCWSTR desc, LPCWSTR, void* cxt) -> BOOL
	{
		if (!guid)
		{
			return TRUE;
		}

		auto ctx = (EnumCtx*)cxt;
		auto current = rage__atDataHash((void*)guid, 16, 0);
		
		if (current == ctx->target)
		{
			if (!guid)
			{
				ctx->guidStr = "";
			}
			else
			{
				ctx->guid = *guid;
				ctx->guidStr = fmt::sprintf("{%08lX-%04hX-%04hX-%02hX%02hX-%02hX%02hX%02hX%02hX%02hX%02hX}", guid->Data1, guid->Data2, guid->Data3,
					guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
					guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
			}

			return FALSE;
		}

		return TRUE;
	};
#endif

	if (inDevice != curInDevice)
	{
		console::DPrintf("Mumble", __FUNCTION__ ": capture device changed in GTA code, changing to index %d (last %d)\n", inDevice, curInDevice);

		enumCtx.cur = -1;
		enumCtx.target = inDevice;
		DirectSoundCaptureEnumerateW(enumCb, &enumCtx);

		console::DPrintf("Mumble", __FUNCTION__ ": this device index is GUID %s\n", enumCtx.guidStr);

		g_mumbleClient->SetInputDevice(enumCtx.guidStr);

		curInDevice = inDevice;

		console::DPrintf("Mumble", __FUNCTION__ ": device should've changed by now!\n");
	}

	if (outDevice != curOutDevice)
	{
		enumCtx.cur = -1;
		enumCtx.target = outDevice;
		DirectSoundEnumerateW(enumCb, &enumCtx);

		g_mumbleClient->SetOutputDevice(enumCtx.guidStr);

		curOutDevice = outDevice;
	}

	g_mumbleClient->RunFrame();
}

static std::bitset<256> g_talkers;
static std::bitset<256> o_talkers;

static std::unordered_map<std::string, int> g_userNamesToClientIds;

static auto PositionHook(const std::string& userName) -> std::optional<std::array<float, 3>>
{
	auto it = g_userNamesToClientIds.find(userName);

	if (it == g_userNamesToClientIds.end())
	{
		if (userName.length() >= 2)
		{
			int serverId = atoi(userName.substr(1, userName.length() - 1).c_str());

			it = g_userNamesToClientIds.insert({ userName, serverId }).first;
		}
	}

	if (it != g_userNamesToClientIds.end())
	{
		rage::sysMemAllocator::UpdateAllocatorValue();

		static auto getByServerId = fx::ScriptEngine::GetNativeHandler(HashString("GET_PLAYER_FROM_SERVER_ID"));

#ifdef GTA_FIVE
		static auto getPlayerPed = fx::ScriptEngine::GetNativeHandler(0x43A66C31C68491C0);
#elif IS_RDR3
		static auto getPlayerPed = fx::ScriptEngine::GetNativeHandler(0x275F255ED201B937);
#endif

		auto playerId = FxNativeInvoke::Invoke<uint32_t>(getByServerId, it->second);

		if (playerId < 256 && playerId != -1)
		{
			int ped = FxNativeInvoke::Invoke<int>(getPlayerPed, playerId);

			if (ped > 0)
			{
#ifdef GTA_FIVE
				auto coords = NativeInvoke::Invoke<0x3FEF770D40960D5A, scrVector>(ped);
#elif IS_RDR3
				auto coords = NativeInvoke::Invoke<0xA86D5F069399F44D, scrVector>(ped);
#endif

				return { { coords.x, coords.z, coords.y } };
			}
		}
	}

	return {};
}

static HookFunction initFunction([]()
{
	g_mumble.nextConnectDelay = 4 * 1000;

	auto mc = CreateMumbleClient();
	mc->AddRef();
	g_mumbleClient = mc.GetRef();

	g_mumbleClient->Initialize();

	g_mumbleClient->SetPositionHook(PositionHook);

	OnMainGameFrame.Connect([=]()
	{
		Mumble_RunFrame();
	});

	OnKillNetworkDone.Connect([]()
	{
		g_mumbleClient->SetAudioDistance(0.0f);
		g_mumble.overridePeer = {};

		Mumble_Disconnect();
		o_talkers.reset();

		g_voiceActiveByScript = true;
	});
});

static bool(*g_origIsAnyoneTalking)(void*);

static bool _isAnyoneTalking(void* mgr)
{
	return (g_origIsAnyoneTalking(mgr) || g_mumbleClient->IsAnyoneTalking());
}

static bool(*g_origIsPlayerTalking)(void*, void*);

extern CNetGamePlayer* netObject__GetPlayerOwner(rage::netObject* object);

extern rage::netObject* GetNetObjectFromEntity(void* entity);

static bool _isPlayerTalking(void* mgr, char* playerData)
{
	if (g_origIsPlayerTalking(mgr, playerData))
	{
		return true;
	}

#ifdef GTA_FIVE
	// rlGamerInfo = rlGamerId - (1604: 64, 2060: 72, 2372: 104, 2944: 192, ...)
	// CPlayerInfo = rlGamerInfo - 32 (fwExtensibleBase offset, unlikely to change)
	auto playerInfo = playerData - g_gamerInfoGamerIdOffset - 32;

	// preemptive check for invalid players (FIVEM-CLIENT-1604-TQKV) with uncertain server changes
	if ((uintptr_t)playerInfo < 0xFFFF)
	{
		return false;
	}

	// get the ped
	auto ped = *(char**)(playerInfo + g_playerInfoPedOffset);
#elif IS_RDR3
	// rlGamerInfo = playerData - 200
	// CPed = rlGamerInfo + 896
	auto ped = *(char**)(playerData + 696);
#endif

	if (ped)
	{
		auto netObj = GetNetObjectFromEntity(ped);

		if (netObj)
		{
			// actually: netobj owner
			if (auto owner = netObject__GetPlayerOwner(netObj))
			{
				auto index = owner->physicalPlayerIndex();

				if (g_talkers.test(index) || o_talkers.test(index))
				{
					return true;
				}
			}
		}
	}

	return false;
}

static bool(*g_origGetPlayerHasHeadset)(void*, void*);

static bool _getPlayerHasHeadset(void* mgr, void* plr)
{
	if (IsMumbleConnected())
	{
		return true;
	}

	return g_origGetPlayerHasHeadset(mgr, plr);
}

static float(*g_origGetLocalAudioLevel)(void* mgr, int localIdx);

static float _getLocalAudioLevel(void* mgr, int localIdx)
{
	float mumbleLevel = (IsMumbleConnected()) ? g_mumbleClient->GetInputAudioLevel() : 0.0f;

	return std::max(g_origGetLocalAudioLevel(mgr, localIdx), mumbleLevel);
}

static void(*g_origInitVoiceEngine)(void* engine, char* config);

static void _filterVoiceChatConfig(void* engine, char* config)
{
#ifdef IS_RDR3
	// cache enabled state preference
	g_voiceChatPrefEnabled = *config;
#endif

	// disable voice if mumble is used
	if (IsMumbleConnected())
	{
		*config = 0;
	}

	g_origInitVoiceEngine(engine, config);
}

#include <LabSound/extended/LabSound.h>

static fx::TNativeHandler getPlayerName;
static fx::TNativeHandler getServerId;

static std::optional<std::string> getMumbleName(int playerId)
{
	int serverId = FxNativeInvoke::Invoke<int>(getServerId, playerId);

	// if the server id is 0 then we don't have a player.
	if (serverId == 0)
	{
		return std::nullopt;
	}
	
	return fmt::sprintf("[%d] %s",
		serverId,
		FxNativeInvoke::Invoke<const char*>(getPlayerName, playerId));
}

static std::shared_ptr<lab::AudioContext> getAudioContext(int playerId)
{
	const auto name = getMumbleName(playerId);

	// if the server id is 0 then we don't have a player.
	if (!IsMumbleConnected() || !name)
	{
		return {};
	}

	return g_mumbleClient->GetAudioContext(*name);
}

static std::shared_ptr<lab::AudioContext> getAudioContextByServerId(int serverId)
{
	if (!IsMumbleConnected())
	{
		return {};
	}
	
	std::string name = g_mumbleClient->GetPlayerNameFromServerId(serverId);
	if (name.empty())
	{
		return {};
	}
	
	return g_mumbleClient->GetAudioContext(name);
}

std::string GetMumbleChannel(int channelId)
{
	return fmt::sprintf("Game Channel %d", channelId);
}

// Returns true if the voice target id valid to use with the `VoiceTarget` packet (1..30)
// see: https://github.com/citizenfx/fivem/blob/0ec3c8f9f6e715e65beca971712384d0300a553a/code/components/voip-server-mumble/src/Mumble.proto#L438-L441
bool IsVoiceTargetIdValid(int id)
{
	return id >= 1 && id <= 30;
}

// Ensures that mumble is connected before calling any mumble related functions
template<typename MumbleFn>
inline auto MakeMumbleNative(MumbleFn fn, uintptr_t defaultValue = 0)
{
	return [=](fx::ScriptContext& context)
	{
		if (!IsMumbleConnected())
		{
			context.SetResult(defaultValue);
			return;
		}
		
		fn(context);
	};
};

static void InvalidTargetIdWarning(const std::string_view& nativeName)
{
	fx::scripting::Warningf("mumble", "%s: Tried to use an invalid targetId, the minimum target id is 1, the maximum is 30.", nativeName);
}

#include <scrBind.h>

static HookFunction hookFunction([]()
{
#ifdef GTA_FIVE
	g_preferenceArray = hook::get_address<uint32_t*>(hook::get_pattern("48 8D 15 ? ? ? ? 8D 43 01 83 F8 02 77 2D", 3));
	g_viewportGame = hook::get_address<CViewportGame**>(hook::get_pattern("33 C0 48 39 05 ? ? ? ? 74 2E 48 8B 0D ? ? ? ? 48 85 C9 74 22", 5));
	g_actorPos = hook::get_address<float*>(hook::get_pattern("BB 00 00 40 00 48 89 7D F8 89 1D", -4)) + 12;

	{
		auto location = hook::get_pattern<char>("BA 11 00 00 00 0F 10");
		g_gamerInfoGamerIdOffset = xbr::IsGameBuildOrGreater<2944>() ? *(uint32_t*)(location + 8) : *(uint8_t*)(location - 1);
	}

	g_playerInfoPedOffset = *hook::get_pattern<uint32_t>("4C 8B 81 ? ? ? ? 41 8B 80", 3);

	GetDynamicVoicePrefEnums();
#elif IS_RDR3
	g_viewportGame = hook::get_address<CViewportGame**>(hook::get_pattern("0F 2F F0 76 ? 4C 8B 35", 8));

	g_actorPos = hook::get_address<float*>(hook::get_pattern("45 33 C9 48 89 5D E0 8D 53 01", 63)) + 16;

	{
		auto location = hook::get_pattern<char>("75 0D 8B C8 E8 ? ? ? ? 84 C0 B0 01 75 03");
		auto prefsOffset = *(uint32_t*)(location +  40);

		g_voiceChatMgr = *hook::get_address<void**>(location - 16);
		g_voiceChatMgrPrefs = (VoiceChatMgrPrefs*)((uint64_t)g_voiceChatMgr + prefsOffset);
	}
#endif

	rage::scrEngine::OnScriptInit.Connect([]()
	{
		static auto isPlayerActive = fx::ScriptEngine::GetNativeHandler(0xB8DFD30D6973E135);
		getServerId = fx::ScriptEngine::GetNativeHandler(HashString("GET_PLAYER_SERVER_ID"));

#ifdef GTA_FIVE
		getPlayerName = fx::ScriptEngine::GetNativeHandler(0x6D0DE6A7B5DA71F8);
#elif IS_RDR3
		getPlayerName = fx::ScriptEngine::GetNativeHandler(0x7124FD9AC0E01BA0);
#endif

		OnMainGameFrame.Connect([=]()
		{
			std::function<void()> func;

			while (g_mumble.mainFrameExecQueue.try_pop(func))
			{
				func();
			}

			if (!IsMumbleConnected())
			{
				return;
			}

			std::vector<std::string> talkers;
			g_mumbleClient->GetTalkers(&talkers);

			std::set<std::string> talkerSet(talkers.begin(), talkers.end());

			g_talkers.reset();

			for (int i = 0; i < 256; i++)
			{
				if (FxNativeInvoke::Invoke<bool>(isPlayerActive, i))
				{
					static std::map<int, std::string> names;

					int sid = FxNativeInvoke::Invoke<int>(getServerId, i);
					auto nameIt = names.find(sid);

					if (nameIt == names.end())
					{
						nameIt = names.emplace(sid, fmt::sprintf("[%d] %s",
							sid,
							FxNativeInvoke::Invoke<const char*>(getPlayerName, i))).first;
					}

					if (talkerSet.find(nameIt->second) != talkerSet.end())
					{
						g_talkers.set(i);
					}
				}
			}
		});


		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_TALKER_PROXIMITY", MakeMumbleNative([](fx::ScriptContext& context)
		{
			float proximity = context.GetArgument<float>(0);

			g_mumbleClient->SetAudioDistance(proximity);
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_GET_TALKER_PROXIMITY", MakeMumbleNative([](fx::ScriptContext& context)
		{
			context.SetResult<float>(g_mumbleClient->GetAudioDistance());
		}, 0.0f));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_ACTIVE", [](fx::ScriptContext& context)
		{
			g_voiceActiveByScript = context.GetArgument<bool>(0);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_IS_ACTIVE", [](fx::ScriptContext& context)
		{
			context.SetResult<bool>(g_voiceActiveByScript);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_IS_PLAYER_TALKING", MakeMumbleNative([](fx::ScriptContext& context)
		{
			int playerId = context.GetArgument<int>(0);
			bool isTalking = false;

			if (playerId >= 0 && playerId < g_talkers.size())
			{
				isTalking = g_talkers.test(playerId);
			}

			context.SetResult(isTalking);
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_VOLUME_OVERRIDE", MakeMumbleNative([](fx::ScriptContext& context)
		{
			int playerId = context.GetArgument<int>(0);
			float volume = context.GetArgument<float>(1);

			if (auto name = getMumbleName(playerId))
			{
				g_mumbleClient->SetClientVolumeOverride(*name, volume);
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_VOLUME_OVERRIDE_BY_SERVER_ID", MakeMumbleNative([](fx::ScriptContext& context)
		{
			int serverId = context.GetArgument<int>(0);
			float volume = context.GetArgument<float>(1);

			g_mumbleClient->SetClientVolumeOverrideByServerId(serverId, volume);
		}));

		static VoiceTargetConfig vtConfigs[31];


		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_VOICE_TARGET", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);

			if (IsVoiceTargetIdValid(id))
			{
				vtConfigs[id] = {};
				g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
			}
			else
			{
				InvalidTargetIdWarning("MUMBLE_CLEAR_VOICE_TARGET");
			}
		}));


		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_REMOVE_VOICE_TARGET_CHANNEL", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			auto channel = context.GetArgument<int>(1);

			if (IsVoiceTargetIdValid(id))
			{
				auto targetChannel = GetMumbleChannel(channel);
				auto& targets = vtConfigs[id];

				// we only want to mark the voice target config as pending if we actually modified it
				// `erase()` will return `0` if it didn't remove anything or `1` if it did
				if (targets.channels.erase(targetChannel))
				{
					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
			else
			{
				InvalidTargetIdWarning("MUMBLE_REMOVE_VOICE_TARGET_CHANNEL");
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_REMOVE_VOICE_TARGET_PLAYER", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			auto playerId = context.GetArgument<int>(1);

			if (IsVoiceTargetIdValid(id))
			{
				if (auto targetName = getMumbleName(playerId))
				{
					auto& targets = vtConfigs[id];

					// we only want to mark the voice target config as pending if we actually modified it
					// `erase()` will return `0` if it didn't remove anything or `1` if it did
					if (targets.users.erase(*targetName))
					{
						g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
					}
				}
			}
			else
			{
				InvalidTargetIdWarning("MUMBLE_REMOVE_VOICE_TARGET_PLAYER");
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_REMOVE_VOICE_TARGET_PLAYER_BY_SERVER_ID", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			auto serverId = context.GetArgument<int>(1);

			if (IsVoiceTargetIdValid(id))
			{
				std::string targetName = g_mumbleClient->GetPlayerNameFromServerId(serverId);

				// if the player doesn't exist then we don't want to update targetting 
				if (targetName.empty())
				{
					return;
				}
				
				auto& targets = vtConfigs[id];

				if (targets.users.erase(targetName))
				{
					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
			else
			{
				InvalidTargetIdWarning("MUMBLE_REMOVE_VOICE_TARGET_PLAYER_BY_SERVER_ID");
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_VOICE_TARGET_CHANNELS", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);

			if (IsVoiceTargetIdValid(id))
			{
				auto& targets = vtConfigs[id];
				
				targets.channels.clear();

				g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
			}
			else
			{
				InvalidTargetIdWarning("MUMBLE_CLEAR_VOICE_TARGET_CHANNELS");
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_VOICE_TARGET_PLAYERS", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);

			if (IsVoiceTargetIdValid(id))
			{
				auto& targets = vtConfigs[id];

				targets.users.clear();

				g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
			}
			else
			{
				InvalidTargetIdWarning("MUMBLE_CLEAR_VOICE_TARGET_PLAYERS");
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_ADD_VOICE_CHANNEL_LISTEN", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto channel = context.GetArgument<int>(0);

			const std::string channelName =	GetMumbleChannel(channel); 
			if (g_mumbleClient->DoesChannelExist(channelName))
			{
				g_mumbleClient->AddListenChannel(channelName);
			}
			else
			{
				fx::scripting::Warningf("mumble", "MUMBLE_ADD_VOICE_CHANNEL_LISTEN: Tried to call native on a channel that didn't exist");
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_REMOVE_VOICE_CHANNEL_LISTEN", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto channel = context.GetArgument<int>(0);

			g_mumbleClient->RemoveListenChannel(GetMumbleChannel(channel));
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_ADD_VOICE_TARGET_CHANNEL", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			auto channel = context.GetArgument<int>(1);

			if (IsVoiceTargetIdValid(id))
			{
				auto& targets = vtConfigs[id];
				
				targets.channels.emplace(GetMumbleChannel(channel));
				
				g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
			}
			else
			{
				InvalidTargetIdWarning("MUMBLE_ADD_VOICE_TARGET_CHANNEL");
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_DOES_CHANNEL_EXIST", MakeMumbleNative([](fx::ScriptContext& context) {
			auto channel = context.GetArgument<int>(0);

			context.SetResult<bool>(g_mumbleClient->DoesChannelExist(GetMumbleChannel(channel)));
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_ADD_VOICE_TARGET_PLAYER", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			auto playerId = context.GetArgument<int>(1);

			if (IsVoiceTargetIdValid(id))
			{
				auto& targets = vtConfigs[id];
				if (auto name = getMumbleName(playerId))
				{
					targets.users.emplace(*name);
					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
			else
			{
				InvalidTargetIdWarning("MUMBLE_ADD_VOICE_TARGET_PLAYER");
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_ADD_VOICE_TARGET_PLAYER_BY_SERVER_ID", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			int serverId = context.GetArgument<int>(1);

			if (IsVoiceTargetIdValid(id))
			{
				std::string name = g_mumbleClient->GetPlayerNameFromServerId(serverId);

				if (!name.empty())
				{
					vtConfigs[id].users.emplace(name);
					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
			else
			{
				InvalidTargetIdWarning("MUMBLE_ADD_VOICE_TARGET_PLAYER_BY_SERVER_ID");
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_VOICE_TARGET", MakeMumbleNative([](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);

			// We can set our voice target to 0..31 here (and only here!)
			if (id >= 0 && id < 31)
			{
				g_mumbleClient->SetVoiceTarget(id);
			}
			else
			{
				fx::scripting::Warningf("mumble", "Invalid voice target id %d", id);
			}
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_GET_VOICE_CHANNEL_FROM_SERVER_ID", MakeMumbleNative([](fx::ScriptContext& context)
		{
			int serverId = context.GetArgument<int>(0);
			int channelId = -1;

			auto channelName = g_mumbleClient->GetVoiceChannelFromServerId(serverId);

			if (!channelName.empty())
			{
				if (channelName.find("Game Channel ") == 0)
				{
					channelId = std::stoi(channelName.substr(13));
				}
				else if (channelName == "Root")
				{
					channelId = 0;
				}
			}

			context.SetResult<int>(channelId);
		}));

		// MakeMumbleNative will return false automatically if we're not connected.
		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_IS_CONNECTED", MakeMumbleNative([](fx::ScriptContext& context)
		{
			context.SetResult<bool>(true);
		}));
		
		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_SERVER_ADDRESS", [](fx::ScriptContext& context)
		{
			auto address = context.GetArgument<const char*>(0);
			int port = context.GetArgument<int>(1);
			
			// If we set our address to an empty and our port is -1 we should reset our override
			if (address == "" && port == -1)
			{
				g_mumble.overridePeer = {};
				Mumble_Disconnect(true);
				return;
			}

			auto formattedAddress = fmt::sprintf("%s:%d", address, port);
			boost::optional<net::PeerAddress> overridePeer = net::PeerAddress::FromString(formattedAddress, port);

			if (overridePeer)
			{
				g_mumble.overridePeer = overridePeer;

				Mumble_Disconnect(true);
			}
			else
			{
				throw std::exception(va("Couldn't resolve Mumble server address %s.", formattedAddress));
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_AUDIO_INPUT_DISTANCE", [](fx::ScriptContext& context)
		{
			float dist = context.GetArgument<float>(0);

			g_mumbleClient->SetAudioInputDistance(dist);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_AUDIO_OUTPUT_DISTANCE", [](fx::ScriptContext& context)
		{
			float dist = context.GetArgument<float>(0);

			g_mumbleClient->SetAudioOutputDistance(dist);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_VOICE_CHANNEL", MakeMumbleNative([](fx::ScriptContext& context)
		{
			g_mumbleClient->SetChannel("Root");
		}));

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_VOICE_CHANNEL", MakeMumbleNative([](fx::ScriptContext& context)
		{
			g_mumbleClient->SetChannel(GetMumbleChannel(context.GetArgument<int>(0)));
		}));

		scrBindGlobal("GET_AUDIOCONTEXT_FOR_CLIENT", getAudioContext);
		scrBindGlobal("GET_AUDIOCONTEXT_FOR_SERVERID", getAudioContextByServerId);

		fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_TALKING_OVERRIDE", [](fx::ScriptContext& context)
		{
			int playerId = context.GetArgument<int>(0);
			int state = context.GetArgument<bool>(1);

			if (playerId < o_talkers.size() && playerId >= 0)
			{
				if (FxNativeInvoke::Invoke<bool>(isPlayerActive, playerId))
				{
					if (state)
					{
						o_talkers.set(playerId);
					}
					else
					{
						o_talkers.reset(playerId);
					}
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_GET_PLAYER_VOLUME_FROM_SERVER_ID", [](fx::ScriptContext& context)
		{
			int serverId = context.GetArgument<int>(0);
			auto volume = g_mumbleClient->GetPlayerVolumeFromServerId(serverId);

			context.SetResult(volume);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_PLAYER_VOLUME_FROM_SERVER_ID", [](fx::ScriptContext& context)
		{
			int serverId = context.GetArgument<int>(0);
			float volume = context.GetArgument<float>(1);

			if (volume < 0.0f || volume > 1.0f)
			{
				return;
			}

			g_mumbleClient->SetPlayerVolumeFromServerId(serverId, volume);
		});

#ifdef GTA_FIVE
		static auto origIsTalking = fx::ScriptEngine::GetNativeHandler(0x031E11F3D447647E);

		fx::ScriptEngine::RegisterNativeHandler(0x031E11F3D447647E, [=](fx::ScriptContext& context)
		{
			if (!IsMumbleConnected())
			{
				origIsTalking(context);
				return;
			}

			int playerId = context.GetArgument<int>(0);

			if (playerId >= g_talkers.size() || playerId < 0)
			{
				context.SetResult(0);
				return;
			}

			context.SetResult(g_talkers.test(playerId));
		});

		auto origSetChannel = fx::ScriptEngine::GetNativeHandler(0xEF6212C2EFEF1A23);
		auto origClearChannel = fx::ScriptEngine::GetNativeHandler(0xE036A705F989E049);

		fx::ScriptEngine::RegisterNativeHandler(0xEF6212C2EFEF1A23, [=](fx::ScriptContext& context)
		{
			origSetChannel(context);

			if (IsMumbleConnected())
			{
				g_mumbleClient->SetChannel(GetMumbleChannel(context.GetArgument<int>(0)));
			}
		});

		fx::ScriptEngine::RegisterNativeHandler(0xE036A705F989E049, [=](fx::ScriptContext& context)
		{
			origClearChannel(context);

			if (IsMumbleConnected())
			{
				g_mumbleClient->SetChannel("Root");
			}
		});

		auto origSetProximity = fx::ScriptEngine::GetNativeHandler(0xCBF12D65F95AD686);
		auto origGetProximity = fx::ScriptEngine::GetNativeHandler(0x84F0F13120B4E098);

		fx::ScriptEngine::RegisterNativeHandler(0xCBF12D65F95AD686, [=](fx::ScriptContext& context)
		{
			origSetProximity(context);

			float dist = context.GetArgument<float>(0);

			g_mumbleClient->SetAudioDistance(dist);
		});

		fx::ScriptEngine::RegisterNativeHandler(0x84F0F13120B4E098, [=](fx::ScriptContext& context)
		{
			origGetProximity(context);

			float proximity = g_mumbleClient->GetAudioDistance();

			context.SetResult<float>(proximity);
		});

		auto origSetVoiceActive = fx::ScriptEngine::GetNativeHandler(0xBABEC9E69A91C57B);

		fx::ScriptEngine::RegisterNativeHandler(0xBABEC9E69A91C57B, [origSetVoiceActive](fx::ScriptContext& context)
		{
			origSetVoiceActive(context);

			g_voiceActiveByScript = context.GetArgument<bool>(0);
		});
#endif
	});

	MH_Initialize();

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 74 26 66 0F 6E 35")), _isAnyoneTalking, (void**)&g_origIsAnyoneTalking);
	MH_CreateHook(hook::get_call(hook::get_pattern("48 8B D0 E8 ? ? ? ? 40 8A F0 8B 8F", 3)), _isPlayerTalking, (void**)&g_origIsPlayerTalking);
	MH_CreateHook(hook::get_call(hook::get_pattern("89 44 24 58 0F 29 44 24 40 E8", 9)), _filterVoiceChatConfig, (void**)&g_origInitVoiceEngine);
	MH_CreateHook(hook::get_pattern("48 8B F8 48 85 C0 74 33 48 83 C3 30", -0x19), _getLocalAudioLevel, (void**)&g_origGetLocalAudioLevel);
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 74 ? 48 8D 4D ? E8 ? ? ? ? 84 C0 75 ? 48 8D 55 ? 49 8B CF")), _getPlayerHasHeadset, (void**)&g_origGetPlayerHasHeadset);
#elif IS_RDR3
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 74 13 F3 0F 10 35")), _isAnyoneTalking, (void**)&g_origIsAnyoneTalking);
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 8A D8 45 84 ED 75 08")), _isPlayerTalking, (void**)&g_origIsPlayerTalking);
	MH_CreateHook(hook::get_call(hook::get_pattern("8B 83 ? ? ? ? F2 0F 10 8B ? ? ? ? 48", 32)), _filterVoiceChatConfig, (void**)&g_origInitVoiceEngine);
	MH_CreateHook(hook::get_pattern("48 8B F8 48 85 C0 74 33 48 83 C3 30", -0x19), _getLocalAudioLevel, (void**)&g_origGetLocalAudioLevel);
	MH_CreateHook(hook::get_pattern("80 78 19 00 B9", -0x20), _getPlayerHasHeadset, (void**)&g_origGetPlayerHasHeadset);
#endif

	MH_EnableHook(MH_ALL_HOOKS);
});
