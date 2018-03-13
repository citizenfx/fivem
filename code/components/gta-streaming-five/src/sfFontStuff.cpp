#include "StdInc.h"
#include <sfFontStuff.h>

#include <Hooking.h>

#include <Streaming.h>
#include <nutsnbolts.h>

#include <queue>

static std::vector<std::string> g_fontIds = {
	"$Font2",
	"$Font5",
	"$RockstarTAG",
	"$GTAVLeaderboard",
	"$Font2_cond",
	"$FixedWidthNumbers",
	"$Font2_cond_NOT_GAMERNAME",
	"$gtaCash",
	// padding for bad scripts
	"$Font2",
	"$Font2",
	"$Font2",
	"$Font2",
	"$Font2",
};

static std::set<std::string> g_fontLoadQueue;

namespace sf
{
	int RegisterFontIndex(const std::string& fontName)
	{
		g_fontIds.push_back(fontName);
		return g_fontIds.size() - 1;
	}

	void RegisterFontLib(const std::string& swfName)
	{
		g_fontLoadQueue.insert(swfName);
	}
}

static void(*gfxStringAssign)(void*, const char*);

static void GetFontById(void* gfxString, int id)
{
	if (id < 0 || id >= g_fontIds.size())
	{
		id = 0;
	}

	gfxStringAssign(gfxString, g_fontIds[id].c_str());
}

static void(*assignFontLib)(void*, void*, int);

struct strStreamingModule
{
	uintptr_t vtable;
	int baseIdx;
};

strStreamingModule* g_scaleformStore;
void* g_fontLib;

static void AssignFontLibWrap(strStreamingModule* store, void* lib, int idx)
{
	assignFontLib(store, lib, idx);

	if (store)
	{
		g_scaleformStore = store;
		g_fontLib = lib;
	}
}

static void UpdateFontLoading()
{
	std::set<std::string> toRemove;

	auto cstreaming = streaming::Manager::GetInstance();

	for (const auto& font : g_fontLoadQueue)
	{
		// get the streaming entry
		auto strIdx = streaming::GetStreamingIndexForName(font + ".gfx");

		if (strIdx != 0)
		{
			// check if the font file is loaded
			if ((cstreaming->Entries[strIdx].flags & 3) == 1)
			{
				// it has. add to the font library
				trace("font file %s loaded - adding to GFxFontLib\n", font);

				assignFontLib(g_scaleformStore, g_fontLib, strIdx - g_scaleformStore->baseIdx);

				toRemove.insert(font);
			}
			else
			{
				cstreaming->RequestObject(strIdx, 0);
			}
		}
		else
		{
			trace("unknown font file %s\n", font);

			toRemove.insert(font);
		}
	}

	for (const auto& font : toRemove)
	{
		g_fontLoadQueue.erase(font);
	}
}

static HookFunction hookFunction([]()
{
	// get font by id, hook
	{
		auto location = hook::get_pattern<char>("85 D2 74 68 FF CA 74 5B FF CA 74 4E");
		hook::jump(location, GetFontById);
		hook::set_call(&gfxStringAssign, location + 115);
	}

	// init font things
	{
		auto location = hook::get_pattern<char>("BA 13 00 00 00 48 8B 48 20 48 8B 01 FF 50 10 44");
		hook::set_call(&assignFontLib, location + 32);
		hook::call(location + 32, AssignFontLibWrap);
	}

	OnMainGameFrame.Connect([]()
	{
		UpdateFontLoading();
	});
});
