#include <StdInc.h>
#include <discord-rpc.h>

#include <nutsnbolts.h>

#include <NetLibrary.h>

static bool g_richPresenceChanged;

static std::string g_richPresenceTemplate;

static std::string g_richPresenceValues[8];

static void UpdatePresence()
{
	static time_t startTime = time(nullptr);

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

		DiscordRichPresence discordPresence;
		memset(&discordPresence, 0, sizeof(discordPresence));
		discordPresence.state = line1.c_str();
		discordPresence.details = line2.c_str();
		discordPresence.startTimestamp = startTime;
		discordPresence.largeImageKey = "fivem_large";
		Discord_UpdatePresence(&discordPresence);

		g_richPresenceChanged = false;
	}
}

static InitFunction initFunction([]()
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	Discord_Initialize("382624125287399424", &handlers, 1, nullptr);

	OnRichPresenceSetTemplate.Connect([](const std::string& text)
	{
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
});
