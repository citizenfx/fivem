#include <StdInc.h>
#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <discord-rpc.h>

#include <nutsnbolts.h>

#include <GameInit.h>
#include <NetLibrary.h>

#include <ScriptEngine.h>

static bool g_richPresenceChanged;

static std::string g_richPresenceTemplate;

static std::string g_richPresenceValues[8];

static std::string g_richPresenceOverride;

static std::string g_richPresenceOverrideAsset;

static time_t g_startTime = time(nullptr);

static std::string DiscordId;

static std::string DiscordImage;

static void UpdatePresence()
{
	if (g_richPresenceChanged)
	{
		std::string formattedRichPresence = fmt::format(g_richPresenceTemplate,
			g_richPresenceValues[0],
			g_richPresenceValues[1],
			g_richPresenceValues[2],
			g_richPresenceValues[3],
			g_richPresenceValues[4],
			g_richPresenceValues[5],
			g_richPresenceValues[6],
			g_richPresenceValues[7]
		);

		std::string line1 = formattedRichPresence.substr(formattedRichPresence.find_first_of("\n") + 1);
		std::string line2 = formattedRichPresence.substr(0, formattedRichPresence.find_first_of("\n"));

		if (!g_richPresenceOverride.empty())
		{
			line1 = g_richPresenceOverride;
		}
		
		DiscordRichPresence discordPresence;
		memset(&discordPresence, 0, sizeof(discordPresence));
		discordPresence.state = line1.c_str();
		discordPresence.details = line2.c_str();
		discordPresence.startTimestamp = g_startTime;
		if (!g_richPresenceOverrideAsset.empty())
		{
		discordPresence.largeImageKey = g_richPresenceOverrideAsset.c_str();
		}
		else {
			discordPresence.largeImageKey = DiscordImage.c_str();
		}
		
		Discord_UpdatePresence(&discordPresence);

		g_richPresenceChanged = false;
	}
}

static InitFunction initFunction([]()
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
	
	if (GetFileAttributes(fpath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		DiscordId = "382624125287399424";
		DiscordImage = "fivem_large";
	}
	else {
		wchar_t path[512];
		const wchar_t* pathKey = L"DiscordId";
		GetPrivateProfileString(L"Game", pathKey, NULL, path, _countof(path), fpath.c_str());
		if (wcscmp(path, L"") == 0)
		{
			DiscordId = "382624125287399424";
		}
		else {
			char DiscordIdS[512];
			wcstombs(DiscordIdS, path, sizeof(DiscordIdS));
			DiscordId = DiscordIdS;
		}
		
		pathKey = L"DiscordImage";
		GetPrivateProfileString(L"Game", pathKey, NULL, path, _countof(path), fpath.c_str());
		if (wcscmp(path, L"") == 0)
		{
			DiscordImage = "fivem_large";
		}
		else {
			char DiscordImageS[512];
			wcstombs(DiscordImageS, path, sizeof(DiscordImageS));
			DiscordImage = DiscordImageS;
		}
		
	}
	Discord_Initialize(DiscordId.c_str(), &handlers, 1, nullptr);

	OnRichPresenceSetTemplate.Connect([](const std::string& text)
	{
		g_startTime = time(nullptr);

		g_richPresenceTemplate = text;

		g_richPresenceChanged = true;
	});

	OnRichPresenceSetValue.Connect([](int idx, const std::string& value)
	{
		assert(idx >= 0 && idx < _countof(g_richPresenceValues));

		g_richPresenceValues[idx] = value;

		g_richPresenceChanged = true;
	});

	OnRichPresenceSetTemplate("In the menus\n");

	OnGameFrame.Connect([]()
	{
		Discord_RunCallbacks();

		UpdatePresence();
	});

	OnKillNetworkDone.Connect([]()
	{
		g_richPresenceOverride = "";

		OnRichPresenceSetTemplate("In the menus\n");
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_RICH_PRESENCE", [](fx::ScriptContext& context)
	{
		const char* str = context.GetArgument<const char*>(0);

		if (str)
		{
			g_richPresenceOverride = str;
		}
		else
		{
			g_richPresenceOverride = "";
		}

		g_richPresenceChanged = true;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_RICH_PRESENCE_ASSET", [](fx::ScriptContext& context)
	{
		const char* str = context.GetArgument<const char*>(0);

		if (str)
		{
			g_richPresenceOverrideAsset = str;
		}
		else
		{
			g_richPresenceOverrideAsset = "";
		}

		g_richPresenceChanged = true;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_RICH_PRESENCE_ID", [](fx::ScriptContext& context)
	{
		const char* str = context.GetArgument<const char*>(0);

		if (str)
		{
			DiscordId = str;
		}
		else
		{
			DiscordId = "382624125287399424";
		}
		Discord_Shutdown();
		DiscordEventHandlers handlers;
		memset(&handlers, 0, sizeof(handlers));
		Discord_Initialize(DiscordId.c_str(), &handlers, 1, nullptr);

		g_richPresenceChanged = true;
	});
});
