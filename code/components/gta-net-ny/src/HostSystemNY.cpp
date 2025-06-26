#include <StdInc.h>
#include <HostSystemNY.h>

#include <Error.h>
#include <ICoreGameInit.h>
#include <nutsnbolts.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <msgpack.hpp>
#include <scrEngine.h>

#include <NetLibrary.h>

extern NetLibrary* g_netLibrary;

GTA_NET_EXPORT fwEvent<HostState, HostState> OnHostStateTransition;

enum NativeIdentifiers : uint64_t
{
	CREATE_PLAYER = 0x335E3951,
	SET_FILTER_MENU_ON = 0x18F43649,
	NETWORK_FIND_GAME = 0x5D4D0C86,
	NETWORK_FIND_GAME_PENDING = 0x23D60810,
	NETWORK_GET_NUMBER_OF_GAMES = 0x10DF4CED,
	NETWORK_JOIN_GAME = 0x60806A0C,
	NETWORK_JOIN_GAME_PENDING = 0x76C53927,
	NETWORK_JOIN_GAME_SUCCEEDED = 0x59F24327,
	NETWORK_HOST_GAME = 0x5BEA05E2,
	NETWORK_HOST_GAME_PENDING = 0x391E4575,
	NETWORK_HOST_GAME_SUCCEEDED = 0x1CA77E94,
	LOCAL_PLAYER_IS_READY_TO_START_PLAYING = 0x5C03585C,
	LAUNCH_LOCAL_PLAYER_IN_NETWORK_GAME = 0x70FE415C,
	PLAYER_WANTS_TO_JOIN_NETWORK_GAME = 0x7D99343C,
	TELL_NET_PLAYER_TO_START_PLAYING = 0x465D424D,
	IS_NETWORK_PLAYER_ACTIVE = 0x4E237943,
	IS_THIS_MACHINE_THE_SERVER = 0x2E5E1600,
};

class InitScriptThread : public GtaThread
{
	bool inited = false;

	virtual void DoRun() override
	{
		if (!inited)
		{
			int playerId = 0;
			NativeInvoke::Invoke<CREATE_PLAYER, int>(0, -2000.0f, -2000.0f, 240.5f, &playerId);
			inited = true;
		}
	}
};

InitScriptThread thread;

struct HostStateHolder
{
	HostState state;

	inline bool operator==(HostState right)
	{
		return (state == right);
	}

	inline HostState operator=(HostState right)
	{
		trace("HostState transitioning from %s to %s\n", HostStateToString(state), HostStateToString(right));

		AddCrashometry("hs_state", HostStateToString(right));

		OnHostStateTransition(right, state);
		state = right;

		return right;
	}
};

struct
{
	HostStateHolder state;
	int attempts;

	std::string hostResult;

	void handleHostResult(const std::string& str)
	{
		hostResult = str;
	}

	void process()
	{
		ICoreGameInit* cgi = Instance<ICoreGameInit>::Get();

		if (state == HS_LOADED)
		{
			NativeInvoke::Invoke<SET_FILTER_MENU_ON, int>(true);

			// if there's no host, start hosting - if there is, start joining
			if (g_netLibrary->GetHostNetID() == 0xFFFF || g_netLibrary->GetHostNetID() == g_netLibrary->GetServerNetID())
			{
				state = HS_START_HOSTING;
			}
			else
			{
				state = HS_START_FINDING;
			}
		}
		else if (state == HS_START_FINDING)
		{
			// SetFilterMenuOn
			NativeInvoke::Invoke<SET_FILTER_MENU_ON, int>(true);

			// NetworkFindGame
			NativeInvoke::Invoke<NETWORK_FIND_GAME, int>(16, false, 0, 0);

			state = HS_FINDING;
		}
		else if (state == HS_FINDING)
		{
			// find pending
			if (!NativeInvoke::Invoke<NETWORK_FIND_GAME_PENDING, bool>())
			{
				int numGames = NativeInvoke::Invoke<NETWORK_GET_NUMBER_OF_GAMES, int>();

				if (numGames == 0)
				{
					state = HS_JOIN_FAILURE;
				}
				else
				{
					state = HS_START_JOINING;
				}

				NativeInvoke::Invoke<SET_FILTER_MENU_ON, int>(false);
			}
		}
		else if (state == HS_START_JOINING)
		{
			NativeInvoke::Invoke<NETWORK_JOIN_GAME, int>(0);

			state = HS_JOINING;
		}
		else if (state == HS_JOINING)
		{

			if (!NativeInvoke::Invoke<NETWORK_JOIN_GAME_PENDING, bool>())
			{
				state = HS_JOINING_NET_GAME;
			}
		}
		else if (state == HS_JOINING_NET_GAME)
		{
			// two possible target states
			if (NativeInvoke::Invoke<NETWORK_JOIN_GAME_SUCCEEDED, bool>()) // join succeeded
			{
				cgi->SetVariable("networkInited");
				state = HS_JOINED;
			}
			else
			{
				state = HS_JOIN_FAILURE;
			}
		}
		else if (state == HS_JOIN_FAILURE)
		{
			NativeInvoke::Invoke<NETWORK_JOIN_GAME, int>(0);

			if (cgi->EnhancedHostSupport || true)
			{
				state = HS_START_HOSTING;
			}
			else
			{
				trace("No enhanced host support is active, failing fast.\n");

				state = HS_FATAL;
			}
		}
		else if (state == HS_START_HOSTING)
		{
			NativeInvoke::Invoke<SET_FILTER_MENU_ON, int>(false);

			if (cgi->EnhancedHostSupport)
			{
				msgpack::sbuffer nameArgs;
				msgpack::packer<msgpack::sbuffer> packer(nameArgs);

				packer.pack_array(0);

				g_netLibrary->SendNetEvent("hostingSession", std::string(nameArgs.data(), nameArgs.size()));

				state = HS_WAIT_HOSTING;
			}
			else
			{
				if (!NativeInvoke::Invoke<NETWORK_HOST_GAME, bool>(16, false, 32, false, 0, 0))
				{
					state = HS_FATAL;
				}
				else
				{
					state = HS_HOSTING;
				}
			}
		}
		else if (state == HS_WAIT_HOSTING || state == HS_WAIT_HOSTING_2)
		{
			if (!hostResult.empty())
			{
				std::string result = hostResult;
				hostResult = "";

				trace("received hostResult %s\n", result);

				// 'wait' is a no-op, should wait for the next response
				if (result == "wait")
				{
					state = HS_WAIT_HOSTING_2;
					return;
				}

				if (state == HS_WAIT_HOSTING_2)
				{
					if (result == "free")
					{
						// if free after a wait, restart as if loaded
						state = HS_LOADED;
						return;
					}
				}

				if (result == "conflict")
				{
					if (g_netLibrary->GetHostNetID() != 0xFFFF && g_netLibrary->GetHostNetID() != g_netLibrary->GetServerNetID())
					{
						trace("session creation conflict");

						state = HS_FATAL;
						return;
					}
				}

				// no conflict, no wait, start hosting
				if (!NativeInvoke::Invoke<NETWORK_HOST_GAME, bool>(16, false, 32, false, 0, 0))
				{
					state = HS_FATAL;
				}
				else
				{
					state = HS_HOSTING;
				}
			}
		}
		else if (state == HS_HOSTING)
		{
			if (!NativeInvoke::Invoke<NETWORK_HOST_GAME_PENDING, bool>())
			{
				state = HS_HOSTING_NET_GAME;
			}
		}
		else if (state == HS_HOSTING_NET_GAME)
		{
			if (NativeInvoke::Invoke<NETWORK_HOST_GAME_SUCCEEDED, bool>())
			{
				cgi->SetVariable("networkInited");
				state = HS_HOSTED;

				if (cgi->EnhancedHostSupport)
				{
					msgpack::sbuffer nameArgs;
					msgpack::packer<msgpack::sbuffer> packer(nameArgs);

					packer.pack_array(0);

					g_netLibrary->SendNetEvent("hostedSession", std::string(nameArgs.data(), nameArgs.size()));
				}
			}
			else
			{
				state = HS_FATAL;
			}
		}
		else if (state == HS_FATAL)
		{
			if (attempts < 3)
			{
				++attempts;
				state = HS_LOADED;
			}
			else
			{
				GlobalError("Could not connect to session provider. This may happen when you recently updated, but other players in the server have not. Alternately, the server accepted you, despite being full. Please try again later, or try a different server.");
				state = HS_IDLE;
			}
		}
		else if (state == HS_HOSTED || state == HS_JOINED)
		{
			// servicing: local join

			if (NativeInvoke::Invoke<NETWORK_LOCAL_PLAYER_IS_READY_TO_START_PLAYING, bool>())
			{
				NativeInvoke::Invoke<NETWORK_LAUNCH_LOCAL_PLAYER_IN_NETWORK_GAME, int>();
			}

			// remote join
			for (int i = 0; i < 32; i++)
			{
				if (NativeInvoke::Invoke<NETWORK_PLAYER_WANTS_TO_JOIN_NETWORK_GAME, bool>(i))
				{
					NativeInvoke::Invoke<TELL_NET_PLAYER_TO_START_PLAYING, int>(i, 0);
				}
			}

			// mismatch logic
			int playerCount = 0;

			for (int i = 0; i < 32; i++)
			{
				if (NativeInvoke::Invoke<IS_NETWORK_PLAYER_ACTIVE, bool>(i))
				{
					++playerCount;
				}
			}

			bool isHost = NativeInvoke::Invoke<IS_THIS_MACHINE_THE_SERVER, bool>();

			static uint64_t mismatchStart = UINT64_MAX;

			if (isHost && playerCount == 1 && !cgi->OneSyncEnabled && g_netLibrary->GetHostNetID() != g_netLibrary->GetServerNetID())
			{
				if (mismatchStart == UINT64_MAX)
				{
					mismatchStart = GetTickCount64();
				}
				else if ((GetTickCount64() - mismatchStart) > 7500)
				{
					state = HS_MISMATCH;
				}
			}
			else
			{
				mismatchStart = UINT64_MAX;
			}
		}
		else if (state == HS_MISMATCH)
		{
			cgi->ClearVariable("networkInited");

			//networkBail(7, -1, -1, -1, true);

			state = HS_DISCONNECTING;
		}
		else if (state == HS_FORCE_DISCONNECT)
		{
			//networkBail(7, -1, -1, -1, true);

			state = HS_DISCONNECTING_FINAL;
		}
		else if (state == HS_DISCONNECTING)
		{
			//if (!isSessionStarted())
			{
				state = HS_LOADED;
			}
		}
		else if (state == HS_DISCONNECTING_FINAL)
		{
			//if (!isSessionStarted())
			{
				cgi->OneSyncEnabled = false;

				state = HS_IDLE;
			}
		}
	}
} hostSystem;

static InitFunction initFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		static bool gameLoaded = false;
		static bool eventConnected = false;

		if (!eventConnected)
		{
			Instance<ICoreGameInit>::Get()->OnGameFinalizeLoad.Connect([]()
			{
				gameLoaded = true;
			});
// 
// 			OnKillNetwork.Connect([](const char*)
// 			{
// 				gameLoaded = false;
// 			});

			eventConnected = true;
		}

		static bool had = false;

		if (Instance<ICoreGameInit>::Get()->GetGameLoaded() && !had)
		{
			gameLoaded = true;
			had = true;
		}

		if (gameLoaded)
		{
			gameLoaded = false;

			hostSystem.state = HS_LOADED;
		}

		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			hostSystem.process();
		}
	});

	rage::scrEngine::OnScriptInit.Connect([]()
	{
		rage::scrEngine::CreateThread(&thread);
	});
});
