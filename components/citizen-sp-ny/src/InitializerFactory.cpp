/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "InitializerFactory.h"
#include "Hooking.h"

#include <CefOverlay.h>
#include <scrEngine.h>
#include <ResourceManager.h>
#include <nutsnbolts.h>

#include <concurrent_queue.h>

// stub for ASM
static int g_currentEpisodeID;
static bool g_isInEpisode;
static concurrency::concurrent_queue<std::function<void()>> g_mainThreadQueue;

int GetCurrentEpisodeID()
{
	return g_currentEpisodeID;
}

Episode::EpisodeInitializer InitializerFactory::GetSinglePlayerInitializer(int episodeNum, std::string episodeDat)
{
	return [=] (fwRefContainer<Episode> episode)
	{
		// the next game *isn't* a network game
		hook::put<uint32_t>(0x10F8070, 0);

		// set the episode ID for legacy HookEpisodicContent
		g_currentEpisodeID = episodeNum;

		// mark scripts as allowed
		g_isInEpisode = true;

		// disable main UI
		nui::SetMainUI(false);
		nui::DestroyFrame("mpMenu");

		// initialize the resource manager on the main thread
		g_mainThreadQueue.push([=] ()
		{
			// if we're still in the main menu...
			if (*(uint8_t*)0xF22B3C)
			{
				// tell the game to load after the initial main menu
				hook::put<uint32_t>(0x10C7F80, 6);
			}
			else
			{
				// make the game reinitialize
				hook::put<uint8_t>(0x10F8074, 1);

				*(bool*)0x18A8238 = true;
			}

			// muh
			hook::put<uint8_t>(0x18A823A, 0);

			((void(*)(int, int, int))0x423CE0)(0, 0, 0);

			*(BYTE*)0x18A823A = 1;

			// reset the resource manager
			TheResources.Reset();

			// create and 'start' the startup resource
			auto resource = TheResources.AddResource("startup", "citizen:/resources/sp/startup/");
			resource->Start();
		});

		return true;
	};
}

static InitFunction initFunction([] ()
{
	OnGameFrame.Connect([] ()
	{
		std::function<void()> runFunc;

		while (g_mainThreadQueue.try_pop(runFunc))
		{
			runFunc();
		}
	});

	rage::scrEngine::CheckNativeScriptAllowed.Connect([] (bool& allowed)
	{
		if (g_isInEpisode)
		{
			allowed = true;
			return false;
		}

		return true;
	});
});