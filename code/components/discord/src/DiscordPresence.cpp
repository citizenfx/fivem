#include <StdInc.h>
#include <discord_rpc.h>

#include <optional>
#include <nutsnbolts.h>

#include <GameInit.h>
#include <NetLibrary.h>

#include <ScriptEngine.h>
#include <scrEngine.h>

#define DEFAULT_APP_ID "382624125287399424"
#define DEFAULT_APP_ASSET "fivem_large"
#define DEFAULT_APP_ASSET_SMALL ""
#define DEFAULT_APP_ASSET_TEXT ""
#define DEFAULT_APP_ASSET_SMALL_TEXT ""

static bool g_richPresenceChanged;

static std::string g_richPresenceTemplate;

static std::string g_richPresenceValues[8];

static std::string g_richPresenceOverride;

static std::string g_richPresenceOverrideAsset;

static std::string g_richPresenceOverrideAssetSmall;

static std::string g_richPresenceOverrideAssetText;

static std::string g_richPresenceOverrideAssetSmallText;

static time_t g_startTime = time(nullptr);

static std::string g_discordAppId;

static std::string g_lastDiscordAppId = DEFAULT_APP_ID;

static std::string g_discordAppAsset;

static std::string g_discordAppAssetSmall;

static std::string g_discordAppAssetText;

static std::string g_discordAppAssetSmallText;

static std::optional<std::tuple<std::pair<std::string, std::string>, std::pair<std::string, std::string>>> g_buttons;

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

		if (!g_richPresenceOverrideAssetSmall.empty())
		{
			discordPresence.smallImageKey = g_richPresenceOverrideAssetSmall.c_str();
		}
		else
		{
			discordPresence.smallImageKey = g_discordAppAssetSmall.c_str();
		}

		if (!g_richPresenceOverrideAssetText.empty())
		{
			discordPresence.largeImageText = g_richPresenceOverrideAssetText.c_str();
		}
		else
		{
			discordPresence.largeImageText = g_discordAppAssetText.c_str();
		}

		if (!g_richPresenceOverrideAssetSmallText.empty())
		{
			discordPresence.smallImageText = g_richPresenceOverrideAssetSmallText.c_str();
		}
		else
		{
			discordPresence.smallImageText = g_discordAppAssetSmallText.c_str();
		}

		if (g_buttons)
		{
			DiscordButton buttons[2];
			buttons[0].label = std::get<0>(*g_buttons).first.c_str();
			buttons[0].url = std::get<0>(*g_buttons).second.c_str();
			buttons[1].label = std::get<1>(*g_buttons).first.c_str();
			buttons[1].url = std::get<1>(*g_buttons).second.c_str();

			discordPresence.buttons = buttons;
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
	g_discordAppAssetSmall = DEFAULT_APP_ASSET_SMALL;
	g_discordAppAssetText = DEFAULT_APP_ASSET_TEXT;
	g_discordAppAssetSmallText = DEFAULT_APP_ASSET_SMALL_TEXT;
	
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
		g_richPresenceOverrideAssetSmall = DEFAULT_APP_ASSET_SMALL;
		g_richPresenceOverrideAssetText = DEFAULT_APP_ASSET_TEXT;
		g_richPresenceOverrideAssetSmallText = DEFAULT_APP_ASSET_SMALL_TEXT;
		g_buttons = {};
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

	fx::ScriptEngine::RegisterNativeHandler("SET_DISCORD_RICH_PRESENCE_ACTION", [](fx::ScriptContext& context)
	{
		int idx = context.GetArgument<int>(0);
		std::string label = context.CheckArgument<const char*>(1);
		std::string url = context.CheckArgument<const char*>(2);

		if (idx < 0 || idx >= 2)
		{
			return;
		}

		if (url.find("https://") != 0 && url.find("fivem://connect/") != 0)
		{
			return;
		}

		std::decay_t<decltype(*g_buttons)> buttons;
		if (g_buttons)
		{
			buttons = *g_buttons;
		}

		if (idx == 0)
		{
			std::get<0>(buttons) = { label, url };
		}
		else if (idx == 1)
		{
			std::get<1>(buttons) = { label, url };
		}

		g_buttons = buttons;
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

	fx::ScriptEngine::RegisterNativeHandler("SET_DISCORD_RICH_PRESENCE_ASSET_SMALL", [](fx::ScriptContext& context)
	{
		const char* str = context.GetArgument<const char*>(0);

		if (str)
		{
			g_richPresenceOverrideAssetSmall = str;
		}
		else
		{
			g_richPresenceOverrideAssetSmall = DEFAULT_APP_ASSET_SMALL;
		}

		g_richPresenceChanged = true;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_DISCORD_RICH_PRESENCE_ASSET_TEXT", [](fx::ScriptContext& context)
	{
		const char* str = context.GetArgument<const char*>(0);

		if (str)
		{
			g_richPresenceOverrideAssetText = str;
		}
		else
		{
			g_richPresenceOverrideAssetText = DEFAULT_APP_ASSET_TEXT;
		}

		g_richPresenceChanged = true;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_DISCORD_RICH_PRESENCE_ASSET_SMALL_TEXT", [](fx::ScriptContext& context)
	{
		const char* str = context.GetArgument<const char*>(0);

		if (str)
		{
			g_richPresenceOverrideAssetSmallText = str;
		}
		else
		{
			g_richPresenceOverrideAssetSmallText = DEFAULT_APP_ASSET_SMALL_TEXT;
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
