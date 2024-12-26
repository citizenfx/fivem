#pragma once

#include <atArray.h>

namespace rage
{
struct CTrackNode
{
public:
	float m_x, m_y, m_z;
	float m_unk;
	uint8_t m_unk1;
	int m_station;
};

struct CTrainTrack
{
public:
	// This has never changed.
	static const int kMaxTracks = 27;

	uint32_t m_hash;
	bool m_enabled;
	bool m_isLooped;
	bool m_stopsAtStation;
	bool m_MPStopsAtStation;
	uint32_t m_speed;
	uint32_t m_brakeDistance;

	int m_nodeCount;
	CTrackNode* m_nodes;

	uint8_t m_pad[8];

	bool m_disableAmbientTrains;
	uint8_t m_pad2[0x220];

	// Helper functions
	static bool AreAllTracksDisabled();
};

struct CTrainConfig
{
	struct CarriageData
	{
		uint32_t m_hash;
		uint8_t m_pad[0xE];
	};

	uint32_t m_hash;
	uint8_t m_pad[0x1A];
	atArray<CarriageData> m_carriages;
};

struct CTrainConfigData
{
	atArray<CTrainConfig> m_trainConfigs;
};
}
