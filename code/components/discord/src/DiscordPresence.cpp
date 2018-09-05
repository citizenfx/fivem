#include <StdInc.h>
#include <discord-rpc.h>

#include <nutsnbolts.h>

#include <GameInit.h>
#include <NetLibrary.h>

#include <ScriptEngine.h>
#include <scrEngine.h>

#define DEFAULT_APP_ID "382624125287399424"
#define DEFAULT_APP_ASSET "fivem_large"

static bool g_richPresenceChanged;

static std::string g_richPresenceTemplate;

static std::string g_richPresenceValues[8];

static std::string g_richPresenceOverride;

static std::string g_richPresenceOverrideAsset;

static time_t g_startTime = time(nullptr);

static std::string g_discordAppId;

static std::string g_lastDiscordAppId = DEFAULT_APP_ID;

static std::string g_discordAppAsset;

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

		if (g_discordAppId != g_lastDiscordAppId) 
		{
			g_lastDiscordAppId = g_discordAppId;
			Discord_Shutdown();
			DiscordEventHandlers handlers;
			memset(&handlers, 0, sizeof(handlers));
			Discord_Initialize(g_discordAppId.c_str(), &handlers, 1, nullptr);
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
		else 
		{
			discordPresence.largeImageKey = g_discordAppAsset.c_str();
		}
		
		Discord_UpdatePresence(&discordPresence);

		g_richPresenceChanged = false;
	}
}

static InitFunction initFunction([]()
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	g_discordAppId = DEFAULT_APP_ID;
	g_discordAppAsset = DEFAULT_APP_ASSET;
	
	Discord_Initialize(g_discordAppId.c_str(), &handlers, 1, nullptr);

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
		g_discordAppId = DEFAULT_APP_ID;
		g_richPresenceOverrideAsset = DEFAULT_APP_ASSET;
		g_richPresenceChanged = true;

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

	fx::ScriptEngine::RegisterNativeHandler("SET_DISCORD_RICH_PRESENCE_ASSET", [](fx::ScriptContext& context)
	{
		const char* str = context.GetArgument<const char*>(0);

		if (str)
		{
			g_richPresenceOverrideAsset = str;
		}
		else
		{
			g_richPresenceOverrideAsset = DEFAULT_APP_ASSET;
		}

		g_richPresenceChanged = true;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_DISCORD_APP_ID", [](fx::ScriptContext& context)
	{
		const char* str = context.GetArgument<const char*>(0);

		if (str)
		{
			g_discordAppId = str;
		}
		else
		{
			g_discordAppId = DEFAULT_APP_ID;
		}

		g_richPresenceChanged = true;
	});
});
