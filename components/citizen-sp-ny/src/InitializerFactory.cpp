/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "InitializerFactory.h"
#include "Hooking.h"
#include "NativeEpisode.h"

#include <CefOverlay.h>
#include <ICoreGameInit.h>
#include <nutsnbolts.h>
#include <ResourceManager.h>
#include <scrEngine.h>

#include <concurrent_queue.h>

#include <strsafe.h>

// stub for ASM
static int g_currentEpisodeID = -1;
static int g_nextEpisodeID = -1;
static bool g_isInEpisode;
static std::string g_nextEpisodeDat = "-";
static concurrency::concurrent_queue<std::function<void()>> g_mainThreadQueue;

static void ApplyEpisodeDat(const std::string& episodeDat)
{
	// clear the native episode list
	for (int i = 0; i < g_episodes->numEpisodes; i++)
	{
		g_episodes->episodes[i].useThis = false;
		g_episodes->numEpisodes--;
	}

	g_episodes->numContents = 0;

	// mark to the hook that we do want North episodes, if possible
	g_useNorthEpisodes = !episodeDat.empty();

	// if this is an episode...
	if (!episodeDat.empty())
	{
		// load the requested episode .dat file
		CEpisode ep;
		memset(&ep, 0, sizeof(ep));
		StringCbCopyA(ep.path, sizeof(ep.path), episodeDat.c_str());
		strrchr(ep.path, '\\')[1] = '\0'; // strip the filename
		ep.useThis = 1;
		ep.deviceType = 1;

		g_episodes->addEpisode(&ep);
	}

	// rescan episodes
	g_episodes->unknownScanFlag = false;

	g_episodes->ScanEpisodes();
}

int GetCurrentEpisodeID()
{
	int returnedID = g_currentEpisodeID;

	if (g_nextEpisodeID != -1)
	{
		g_currentEpisodeID = g_nextEpisodeID;
		g_nextEpisodeID = -1;
	}
	else
	{
		if (g_nextEpisodeDat != "-")
		{
			ApplyEpisodeDat(g_nextEpisodeDat);

			g_nextEpisodeDat = "-";
		}
	}

	if (returnedID == -1)
	{
		returnedID = 2;
	}

	return returnedID;
}

Episode::EpisodeInitializer InitializerFactory::GetSinglePlayerInitializer(int episodeNum, std::string episodeDat)
{
	return [=] (fwRefContainer<Episode> episode)
	{
		// the next game *isn't* a network game
		hook::put<uint32_t>(0x10F8070, 0);

		// set a flag for the subhandler to know if this was an instant episode change
		bool isInstant = (g_currentEpisodeID == -1);

		// set the episode ID for legacy HookEpisodicContent
		if (g_currentEpisodeID == -1)
		{
			g_currentEpisodeID = episodeNum;
		}
		else
		{
			g_nextEpisodeID = episodeNum;
			g_nextEpisodeDat = episodeDat;
		}

		// mark scripts as allowed
		g_isInEpisode = true;

		// disable main UI
		nui::SetMainUI(false);
		nui::DestroyFrame("mpMenu");

		// initialize the resource manager on the main thread
		g_mainThreadQueue.push([=] ()
		{
			// apply the episode dat if it were instant
			if (isInstant)
			{
				ApplyEpisodeDat(episodeDat);
			}

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

			// prevent save loading for now
			g_preventSaveLoading = true;

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

		Instance<ICoreGameInit>::Get()->SetPreventSavePointer(&g_preventSaveLoading);
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