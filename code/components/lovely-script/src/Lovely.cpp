/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <scrEngine.h>

#include <NetLibrary.h>

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
	bool m_shouldCreate;

	bool m_hosted;

public:
	LovelyThread(bool shouldCreate)
		: m_shouldCreate(shouldCreate)
	{
	}

	virtual rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) override
	{
		m_hosted = false;

		return GtaThread::Reset(scriptHash, pArgs, argCount);
	}

	virtual void DoRun() override
	{
		uint32_t playerPedId = NativeInvoke::Invoke<GET_PLAYER_PED, uint32_t>(-1);

		if (playerPedId != -1 && playerPedId != 0)
		{
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

				int playerCount = 0;

				for (int i = 0; i < 256; i++)
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
					if (steam)
					{
						steam->SetRichPresenceValue(1, fmt::sprintf("%d player%s", playerCount, (playerCount != 1) ? "s" : ""));
					}

					OnRichPresenceSetValue(1, fmt::sprintf("%d player%s", playerCount, (playerCount != 1) ? "s" : ""));
					lastPlayerCount = playerCount;
				}
			}

			if (!m_hosted && NativeInvoke::Invoke<NETWORK_IS_HOST, bool>())
			{
				NativeInvoke::Invoke<0xC19F6C8E7865A6FF, int>(1);

				m_hosted = true;
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
