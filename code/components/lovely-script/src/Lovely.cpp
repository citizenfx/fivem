/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#ifndef GTA_NY
#include <scrEngine.h>

#include <NetLibrary.h>

#include <FontRenderer.h>

#include <ICoreGameInit.h>

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
	bool m_shouldCreate = false;

	bool m_hosted = false;
	bool m_lastOff = false;

public:
	LovelyThread(bool shouldCreate)
		: m_shouldCreate(shouldCreate)
	{
	}

	virtual rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) override
	{
		m_hosted = false;
		m_lastOff = false;

		return GtaThread::Reset(scriptHash, pArgs, argCount);
	}

	void ProcessPopulationToggle()
	{
		// TEMP: force-disable population for 1s big using script
		// should be done natively someday
		static auto icgi = Instance<ICoreGameInit>::Get();
		bool currentOff = (icgi->HasVariable("onesync_big") && !icgi->OneSyncBigIdEnabled) || icgi->HasVariable("strict_entity_lockdown");

		auto setDispatch = [](bool enable)
		{
			for (int i = 1; i <= 15; i++)
			{
				// ENABLE_DISPATCH_SERVICE
				NativeInvoke::Invoke<0xDC0F817884CDD856, int>(i, enable);
			}
		};

		if (currentOff)
		{
			if (!m_lastOff)
			{
				setDispatch(false);

				m_lastOff = true;
			}

			// SET_PED_DENSITY_MULTIPLIER_THIS_FRAME
			NativeInvoke::Invoke<0x95E3D6257B166CF2, int>(0.0f);

			// SET_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME
			NativeInvoke::Invoke<0x245A6883D966D537, int>(0.0f);

			// SET_SCENARIO_PED_DENSITY_MULTIPLIER_THIS_FRAME
			NativeInvoke::Invoke<0x7A556143A1C03898, int>(0.0f, 0.0f);

			// SET_AMBIENT_VEHICLE_RANGE_MULTIPLIER_THIS_FRAME
			NativeInvoke::Invoke<0x90B6DA738A9A25DA, int>(0.0f);

			// SET_PARKED_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME
			NativeInvoke::Invoke<0xEAE6DCC7EEE3DB1D, int>(0.0f);

			// SET_RANDOM_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME
			NativeInvoke::Invoke<0xB3B3359379FE77D3, int>(0.0f);
		}
		else
		{
			if (m_lastOff)
			{
				setDispatch(true);

				m_lastOff = false;
			}
		}
	}

	virtual void DoRun() override
	{
		ProcessPopulationToggle();

// PLAYER_PED_ID
#ifdef GTA_FIVE
		uint32_t playerPedId = NativeInvoke::Invoke<0xD80958FC74E988A6, uint32_t>();
#elif defined(IS_RDR3)
		uint32_t playerPedId = NativeInvoke::Invoke<0x096275889B8E0EE0, uint32_t>();
#endif
		

		if (playerPedId != -1 && playerPedId != 0)
		{
#ifdef GTA_FIVE
			// NETWORK_IS_SESSION_STARTED
			if (!NativeInvoke::Invoke<0x9DE624D2FC4B603F, bool>())
			{
				CRect rect(0, 0, 22, 22);
				CRGBA color;

				static auto icgi = Instance<ICoreGameInit>::Get();

				if (!icgi->HasVariable("storyMode"))
				{
					color = CRGBA(200, 0, 0, 180);
					TheFonts->DrawRectangle(rect, color);
				}
			}
			else
#elif defined(IS_RDR3)
			if (NativeInvoke::Invoke<0x9DE624D2FC4B603F, bool>())
#endif
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
#ifdef GTA_FIVE
			// NETWORK_IS_HOST
			if (!m_hosted && NativeInvoke::Invoke<0x8DB296B814EDDA07, bool>())
			{
				// NETWORK_SESSION_VALIDATE_JOIN
				NativeInvoke::Invoke<0xC19F6C8E7865A6FF, int>(1);

				m_hosted = true;
			}
#endif
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
#endif
