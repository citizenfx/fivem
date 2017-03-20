/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <scrEngine.h>

#include <FontRenderer.h>

enum NativeIdentifiers : uint64_t
{
	GET_PLAYER_PED = 0x43A66C31C68491C0,
	GET_ENTITY_COORDS = 0x3FEF770D40960D5A,
	GET_FIRST_BLIP_INFO_ID = 0x1BEDE233E6CD2A1F,
	GET_NEXT_BLIP_INFO_ID = 0x14F96AA50D6FBEA7,
	GET_BLIP_INFO_ID_TYPE = 0xBE9B0959FFD0779B,
	GET_BLIP_COORDS = 0x586AFE3FF72D996E,
	GET_GROUND_Z_FOR_3D_COORD = 0xC906A7DAB05C8D2B,
	SET_ENTITY_COORDS = 0x621873ECE1178967,
	SET_ENTITY_COORDS_NO_OFFSET = 0x239A3351AC1DA385,
	LOAD_SCENE = 0x4448EB75B4904BDB,
	REQUEST_MODEL = 0x963D27A58DF860AC,
	HAS_MODEL_LOADED = 0x98A4EB5D89A0C952,
	CREATE_VEHICLE = 0xAF35D0D2583051B0,
	SHUTDOWN_LOADING_SCREEN = 0x078EBE9809CCD637,
	DO_SCREEN_FADE_IN = 0xD4E8E24955024033,
	NETWORK_IS_HOST = 0x8DB296B814EDDA07,
	NETWORK_RESURRECT_LOCAL_PLAYER = 0xEA23C49EAA83ACFB,
	NETWORK_IS_GAME_IN_PROGRESS = 0x10FAB35428CCC9D7,
	IS_ENTITY_DEAD = 0x5F9532F3B5CC2551
};

// BLIP_8 in global.gxt2 -> 'Waypoint'
#define BLIP_WAYPOINT 8

#include <SteamComponentAPI.h>

inline ISteamComponent* GetSteam()
{
	auto steamComponent = Instance<ISteamComponent>::Get();

	// if Steam isn't running, return an error
	if (!steamComponent->IsSteamRunning())
	{
		return nullptr;
	}

	return steamComponent;
}

class LovelyThread : public GtaThread
{
private:
	bool m_isWaitingForModelToLoad;

	bool m_doInityThings;

	bool m_shouldCreate;

	bool m_markScript;

public:
	LovelyThread(bool shouldCreate)
		: m_shouldCreate(shouldCreate)
	{
		m_isWaitingForModelToLoad = false;
		m_doInityThings = true;
		m_markScript = false;
	}

	virtual rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) override
	{
		m_isWaitingForModelToLoad = false;
		m_doInityThings = true;
		m_markScript = false;

		return GtaThread::Reset(scriptHash, pArgs, argCount);
	}

	virtual void DoRun() override
	{
		if (!m_markScript)
		{
			static int id = 0;
			id++;

			NativeInvoke::Invoke<0xD1110739EEADB592, uint32_t>(18, false, -1);
			m_markScript = true;
		}

		uint32_t playerPedId = NativeInvoke::Invoke<GET_PLAYER_PED, uint32_t>(-1);

		if (!m_shouldCreate && m_doInityThings)
		{
			/*NativeInvoke::Invoke<LOAD_SCENE, int>(-426.858f, -957.54f, 3.621f);

			NativeInvoke::Invoke<SHUTDOWN_LOADING_SCREEN, int>();
			NativeInvoke::Invoke<DO_SCREEN_FADE_IN, int>(0);

			NativeInvoke::Invoke<SET_ENTITY_COORDS, int>(playerPedId, -426.858f, -957.54f, 3.621f);*/

			m_doInityThings = false;
		}

		if (playerPedId != -1 && playerPedId != 0)
		{
			static bool respawning = false;

			/*
			if (NativeInvoke::Invoke<NETWORK_IS_GAME_IN_PROGRESS, bool>())
			{
				if (NativeInvoke::Invoke<IS_ENTITY_DEAD, bool>(playerPedId))
				{
					scrVector entityCoords = NativeInvoke::Invoke<GET_ENTITY_COORDS, scrVector>(playerPedId);

					NativeInvoke::Invoke<NETWORK_RESURRECT_LOCAL_PLAYER, int>(entityCoords.x, entityCoords.y, entityCoords.z, 0.0f, true, true);
				}
			}
			*/

			if (!m_shouldCreate)
			{
				scrVector entityCoords = NativeInvoke::Invoke<GET_ENTITY_COORDS, scrVector>(playerPedId);

				CRect rect(0, 0, 22, 22);
				CRGBA color;

				if (!NativeInvoke::Invoke<0x9DE624D2FC4B603F, bool>())
				{
					color = CRGBA(200, 0, 0, 180);
					TheFonts->DrawRectangle(rect, color);
				}
				else
				{
					auto steam = GetSteam();

					if (steam)
					{
						int playerCount = 0;

						for (int i = 0; i < 32; i++)
						{
							// NETWORK_IS_PLAYER_ACTIVE
							if (NativeInvoke::Invoke<0xB8DFD30D6973E135, bool>(i))
							{
								++playerCount;
							}
						}

						static int lastPlayerCount;

						if (playerCount != lastPlayerCount)
						{
							steam->SetRichPresenceValue(1, fmt::sprintf("%d players", playerCount));
							lastPlayerCount = playerCount;
						}
					}
				}

				// if the particular key we like is pressed...
				/*
				static bool wasPressed = true;

				if (GetAsyncKeyState(VK_F11) & 0x8000)
				{
					if (!wasPressed)
					{
						// iterate through blips to find a waypoint
						int infoId = NativeInvoke::Invoke<GET_FIRST_BLIP_INFO_ID, int>(BLIP_WAYPOINT);

						if (infoId > 0)
						{
							scrVector blipCoords = NativeInvoke::Invoke<GET_BLIP_COORDS, scrVector>(infoId);

							NativeInvoke::Invoke<LOAD_SCENE, int>(blipCoords.x, blipCoords.y, blipCoords.z);

							float newZ = 0.0f;
							NativeInvoke::Invoke<GET_GROUND_Z_FOR_3D_COORD, int>(blipCoords.x, blipCoords.y, 1000.0f, &newZ);

							NativeInvoke::Invoke<SET_ENTITY_COORDS, int>(playerPedId, blipCoords.x, blipCoords.y, newZ);
						}

						wasPressed = true;
					}
				}
				else
				{
					wasPressed = false;
				}

				// spawn the vehicle, somewhere
				static bool wasF9Pressed = true;

				if (GetAsyncKeyState(VK_F9) & 0x8000)
				{
					if (!wasF9Pressed)
					{
						//NativeInvoke::Invoke<REQUEST_MODEL, int>(0x2B6DC64A);

						//m_isWaitingForModelToLoad = true;

						NativeInvoke::Invoke<0x593850C16A36B692, int>();

						wasF9Pressed = true;
					}
				}
				else
				{
					wasF9Pressed = false;
				}

				if (m_isWaitingForModelToLoad)
				{
					if (NativeInvoke::Invoke<HAS_MODEL_LOADED, bool>(0x2B6DC64A))
					{
						scrVector entityCoords = NativeInvoke::Invoke<GET_ENTITY_COORDS, scrVector>(playerPedId);

						NativeInvoke::Invoke<CREATE_VEHICLE, int>(0x2B6DC64A, entityCoords.x, entityCoords.y + 2, entityCoords.z, 0.0f, 1, 0);

						m_isWaitingForModelToLoad = false;
					}
				}*/
				static bool hosted = false;

				if (!hosted && NativeInvoke::Invoke<NETWORK_IS_HOST, bool>())
				{
					NativeInvoke::Invoke<0xC19F6C8E7865A6FF, int>(1);

					hosted = true;
				}
			}
			else
			{
				static bool hosted = false;

				if (!hosted && NativeInvoke::Invoke<NETWORK_IS_HOST, bool>())
				{
					rage::scrEngine::CreateThread(new LovelyThread(true));

					hosted = true;
				}
			}
		}
	}
};

static LovelyThread lovelyThread(false);

#include <Hooking.h>

static InitFunction initFunction([] ()
{
	rage::scrEngine::OnScriptInit.Connect([] ()
	{
		rage::scrEngine::CreateThread(&lovelyThread);
	});
});