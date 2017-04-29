#include "StdInc.h"
#include "HudPos.h"

HudPosition* HudPositions::ms_hudPositions = (HudPosition*)0x11DD6F8;

HudPosition* HudPositions::GetPosition(int index)
{
	return &ms_hudPositions[index];
}

static bool g_listInitialized;
static std::unordered_map<std::string, int> g_indexList;

HudPosition* HudPositions::GetPosition(const char* name)
{
	if (!g_listInitialized)
	{
		g_indexList["HUD_BIG_MESSAGE_COMPLETE"] = 0;
		g_indexList["HUD_BIG_MESSAGE_TITLE"] = 1;
		g_indexList["HUD_BIG_MESSAGE_WASTED"] = 2;
		g_indexList["HUD_BIG_MESSAGE_4"] = 3;
		g_indexList["HUD_BIG_MESSAGE_5"] = 4;
		g_indexList["HUD_BIG_MESSAGE_6"] = 5;
		g_indexList["HUD_BIG_MESSAGE_7"] = 6;
		g_indexList["HUD_SUBITILES"] = 7;
		g_indexList["HUD_RADAR"] = 8;
		g_indexList["HUD_RADAR_BLIP_SIZE"] = 9;
		g_indexList["HUD_MAP_BLIP_SIZE"] = 10;
		g_indexList["HUD_HELP_MESSAGE"] = 11;
		g_indexList["HUD_HELP_MESSAGE_ICON"] = 12;
		g_indexList["HUD_LOADING_BAR"] = 13;
		g_indexList["HUD_MP_NAME_TEXT"] = 14;
		g_indexList["HUD_MP_NAME_ICON"] = 15;
		g_indexList["HUD_CUTSCENE_BATTERY"] = 16;
		g_indexList["HUD_CUTSCENE_REC"] = 17;
		g_indexList["HUD_CUTSCENE_CORNER"] = 18;
		g_indexList["HUD_CUTSCENE_METER"] = 19;
		g_indexList["HUD_CREDITS_JOB_BIG"] = 20;
		g_indexList["HUD_CREDITS_JOB_MED"] = 21;
		g_indexList["HUD_CREDITS_JOB_SMALL"] = 22;
		g_indexList["HUD_CREDITS_NAME_BIG"] = 23;
		g_indexList["HUD_CREDITS_NAME_MED"] = 24;
		g_indexList["HUD_CREDITS_NAME_SMALL"] = 25;
		g_indexList["HUD_CREDITS_SPACE_BIG"] = 26;
		g_indexList["HUD_CREDITS_SPACE_MED"] = 27;
		g_indexList["HUD_CREDITS_SPACE_SMALL"] = 28;
		g_indexList["HUD_MP_CLOCK"] = 29;
		g_indexList["HUD_MP_CASH"] = 30;
		g_indexList["HUD_REPLAY_MOUSE"] = 31;
		g_indexList["HUD_WANTED_FRONT"] = 32;
		g_indexList["HUD_WANTED_BACK"] = 33;
		g_indexList["HUD_CASH"] = 34;
		g_indexList["HUD_MISSION_PASSED_CASH"] = 35;
		g_indexList["HUD_AMMO"] = 36;
		g_indexList["HUD_WEAPON_ICON"] = 37;
		g_indexList["HUD_AREA_NAME"] = 38;
		g_indexList["HUD_STREET_NAME"] = 39;
		g_indexList["HUD_VEHICLE_NAME"] = 40;
		g_indexList["HUD_PHONE_MESSAGE_ICON"] = 41;
		g_indexList["HUD_PHONE_MESSAGE_NUMBER"] = 42;
		g_indexList["HUD_PHONE_MESSAGE_BOX"] = 43;
		g_indexList["HUD_TEXT_MESSAGE_ICON"] = 44;
		g_indexList["HUD_SLEEP_MODE_ICON"] = 45;
		g_indexList["HUD_WEAPON_CROSSHAIR"] = 46;
		g_indexList["HUD_WEAPON_HEALTH_TARGET"] = 47;
		g_indexList["HUD_WEAPON_ARMOUR_TARGET"] = 48;
		g_indexList["HUD_WEAPON_DOT"] = 49;
		g_indexList["HUD_MISSION_CLOCK"] = 50;
		g_indexList["HUD_MISSION_COUNTER_NAME_1"] = 51;
		g_indexList["HUD_MISSION_COUNTER_NUMBER_1"] = 52;
		g_indexList["HUD_MISSION_COUNTER_NAME_2"] = 53;
		g_indexList["HUD_MISSION_COUNTER_NUMBER_2"] = 54;
		g_indexList["HUD_MISSION_COUNTER_NAME_3"] = 55;
		g_indexList["HUD_MISSION_COUNTER_NUMBER_3"] = 56;
		g_indexList["HUD_MISSION_COUNTER_NAME_4"] = 57;
		g_indexList["HUD_MISSION_COUNTER_NUMBER_4"] = 58;
		g_indexList["HUD_WEAPON_SCOPE"] = 59;
		g_indexList["HUD_WEAPON_SCOPE_BIT_1"] = 60;
		g_indexList["HUD_WEAPON_SCOPE_BIT_2"] = 61;
		g_indexList["HUD_REPLAY_CONTROLLER"] = 62;
		g_indexList["HUD_LOWERRIGHT_MESSAGE"] = 63;
	}

	auto it = g_indexList.find(name);

	if (it == g_indexList.end())
	{
		return nullptr;
	}

	return GetPosition(it->second);
}