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

using json = nlohmann::json;

static uint64_t* g_preference;

static NetLibrary* g_netLibrary;

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

static void Mumble_Connect()
{
	g_mumble.errored = false;
	g_mumble.connecting = true;

	

	g_mumbleClient->ConnectAsync(g_mumble.overridePeer ? *g_mumble.overridePeer : g_netLibrary->GetCurrentPeer(), fmt::sprintf("[%d] %s", g_netLibrary->GetServerNetID(), g_netLibrary->GetPlayerName())).then([](concurrency::task<MumbleConnectionInfo*> task)
	{
		try
		{
			auto info = task.get();

			g_mumble.connectionInfo = g_mumbleClient->GetConnectionInfo();

			g_mumble.connected = true;
			g_mumble.nextConnectDelay = 4 * 1000;

			g_mumble.mainFrameExecQueue.push([]()
			{

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
	g_mumble.connected = false;
	g_mumble.errored = false;
	g_mumble.connecting = false;
	g_mumble.nextConnectDelay = 4 * 1000;

	g_mumbleClient->DisconnectAsync().then([=]()
	{
		if (reconnect)
		{
			Mumble_Connect();
		}
	});

}



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

	auto voiceEnabled = hook::get_address<uint8_t>(*g_preference +0x2870)-164;
	auto microEnabled = hook::get_address<uint8_t>(*g_preference +0x2872)-166;
	auto microActivationMode =  hook::get_address<uint8_t>(*g_preference +0x2888)-188;
    bool shouldConnect = voiceEnabled;

	if (!g_mumble.connected || (g_mumble.connectionInfo && !g_mumble.connectionInfo->isConnected))
	{
		if (shouldConnect && !g_mumble.connecting && !g_mumble.errored)
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
		if (!shouldConnect)
		{
			Mumble_Disconnect();
		}
	}

	MumbleActivationMode activationMode;

	if (voiceEnabled)
	{
		if (microActivationMode == 1)
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
	auto voicechatvolume =  hook::get_address<uint8_t>(*g_preference +0x2874)-168;
	g_mumbleClient->SetOutputVolume(voicechatvolume * 0.1f);

	float cameraFront[3];
    float cameraTop[3];
    float cameraPos[3];
    float actorPos[3];


    static auto playerped = fx::ScriptEngine::GetNativeHandler(0x096275889B8E0EE0);
    static auto gameplaycamcoord = fx::ScriptEngine::GetNativeHandler(0x595320200B98596E);
    static auto getcoords = fx::ScriptEngine::GetNativeHandler(0xA86D5F069399F44D);
    int pp = FxNativeInvoke::Invoke<int>(playerped);
    scrVector pc = FxNativeInvoke::Invoke<scrVector>(getcoords, pp);
    scrVector gcc = FxNativeInvoke::Invoke<scrVector>(gameplaycamcoord);

    actorPos[0] = pc.x;
    actorPos[1] = pc.y;
    actorPos[2] = pc.z;

    cameraPos[0] =  gcc.x;
    cameraPos[1] =  gcc.y;
    cameraPos[2] =  gcc.z;

    //////TO DO  ////////////

    cameraFront[0] = 0.0f;
    cameraFront[1] = 0.0f;
    cameraFront[2] = 0.0f;

    cameraTop[0] = 0.0f;
    cameraTop[1] = 0.0f;
    cameraTop[2] = 0.0f;

    ////////////////////////

    g_mumbleClient->SetListenerMatrix(actorPos, cameraFront, cameraTop);
    g_mumbleClient->SetActorPosition(actorPos);

	auto likelihoodValue = hook::get_address<uint8_t>(*g_preference +0x2878)-172;

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
	cxt.Push(0x4BC9DABB); // INPUT_PUSH_TO_TALK

	(*isControlPressed)(cxt);

	g_mumbleClient->SetPTTButtonState(cxt.GetResult<bool>());

	// handle device changes
	static int curInDevice = -1;
	static int curOutDevice = -1;
	//TO DO
	int inDevice = 0;
	int outDevice = 0;

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
		static auto getPlayerPed = fx::ScriptEngine::GetNativeHandler(0x275F255ED201B937);

		auto playerId = FxNativeInvoke::Invoke<uint32_t>(getByServerId, it->second);

		if (playerId < 256 && playerId != -1)
		{
			int ped = FxNativeInvoke::Invoke<int>(getPlayerPed, playerId);

			if (ped > 0)
			{
				auto coords = NativeInvoke::Invoke<0xA86D5F069399F44D, scrVector>(ped);
				return { { coords.x, coords.z, coords.y } };
			}
		}
	}

	return {};
}

static HookFunction initFunction([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
    {
        g_netLibrary = netLibrary;
    });

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
	});
});


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

#include <scrBind.h>

static HookFunction hookFunction([]()
{
	 g_preference = hook::get_address<uint64_t*>(hook::get_pattern<char>("0F 57 F6 74 ? 48 8B 0D ? ? ? ? E8", 8));

	rage::scrEngine::OnScriptInit.Connect([]()
	{
		static auto origIsTalking = fx::ScriptEngine::GetNativeHandler(0xEF6F2A35FAAF2ED7);
		getPlayerName = fx::ScriptEngine::GetNativeHandler(0x7124FD9AC0E01BA0);
		static auto isPlayerActive = fx::ScriptEngine::GetNativeHandler(0xB8DFD30D6973E135);
		getServerId = fx::ScriptEngine::GetNativeHandler(HashString("GET_PLAYER_SERVER_ID"));
		static auto mutePlayer = fx::ScriptEngine::GetNativeHandler(0x49623BCFC3A3D829); // Mute player shoud be changed

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
					 FxNativeInvoke::Invoke<int>(mutePlayer, i, true); // Mute player shoud be changed
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

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_VOLUME_OVERRIDE", [](fx::ScriptContext& context)
		{
			int playerId = context.GetArgument<int>(0);
			float volume = context.GetArgument<float>(1);

			if (g_mumble.connected)
			{
				std::wstring name = ToWide(fmt::sprintf("[%d] %s",
					FxNativeInvoke::Invoke<int>(getServerId, playerId),
					FxNativeInvoke::Invoke<const char*>(getPlayerName, playerId)));

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
				vtConfigs[id] = {};
				g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
			}
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_VOICE_TARGET_CHANNELS", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);

			if (id >= 0 && id < 31)
			{
				std::vector<VoiceTargetConfig::Target>& targets = vtConfigs[id].targets;
				for (size_t i = targets.size(); i--;)
				{
					VoiceTargetConfig::Target& target = targets[i];
					if (!target.channel.empty())
					{
						targets.erase(targets.begin() + i);
					}
				}

				g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
			}
		});
		

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_VOICE_TARGET_PLAYERS", [](fx::ScriptContext& context)
		{
			auto id = context.GetArgument<int>(0);

			if (id >= 0 && id < 31)
			{
				std::vector<VoiceTargetConfig::Target>& targets = vtConfigs[id].targets;
				for (size_t i = targets.size(); i--;)
				{
					VoiceTargetConfig::Target& target = targets[i];
					if (target.users.size() > 0)
					{
						targets.erase(targets.begin() + i);
					}
				}

				g_mumbleClient->UpdateVoiceTarget(id, vtConfigs[id]);
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
					std::wstring name = ToWide(fmt::sprintf("[%d] %s",
						FxNativeInvoke::Invoke<int>(getServerId, playerId),
						FxNativeInvoke::Invoke<const char*>(getPlayerName, playerId)));

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
			int channelId = 0;

			if (g_mumble.connected)
			{
				channelId = g_mumbleClient->GetVoiceChannelFromServerId(serverId);
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

		scrBindGlobal("GET_AUDIOCONTEXT_FOR_CLIENT", getAudioContext);

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

		fx::ScriptEngine::RegisterNativeHandler(0xEF6F2A35FAAF2ED7, [=](fx::ScriptContext& context)
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

		 fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_CHANNEL", [](fx::ScriptContext& context)
        {
            
            if (g_mumble.connected)
            {
                g_mumbleClient->SetChannel(fmt::sprintf("Game Channel %d", context.GetArgument<int>(0)));
            }
        });

        fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CLEAR_CHANNEL", [=](fx::ScriptContext& context)
        {

            if (g_mumble.connected)
            {
                g_mumbleClient->SetChannel("Root");
            }
        });

		auto origSetProximity = fx::ScriptEngine::GetNativeHandler(0x08797A8C03868CB8);

		fx::ScriptEngine::RegisterNativeHandler(0x08797A8C03868CB8, [=](fx::ScriptContext& context)
		{
			(*origSetProximity)(context);

			float dist = context.GetArgument<float>(0);

			g_mumbleClient->SetAudioDistance(dist);
		});

		fx::ScriptEngine::RegisterNativeHandler("MUMBLE_GET_PROXIMITY", [](fx::ScriptContext& context)
        {

            float proximity = g_mumbleClient->GetAudioDistance();

            context.SetResult<float>(proximity);
        });
	});

});
  
