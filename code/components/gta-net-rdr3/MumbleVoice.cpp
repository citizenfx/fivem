
#include "StdInc.h"
#include "Hooking.h"

#include <GameInit.h>
#include <ICoreGameInit.h>
#include <nutsnbolts.h>

#include <NetLibrary.h>
#include <MumbleClient.h>

#include <sstream>
#include <regex>

#include <scrEngine.h>
#include <MinHook.h>

#include <mmsystem.h>
#include <dsound.h>
#include <ScriptEngine.h>

#include <CoreConsole.h>

#include <rapidjson/document.h>
#include <LabSound/extended/LabSound.h>
#include <scrBind.h>


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


static NetLibrary* g_netLibrary;

static char* g_preference;
static char* g_device;

static IMumbleClient* g_mumbleClient;

static struct
{
    volatile bool connecting;
    volatile bool connected;
    volatile bool errored;

    volatile MumbleConnectionInfo* connectionInfo;

    volatile int nextConnectDelay;
    volatile uint64_t nextConnectAt;

    concurrency::concurrent_queue<std::function<void()>> mainFrameExecQueue;
} g_mumble;

static void Mumble_Connect()
{
    g_mumble.errored = false;
    g_mumble.connecting = true;


    trace("Exception: %s\n", g_netLibrary->GetPlayerName());

    g_mumbleClient->ConnectAsync(g_netLibrary->GetCurrentPeer(), fmt::sprintf("[%d] %s", g_netLibrary->GetServerNetID(), g_netLibrary->GetPlayerName())).then([](concurrency::task<MumbleConnectionInfo*> task)
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

static void Mumble_Disconnect()
{
    g_mumble.connected = false;
    g_mumble.errored = false;
    g_mumble.connecting = false;
    g_mumble.nextConnectDelay = 4 * 1000;

    g_mumbleClient->DisconnectAsync().then([=]()
    {
    });


}

static float* g_actorPos;

#pragma comment(lib, "dsound.lib")

static std::bitset<256> g_talkers;
static std::bitset<256> o_talkers;

static std::unordered_map<std::string, int> g_userNamesToClientIds;
static std::regex g_usernameRe("^\\[(0-9+)\\] ");

static auto PositionHook(const std::string& userName) -> std::optional<std::array<float, 3>>
{
    auto it = g_userNamesToClientIds.find(userName);

    if (it == g_userNamesToClientIds.end())
    {
        std::smatch matches;

        if (std::regex_match(userName, matches, g_usernameRe))
        {
            int serverId = std::stoi(matches[1].str());

            static auto getByServerId = fx::ScriptEngine::GetNativeHandler(HashString("GET_PLAYER_FROM_SERVER_ID"));

            it = g_userNamesToClientIds.insert({ userName, FxNativeInvoke::Invoke<int>(getByServerId, serverId) }).first;
        }
    }

    if (it != g_userNamesToClientIds.end())
    {
        static auto getPlayerPed = fx::ScriptEngine::GetNativeHandler(HashString("GET_PLAYER_PED"));
        static auto getEntityCoords = fx::ScriptEngine::GetNativeHandler(HashString("GET_ENTITY_COORDS"));

        int ped = FxNativeInvoke::Invoke<int>(getPlayerPed, it->second);

        if (ped > 0)
        {
            auto coords = FxNativeInvoke::Invoke<scrVector>(getEntityCoords, ped);
            return { { coords.x, coords.y, coords.z } };
        }
    }

    return {};
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

    bool shouldConnect = true;

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


    auto voiceEnabled = hook::get_address<uint8_t>(g_preference -4)- 180 ;
    auto microEnabled = hook::get_address<uint8_t>(g_preference -2) - 182;
    auto microActivationMode = hook::get_address<uint8_t>(g_preference + 0x14) -204;

    MumbleActivationMode activationMode;

    if (voiceEnabled && microEnabled)
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

    auto voicechatvolume = hook::get_address<uint8_t>(g_preference) -184;
    g_mumbleClient->SetOutputVolume(voicechatvolume *0.1f);

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

    cameraPos[0] = gcc.x;
    cameraPos[1] = gcc.y;
    cameraPos[2] = gcc.z;

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


    auto likelihoodValue = hook::get_address<uint8_t>(g_preference + 4)  -188;


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

        Mumble_Disconnect();
        o_talkers.reset();
    });
});


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

static bool(*g_origGetPlayerHasHeadset)(void*, void*);

static bool _getPlayerHasHeadset(void* mgr, void* plr)
{
	if (g_mumble.connected)
	{
		return 25;
	}

	return g_origGetPlayerHasHeadset(mgr, plr);
}

static HookFunction hookFunction([]()
{


    rage::scrEngine::OnScriptInit.Connect([]()
    {

        g_preference = hook::get_pattern<char>("40 01 00 00 80 02 00 00 A0 41 00 00 00 00 00 00", +0x9D4);

		//g_device = hook::get_pattern<char>("CD CC 4C 3E 00 00 80 3F CD CC CC 3D 00 00 80 BE",  -0x16E);
		
        static auto origIsTalking = fx::ScriptEngine::GetNativeHandler(0xEF6F2A35FAAF2ED7);
        static auto isPlayerActive = fx::ScriptEngine::GetNativeHandler(0xB8DFD30D6973E135);
        static auto mutePlayer = fx::ScriptEngine::GetNativeHandler(0x49623BCFC3A3D829);
        getPlayerName = fx::ScriptEngine::GetNativeHandler(0x7124FD9AC0E01BA0);
        getServerId = fx::ScriptEngine::GetNativeHandler(HashString("GET_PLAYER_SERVER_ID"));

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
                    FxNativeInvoke::Invoke<int>(mutePlayer, i, true);
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

        fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_PROXIMITY", [](fx::ScriptContext& context)
        {

            float dist = context.GetArgument<float>(0);

            g_mumbleClient->SetAudioDistance(dist);
        });

        fx::ScriptEngine::RegisterNativeHandler("MUMBLE_GET_PROXIMITY", [](fx::ScriptContext& context)
        {

            float proximity = g_mumbleClient->GetAudioDistance();

            context.SetResult<float>(proximity);
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
        auto origgetPlayerHasHeadset = fx::ScriptEngine::GetNativeHandler(0xAA35FD9ABAB490A3);

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

        fx::ScriptEngine::RegisterNativeHandler(0xAA35FD9ABAB490A3, [=](fx::ScriptContext& context)
        {
            (*origgetPlayerHasHeadset)(context);

            if (g_mumble.connected)
            {
                return true;
            }
        });

    });


});
