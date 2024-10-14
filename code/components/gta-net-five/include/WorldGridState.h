#pragma once

namespace fx
{
struct WorldGridEntry
{
	uint8_t sectorX;
	uint8_t sectorY;
	uint16_t slotID;
};

struct WorldGridState
{
	WorldGridEntry entries[32];
};

static WorldGridState g_worldGrid;
}
