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

#if __has_include(<GameInput.h>)
#include <GameInput.h>
#endif

#if __has_include(<GameAudioState.h>)
#include <GameAudioState.h>
#endif

using json = nlohmann::json;

static NetLibrary* g_netLibrary;

#ifdef GTA_FIVE
static hook::cdecl_stub<void()> _initVoiceChatConfig([]()
{
	return hook::get_pattern("89 44 24 58 0F 29 44 24 40 E8", -0x12E);
});
#elif IS_RDR3
static hook::cdecl_stub<void(void*)> _initVoiceChatConfig([]()
{
	return hook::get_pattern("8B 83 ? ? ? ? F2 0F 10 8B ? ? ? ? 48", (xbr::IsGameBuildOrGreater<1436>()) ? -0xDD : -0x81);
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

// 1290
// #TODO1365
// #TODO1493
// #TODO1604
// Outdated as of b2944, we're mapping indexes now.
enum PrefEnum
{
	PREF_VOICE_ENABLE = 0x60,
	PREF_VOICE_OUTPUT_DEVICE = 0x61,
	PREF_VOICE_OUTPUT_VOLUME = 0x62,
	PREF_VOICE_SOUND_VOLUME = 0x63,
	PREF_VOICE_MUSIC_VOLUME = 0x64,
	PREF_VOICE_TALK_ENABLED = 0x65,
	PREF_VOICE_FEEDBACK = 0x66,
	PREF_VOICE_INPUT_DEVICE = 0x67,
	PREF_VOICE_CHAT_MODE = 0x68,
	PREF_VOICE_MIC_VOLUME = 0x69,
	PREF_VOICE_MIC_SENSITIVITY = 0x6A
};

static int MapPrefsEnum(int index)
{
	if (index >= 2 && xbr::IsGameBuildOrGreater<2944>())
	{
		index++;
	}

	return index;
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

	if (!g_mumble.connected || (g_mumble.connectionInfo && !g_mumble.connectionInfo->isConnected))
	{
		if (Mumble_ShouldConnect() && !g_mumble.connecting && !g_mumble.errored)
		{
			if (GetTickCount64() > g_mumble.nextConnectAt)
			{
				Mumble_Connect();

				g_mumble.nextConnectDelay *= 2;

				if (g_mumble.nextConnectDelay > 30 * 1000)
				{
					g_mumble.nextConnectDelay = 30 * 1000;
				}

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

static bool _isPlayerTalking(void* mgr, char* playerData)
{
	if (g_origIsPlayerTalking(mgr, playerData))
	{
		return true;
	}

#ifdef GTA_FIVE
	// 1290
	// #TODO1365
	// #TODO1493
	// #TODO1604
	auto playerInfo = playerData - 32 - 48 - 16 - (xbr::IsGameBuildOrGreater<2060>() ? 8 : 0);

	// preemptive check for invalid players (FIVEM-CLIENT-1604-TQKV) with uncertain server changes
	if ((uintptr_t)playerInfo < 0xFFFF)
	{
		return false;
	}

	// get the ped
	auto ped = *(char**)(playerInfo + 456);
#elif IS_RDR3
	// rlGamerInfo = playerData - 200, rlGamerInfo + 896 = CPed
	auto ped = *(char**)(playerData + 696);
#endif

	if (ped)
	{
#ifdef GTA_FIVE
		auto netObj = *(rage::netObject**)(ped + 208);
#elif IS_RDR3
		auto netObj = *(rage::netObject**)(ped + 224);
#endif

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
	if (g_mumble.connected)
	{
		return true;
	}

	return g_origGetPlayerHasHeadset(mgr, plr);
}

static float(*g_origGetLocalAudioLevel)(void* mgr, int localIdx);

static float _getLocalAudioLevel(void* mgr, int localIdx)
{
	float mumbleLevel = (g_mumble.connected) ? g_mumbleClient->GetInputAudioLevel() : 0.0f;

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
	if (g_mumble.connected)
	{
		*config = 0;
	}

	g_origInitVoiceEngine(engine, config);
}

#include <LabSound/extended/LabSound.h>

static boost::optional<fx::TNativeHandler> getPlayerName;
static boost::optional<fx::TNativeHandler> getServerId;

static std::shared_ptr<lab::AudioContext> getAudioContext(int playerId)
{
	if (!g_mumble.connected)
	{
		return {};
	}

	std::string name = fmt::sprintf("[%d] %s",
		FxNativeInvoke::Invoke<int>(getServerId, playerId),
		FxNativeInvoke::Invoke<const char*>(getPlayerName, playerId));
	return g_mumbleClient->GetAudioContext(name);
}

static std::shared_ptr<lab::AudioContext> getAudioContextByServerId(int serverId)
{
	if (!g_mumble.connected)
	{
		return {};
	}

	std::string name = ToNarrow(g_mumbleClient->GetPlayerNameFromServerId(serverId));
	return g_mumbleClient->GetAudioContext(name);
}

std::wstring getMumbleName(int playerId)
{
	return ToWide(fmt::sprintf("[%d] %s",
		FxNativeInvoke::Invoke<int>(getServerId, playerId),
		FxNativeInvoke::Invoke<const char*>(getPlayerName, playerId)));
}

#include <scrBind.h>

static HookFunction hookFunction([]()
{
#ifdef GTA_FIVE
	g_preferenceArray = hook::get_address<uint32_t*>(hook::get_pattern("48 8D 15 ? ? ? ? 8D 43 01 83 F8 02 77 2D", 3));
	g_viewportGame = hook::get_address<CViewportGame**>(hook::get_pattern("33 C0 48 39 05 ? ? ? ? 74 2E 48 8B 0D ? ? ? ? 48 85 C9 74 22", 5));
	g_actorPos = hook::get_address<float*>(hook::get_pattern("BB 00 00 40 00 48 89 7D F8 89 1D", -4)) + 12;
#elif IS_RDR3
	g_viewportGame = hook::get_address<CViewportGame**>(hook::get_pattern("0F 2F F0 76 ? 4C 8B 35", 8));

	if (xbr::IsGameBuildOrGreater<1436>())
	{
		g_actorPos = hook::get_address<float*>(hook::get_pattern("45 33 C9 48 89 5D E0 8D 53 01", 63)) + 16;
	}
	else
	{
		g_actorPos = hook::get_address<float*>(hook::get_pattern("8B C2 48 03 C0 41 8D 49 FF 48 03 C9", -4)) + 16;
	}

	{
		auto location = hook::get_pattern<char>("75 0D 8B C8 E8 ? ? ? ? 84 C0 B0 01 75 03");
		auto prefsOffset = *(uint32_t*)(location + (xbr::IsGameBuildOrGreater<1436>() ? 40 : 48));

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

			if (!g_mumble.connected)
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

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_TALKER_PROXIMITY", [](fx::ScriptContext& context)
		{
			float proximity = context.GetArgument<float>(0);

			if (g_mumble.connected)
			{
				g_mumbleClient->SetAudioDistance(proximity);
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_GET_TALKER_PROXIMITY", [](fx::ScriptContext& context)
		{
			float proximity = (g_mumble.connected) ? g_mumbleClient->GetAudioDistance() : 0.0f;

			context.SetResult<float>(proximity);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_ACTIVE", [](fx::ScriptContext& context)
		{
			g_voiceActiveByScript = context.GetArgument<bool>(0);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_IS_ACTIVE", [](fx::ScriptContext& context)
		{
			context.SetResult<bool>(g_voiceActiveByScript);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_IS_PLAYER_TALKING", [](fx::ScriptContext& context)
		{
			int playerId = context.GetArgument<int>(0);
			bool isTalking = false;

			if (g_mumble.connected)
			{
				if (playerId >= 0 && playerId < g_talkers.size())
				{
					isTalking = g_talkers.test(playerId);
				}
			}

			context.SetResult(isTalking);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_VOLUME_OVERRIDE", [](fx::ScriptContext& context)
		{
			int playerId = context.GetArgument<int>(0);
			float volume = context.GetArgument<float>(1);

			if (g_mumble.connected)
			{
				std::wstring name = getMumbleName(playerId);

				g_mumbleClient->SetClientVolumeOverride(name, volume);
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_VOLUME_OVERRIDE_BY_SERVER_ID", [](fx::ScriptContext& context)
		{
			int serverId = context.GetArgument<int>(0);
			float volume = context.GetArgument<float>(1);

			if (g_mumble.connected)
			{
				g_mumbleClient->SetClientVolumeOverrideByServerId(serverId, volume);
			}
		});

		static VoiceTargetConfig vtConfigs[31];

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_VOICE_TARGET", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);

			if (id >= 0 && id < 31)
			{
				if (g_mumble.connected)
				{
					vtConfigs[id] = {};
					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_REMOVE_VOICE_TARGET_CHANNEL", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			auto channel = context.GetArgument<int>(1);

			if (id >= 0 && id < 31)
			{
				if (g_mumble.connected)
				{
					auto targetChannel = fmt::sprintf("Game Channel %d", channel);
					auto& targets = vtConfigs[id].targets;
					targets.remove_if([targetChannel](auto& target)
					{
						return target.channel == targetChannel;
					});

					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_REMOVE_VOICE_TARGET_PLAYER", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			auto playerId = context.GetArgument<int>(1);

			if (id >= 0 && id < 31)
			{
				if (g_mumble.connected)
				{
					std::wstring targetName = getMumbleName(playerId);

					auto& targets = vtConfigs[id].targets;
					targets.remove_if([targetName](auto& target)
					{
						return target.users.size() > 0 && std::find(target.users.begin(), target.users.end(), targetName) != target.users.end();
					});

					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_REMOVE_VOICE_TARGET_PLAYER_BY_SERVER_ID", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			auto serverId = context.GetArgument<int>(1);

			if (id >= 0 && id < 31)
			{
				if (g_mumble.connected)
				{
					VoiceTargetConfig::Target ch;
					std::wstring targetName = g_mumbleClient->GetPlayerNameFromServerId(serverId);

					if (!targetName.empty())
					{
						auto& targets = vtConfigs[id].targets;
						targets.remove_if([targetName](auto& target)
						{
							return target.users.size() > 0 && std::find(target.users.begin(), target.users.end(), targetName) != target.users.end();
						});
					}
				}

				g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_VOICE_TARGET_CHANNELS", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);

			if (id >= 0 && id < 31)
			{
				if (g_mumble.connected)
				{
					auto& targets = vtConfigs[id].targets;
					targets.remove_if([](auto& target)
					{
						return !target.channel.empty();
					});

					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_VOICE_TARGET_PLAYERS", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);

			if (id >= 0 && id < 31)
			{
				if (g_mumble.connected)
				{
					auto& targets = vtConfigs[id].targets;
					targets.remove_if([](auto& target)
					{
						return target.users.size() > 0;
					});

					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_ADD_VOICE_CHANNEL_LISTEN", [](fx::ScriptContext& context)
		{
			auto channel = context.GetArgument<int>(0);

			if (g_mumble.connected)
			{
				g_mumbleClient->AddListenChannel(fmt::sprintf("Game Channel %d", channel));
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_REMOVE_VOICE_CHANNEL_LISTEN", [](fx::ScriptContext& context)
		{
			auto channel = context.GetArgument<int>(0);

			if (g_mumble.connected)
			{
				g_mumbleClient->RemoveListenChannel(fmt::sprintf("Game Channel %d", channel));
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_ADD_VOICE_TARGET_CHANNEL", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			auto channel = context.GetArgument<int>(1);

			if (id >= 0 && id < 31)
			{
				if (g_mumble.connected)
				{
					VoiceTargetConfig::Target ch;
					ch.channel = fmt::sprintf("Game Channel %d", channel);

					vtConfigs[id].targets.push_back(ch);
					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_ADD_VOICE_TARGET_PLAYER", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			auto playerId = context.GetArgument<int>(1);

			if (id >= 0 && id < 31)
			{
				if (g_mumble.connected)
				{
					VoiceTargetConfig::Target ch;
					std::wstring name = getMumbleName(playerId);

					ch.users.push_back(name);

					vtConfigs[id].targets.push_back(ch);
					g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_ADD_VOICE_TARGET_PLAYER_BY_SERVER_ID", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);
			int serverId = context.GetArgument<int>(1);

			if (id >= 0 && id < 31)
			{
				if (g_mumble.connected)
				{
					VoiceTargetConfig::Target ch;
					std::wstring name = g_mumbleClient->GetPlayerNameFromServerId(serverId);

					if (!name.empty())
					{
						ch.users.push_back(name);

						vtConfigs[id].targets.push_back(ch);
						g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
					}
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_VOICE_TARGET", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);

			if (id >= 0 && id < 31)
			{
				if (g_mumble.connected)
				{
					g_mumbleClient->SetVoiceTarget(id);
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_GET_VOICE_CHANNEL_FROM_SERVER_ID", [](fx::ScriptContext& context)
		{
			int serverId = context.GetArgument<int>(0);
			int channelId = -1;

			if (g_mumble.connected)
			{
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
			}

			context.SetResult<int>(channelId);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_IS_CONNECTED", [](fx::ScriptContext& context)
		{
			context.SetResult<bool>(g_mumble.connected ? true : false);
		});
		
		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_SERVER_ADDRESS", [](fx::ScriptContext& context)
		{
			auto address = context.GetArgument<const char*>(0);
			int port = context.GetArgument<int>(1);

			boost::optional<net::PeerAddress> overridePeer = net::PeerAddress::FromString(fmt::sprintf("%s:%d", address, port), port);

			if (overridePeer)
			{
				g_mumble.overridePeer = overridePeer;

				Mumble_Disconnect(true);
			}
			else
			{
				throw std::exception("Couldn't resolve Mumble server address.");
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

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_VOICE_CHANNEL", [](fx::ScriptContext& context)
		{
			if (g_mumble.connected)
			{
				g_mumbleClient->SetChannel("Root");
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_VOICE_CHANNEL", [](fx::ScriptContext& context)
		{
			if (g_mumble.connected)
			{
				g_mumbleClient->SetChannel(fmt::sprintf("Game Channel %d", context.GetArgument<int>(0)));
			}
		});

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

#ifdef GTA_FIVE
		static auto origIsTalking = fx::ScriptEngine::GetNativeHandler(0x031E11F3D447647E);

		fx::ScriptEngine::RegisterNativeHandler(0x031E11F3D447647E, [=](fx::ScriptContext& context)
		{
			if (!g_mumble.connected)
			{
				(*origIsTalking)(context);
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
			(*origSetChannel)(context);

			if (g_mumble.connected)
			{
				g_mumbleClient->SetChannel(fmt::sprintf("Game Channel %d", context.GetArgument<int>(0)));
			}
		});

		fx::ScriptEngine::RegisterNativeHandler(0xE036A705F989E049, [=](fx::ScriptContext& context)
		{
			(*origClearChannel)(context);

			if (g_mumble.connected)
			{
				g_mumbleClient->SetChannel("Root");
			}
		});

		auto origSetProximity = fx::ScriptEngine::GetNativeHandler(0xCBF12D65F95AD686);
		auto origGetProximity = fx::ScriptEngine::GetNativeHandler(0x84F0F13120B4E098);

		fx::ScriptEngine::RegisterNativeHandler(0xCBF12D65F95AD686, [=](fx::ScriptContext& context)
		{
			(*origSetProximity)(context);

			float dist = context.GetArgument<float>(0);

			g_mumbleClient->SetAudioDistance(dist);
		});

		fx::ScriptEngine::RegisterNativeHandler(0x84F0F13120B4E098, [=](fx::ScriptContext& context)
		{
			(*origGetProximity)(context);

			float proximity = g_mumbleClient->GetAudioDistance();

			context.SetResult<float>(proximity);
		});

		auto origSetVoiceActive = fx::ScriptEngine::GetNativeHandler(0xBABEC9E69A91C57B);

		fx::ScriptEngine::RegisterNativeHandler(0xBABEC9E69A91C57B, [origSetVoiceActive](fx::ScriptContext& context)
		{
			(*origSetVoiceActive)(context);

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
	MH_CreateHook(hook::get_pattern("80 78 05 00 B9", -0x1B), _getPlayerHasHeadset, (void**)&g_origGetPlayerHasHeadset);
#elif IS_RDR3
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 74 13 F3 0F 10 35")), _isAnyoneTalking, (void**)&g_origIsAnyoneTalking);
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 8A D8 45 84 ED 75 08")), _isPlayerTalking, (void**)&g_origIsPlayerTalking);
	MH_CreateHook(hook::get_call(hook::get_pattern("8B 83 ? ? ? ? F2 0F 10 8B ? ? ? ? 48", 32)), _filterVoiceChatConfig, (void**)&g_origInitVoiceEngine);
	MH_CreateHook(hook::get_pattern("48 8B F8 48 85 C0 74 33 48 83 C3 30", -0x19), _getLocalAudioLevel, (void**)&g_origGetLocalAudioLevel);
	MH_CreateHook(hook::get_pattern("80 78 19 00 B9", -0x20), _getPlayerHasHeadset, (void**)&g_origGetPlayerHasHeadset);
#endif

	MH_EnableHook(MH_ALL_HOOKS);
});
