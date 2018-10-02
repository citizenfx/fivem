#include "StdInc.h"
#include "Hooking.h"

#include <GameInit.h>
#include <ICoreGameInit.h>
#include <nutsnbolts.h>

#include <NetLibrary.h>
#include <MumbleClient.h>

#include <json.hpp>

using json = nlohmann::json;

static NetLibrary* g_netLibrary;

static uint32_t* g_preferenceArray;

// 1290
// #TODO1365
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

static hook::cdecl_stub<void()> _initVoiceChatConfig([]()
{
	return hook::get_pattern("89 44 24 58 0F 29 44 24 40 E8", -0x12E);
});

static bool g_mumbleAllowed;

void MumbleVoice_BindNetLibrary(NetLibrary* library)
{
	g_netLibrary = library;

	g_netLibrary->OnConnectOKReceived.Connect([](NetAddress addr)
	{
		g_mumbleAllowed = false;

		/*Instance<HttpClient>::Get()->DoGetRequest(fmt::sprintf("http://%s:%d/info.json", addr.GetAddress(), addr.GetPort()), [=](bool success, const char* data, size_t size)
		{
			if (success)
			{
				try
				{
					json info = json::parse(data, data + size);

					if (info.is_object() && info["vars"].is_object())
					{
						auto val = info["vars"].value("sv_licenseKeyToken", "");

						if (!val.empty())
						{
							Instance<HttpClient>::Get()->DoGetRequest(fmt::sprintf("https://policy-live.fivem.net/api/policy/%s/mumble_voice_prerelease", val), [=](bool success, const char* data, size_t size)
							{
								if (success)
								{
#ifndef _DEBUG
									if (std::string(data, size).find("yes") != std::string::npos)
#endif
									{
										trace("Server policy - Mumble allowed: yes\n");

										g_mumbleAllowed = true;
									}
								}
							});
						}
					}
				}
				catch (std::exception& e)
				{
					trace("Server policy - Mumble check failed for %s\n", e.what());
				}
			}
		});*/
	});
}

#include <rapidjson/document.h>

void Policy_BindNetLibrary(NetLibrary* library)
{
	g_netLibrary = library;

	g_netLibrary->OnConnectOKReceived.Connect([](NetAddress addr)
	{
		Instance<ICoreGameInit>::Get()->SetData("policy", "");

		Instance<HttpClient>::Get()->DoGetRequest(fmt::sprintf("http://%s:%d/info.json", addr.GetAddress(), addr.GetPort()), [=](bool success, const char* data, size_t size)
		{
			if (success)
			{
				try
				{
					json info = json::parse(data, data + size);

					if (info.is_object() && info["vars"].is_object())
					{
						auto val = info["vars"].value("sv_licenseKeyToken", "");

						if (!val.empty())
						{
							Instance<HttpClient>::Get()->DoGetRequest(fmt::sprintf("https://policy-live.fivem.net/api/policy/%s", val), [=](bool success, const char* data, size_t size)
							{
								if (success)
								{
									rapidjson::Document doc;
									doc.Parse(data, size);

									std::stringstream policyStr;

									if (!doc.HasParseError() && doc.IsArray())
									{
										for (auto it = doc.Begin(); it != doc.End(); it++)
										{
											if (it->IsString())
											{
												policyStr << "[" << it->GetString() << "]";
											}
										}
									}

									std::string policy = policyStr.str();
									trace("Policy is %s\n", policy);

									Instance<ICoreGameInit>::Get()->SetData("policy", policy);
								}
							});
						}
					}
				}
				catch (std::exception& e)
				{
					trace("Server policy - get failed for %s\n", e.what());
				}
			}
		});
	});
}

static fwRefContainer<IMumbleClient> g_mumbleClient;

static struct  
{
	volatile bool connecting;
	volatile bool connected;
	volatile bool errored;

	volatile MumbleConnectionInfo* connectionInfo;

	concurrency::concurrent_queue<std::function<void()>> mainFrameExecQueue;
} g_mumble;

static void Mumble_Connect()
{
	g_mumble.errored = false;
	g_mumble.connecting = true;

	_initVoiceChatConfig();

	g_mumbleClient->ConnectAsync(g_netLibrary->GetCurrentPeer(), fmt::sprintf("[%d] %s", g_netLibrary->GetServerNetID(), g_netLibrary->GetPlayerName())).then([](concurrency::task<MumbleConnectionInfo*> task)
	{
		try
		{
			auto info = task.get();

			g_mumble.connectionInfo = g_mumbleClient->GetConnectionInfo();

			g_mumble.connected = true;

			g_mumble.mainFrameExecQueue.push([]()
			{
				_initVoiceChatConfig();
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

static void Mumble_Disconnect()
{
	g_mumble.connected = false;
	g_mumble.errored = false;
	g_mumble.connecting = false;

	g_mumbleClient->DisconnectAsync().then([=]()
	{
	});

	_initVoiceChatConfig();
}

static float* g_cameraFront;
static float* g_cameraTop;
static float* g_cameraPos;
static float* g_actorPos;

#include <mmsystem.h>
#include <dsound.h>
#include <ScriptEngine.h>

#pragma comment(lib, "dsound.lib")

static void Mumble_RunFrame()
{
	if (!Instance<ICoreGameInit>::Get()->HasVariable("networkInited"))
	{
		return;
	}

	if (g_netLibrary->GetConnectionState() != NetLibrary::CS_ACTIVE)
	{
		return;
	}

	bool shouldConnect = g_preferenceArray[PREF_VOICE_ENABLE] && g_mumbleAllowed;

	if (!g_mumble.connected || (g_mumble.connectionInfo && !g_mumble.connectionInfo->isConnected))
	{
		if (shouldConnect && !g_mumble.connecting && !g_mumble.errored)
		{
			Mumble_Connect();
		}
	}
	else
	{
		if (!shouldConnect)
		{
			Mumble_Disconnect();
		}
	}

	MumbleActivationMode activationMode;

	if (g_preferenceArray[PREF_VOICE_TALK_ENABLED])
	{
		if (g_preferenceArray[PREF_VOICE_CHAT_MODE] == 1)
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

	g_mumbleClient->SetOutputVolume(g_preferenceArray[PREF_VOICE_OUTPUT_VOLUME] * 0.1f);

	float cameraFront[3];
	float cameraTop[3];
	float cameraPos[3];
	float actorPos[3];

	cameraFront[0] = g_cameraFront[0];
	cameraFront[1] = g_cameraFront[2];
	cameraFront[2] = g_cameraFront[1];

	cameraTop[0] = g_cameraTop[0];
	cameraTop[1] = g_cameraTop[2];
	cameraTop[2] = g_cameraTop[1];

	cameraPos[0] = g_cameraPos[0];
	cameraPos[1] = g_cameraPos[2];
	cameraPos[2] = g_cameraPos[1];

	actorPos[0] = g_actorPos[0];
	actorPos[1] = g_actorPos[2];
	actorPos[2] = g_actorPos[1];

	g_mumbleClient->SetListenerMatrix(actorPos, cameraFront, cameraTop);
	g_mumbleClient->SetActorPosition(actorPos);

	auto likelihoodValue = g_preferenceArray[PREF_VOICE_MIC_SENSITIVITY];

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

	// handle PTT
	auto isControlPressed = fx::ScriptEngine::GetNativeHandler(0xF3A21BCD95725A4A);
	fx::ScriptContextBuffer cxt;

	cxt.Push(0);
	cxt.Push(249); // INPUT_PUSH_TO_TALK

	(*isControlPressed)(cxt);

	g_mumbleClient->SetPTTButtonState(cxt.GetResult<bool>());

	// handle device changes
	static int curInDevice = -1;
	static int curOutDevice = -1;

	int inDevice = g_preferenceArray[PREF_VOICE_INPUT_DEVICE];
	int outDevice = g_preferenceArray[PREF_VOICE_OUTPUT_DEVICE];

	struct EnumCtx
	{
		int target;
		int cur;
		GUID guid;
		std::string guidStr;
	} enumCtx;

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

	if (inDevice != curInDevice)
	{
		trace(__FUNCTION__ ": capture device changed in GTA code, changing to index %d (last %d)\n", inDevice, curInDevice);

		enumCtx.cur = -1;
		enumCtx.target = inDevice;
		DirectSoundCaptureEnumerateW(enumCb, &enumCtx);

		trace(__FUNCTION__ ": this device index is GUID %s\n", enumCtx.guidStr);

		g_mumbleClient->SetInputDevice(enumCtx.guidStr);

		curInDevice = inDevice;

		trace(__FUNCTION__ ": device should've changed by now!\n");
	}

	if (outDevice != curOutDevice)
	{
		enumCtx.cur = -1;
		enumCtx.target = outDevice;
		DirectSoundEnumerateW(enumCb, &enumCtx);

		g_mumbleClient->SetOutputDevice(enumCtx.guidStr);

		curOutDevice = outDevice;
	}
}

static std::bitset<256> g_talkers;
static std::bitset<256> o_talkers;

static InitFunction initFunction([]()
{
	g_mumbleClient = CreateMumbleClient();
	g_mumbleClient->Initialize();

	OnMainGameFrame.Connect([=]()
	{
		Mumble_RunFrame();
	});

	OnKillNetworkDone.Connect([]()
	{
		g_mumbleClient->SetAudioDistance(FLT_MAX);

		Mumble_Disconnect();
		o_talkers.reset();
	});
});

static bool(*g_origIsAnyoneTalking)(void*);

static bool _isAnyoneTalking(void* mgr)
{
	return (g_origIsAnyoneTalking(mgr) || g_mumbleClient->IsAnyoneTalking());
}

static bool(*g_origIsPlayerTalking)(void*, void*);

static bool _isPlayerTalking(void* mgr, char* playerData)
{
	if (g_origIsPlayerTalking(mgr, playerData))
	{
		return true;
	}

	// 1290
	// #TODO1365
	auto playerInfo = playerData - 32 - 48 - 16;

	// get the ped
	auto ped = *(char**)(playerInfo + 456);
	
	if (ped)
	{
		auto netObj = *(uint8_t**)(ped + 208);

		if (netObj)
		{
			// actually: netobj owner
			auto index = netObj[73];

			if (g_talkers.test(index) || o_talkers.test(index))
			{
				return true;
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
	// disable voice if mumble is used
	if (g_mumble.connected)
	{
		*config = 0;
	}

	g_origInitVoiceEngine(engine, config);
}

#include <scrEngine.h>
#include <MinHook.h>

class FxNativeInvoke
{
private:
	static inline void Invoke(fx::ScriptContext& cxt, const boost::optional<fx::TNativeHandler>& handler)
	{
		(*handler)(cxt);
	}

public:

	template<typename R, typename... Args>
	static inline R Invoke(const boost::optional<fx::TNativeHandler>& handler, Args... args)
	{
		fx::ScriptContextBuffer cxt;

		pass{ ([&]()
		{
			cxt.Push(args);
		}(), 1)... };

		Invoke(cxt, handler);

		return cxt.GetResult<R>();
	}
};

static HookFunction hookFunction([]()
{
	g_preferenceArray = hook::get_address<uint32_t*>(hook::get_pattern("48 8D 15 ? ? ? ? 8D 43 01 83 F8 02 77 2D", 3));

	g_cameraFront = hook::get_address<float*>(hook::get_pattern("40 F6 C6 02 75 25 F3 41 0F 10 44 24 08 4C 8D 0D", 16));
	g_cameraTop = hook::get_address<float*>(hook::get_pattern("48 69 C0 90 04 00 00 0F C6 C0 00 0F", -11)) - 4;
	g_cameraPos = hook::get_address<float*>(hook::get_pattern("87 AA 00 00 00 66 0F 6E D0 F3 0F 5C 05", 13));

	g_actorPos = hook::get_address<float*>(hook::get_pattern("BB 00 00 40 00 48 89 7D F8 89 1D", -4)) + 12;

	rage::scrEngine::OnScriptInit.Connect([]()
	{
		auto origIsTalking = fx::ScriptEngine::GetNativeHandler(0x031E11F3D447647E);
		auto getPlayerName = fx::ScriptEngine::GetNativeHandler(0x6D0DE6A7B5DA71F8);
		auto isPlayerActive = fx::ScriptEngine::GetNativeHandler(0xB8DFD30D6973E135);

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

			static auto getServerId = fx::ScriptEngine::GetNativeHandler(HashString("GET_PLAYER_SERVER_ID"));

			std::vector<std::string> talkers;
			g_mumbleClient->GetTalkers(&talkers);

			std::set<std::string> talkerSet(talkers.begin(), talkers.end());

			g_talkers.reset();

			for (int i = 0; i < 256; i++)
			{
				if (FxNativeInvoke::Invoke<bool>(isPlayerActive, i))
				{
					std::string name = fmt::sprintf("[%d] %s",
						FxNativeInvoke::Invoke<int>(getServerId, i),
						FxNativeInvoke::Invoke<const char*>(getPlayerName, i));

					if (talkerSet.find(name) != talkerSet.end())
					{
						g_talkers.set(i);
					}
				}
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_TALKING_OVERRIDE", [](fx::ScriptContext& context)
		{
			auto isPlayerActive = fx::ScriptEngine::GetNativeHandler(0xB8DFD30D6973E135);

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

		fx::ScriptEngine::RegisterNativeHandler(0x031E11F3D447647E, [=](fx::ScriptContext& context)
		{
			if (!g_mumble.connected)
			{
				(*origIsTalking)(context);
				return;
			}

			int playerId = context.GetArgument<int>(0);

			if (playerId > g_talkers.size() || playerId < 0)
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

		fx::ScriptEngine::RegisterNativeHandler(0xCBF12D65F95AD686, [=](fx::ScriptContext& context)
		{
			(*origSetProximity)(context);

			if (g_mumble.connected)
			{
				float dist = context.GetArgument<float>(0);

				g_mumbleClient->SetAudioDistance(dist == 0.0f ? FLT_MAX : dist);
			}
		});
	});

	MH_Initialize();
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 74 26 66 0F 6E 35")), _isAnyoneTalking, (void**)&g_origIsAnyoneTalking);
	MH_CreateHook(hook::get_call(hook::get_pattern("48 8B D0 E8 ? ? ? ? 40 8A F0 8B 8F", 3)), _isPlayerTalking, (void**)&g_origIsPlayerTalking);
	MH_CreateHook(hook::get_call(hook::get_pattern("89 44 24 58 0F 29 44 24 40 E8", 9)), _filterVoiceChatConfig, (void**)&g_origInitVoiceEngine);
	MH_CreateHook(hook::get_pattern("48 8B F8 48 85 C0 74 33 48 83 C3 30", -0x19), _getLocalAudioLevel, (void**)&g_origGetLocalAudioLevel);
	MH_CreateHook(hook::get_pattern("80 78 05 00 B9", -0x1B), _getPlayerHasHeadset, (void**)&g_origGetPlayerHasHeadset);
	MH_EnableHook(MH_ALL_HOOKS);
});
