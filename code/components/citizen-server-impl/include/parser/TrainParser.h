#pragma once

#include "Parser.h"

namespace fx::data
{
	struct CTrainCarriageInfo
	{
		const char* m_modelName;

		uint32_t m_maxPedsPerCarriage;
		bool m_flipModelDir;
		bool m_doInteriorLights;
		bool m_carriageVertOffset;
		uint32_t m_repeatCount;
	};

	struct CTrainConfig
	{
		const char* m_name;
		float m_populateTrainDist;
		bool m_announceStations;
		bool m_doorsBeep;
		bool m_carriagesHang;
		bool m_carriagesSwing;
		bool m_carriagesUseEvenTrackSpacing;
		bool m_linkTracksWithAdjacentStations;
		bool m_noRandomSpawns;
		float m_carriageGap;

		std::vector<CTrainCarriageInfo> m_carriages;
	};

	struct CTrainTrack
	{
		const char* m_trainConfigName;
		bool m_isPingPongTrack;
		bool m_stopsAtStations;
		bool m_MPStopsAtStations;
		int32_t m_speed;
		int32_t m_breakingDist;
	};
}

namespace fx
{
	class CTrainConfigParser : public fwRefCountable, public data::Parser<std::vector<data::CTrainConfig>>
	{
		bool ParseFile(tinyxml2::XMLDocument& document) override
		{
			tinyxml2::XMLElement* configs = document.FirstChildElement("train_configs");

			if (!configs)
			{
				return false;
			}

			std::vector<data::CTrainConfig> trainConfigs;
			tinyxml2::XMLElement* config = configs->FirstChildElement("train_config");

			if (config == nullptr)
			{
				return false;
			}

			while (config)
			{
				data::CTrainConfig trainConfig{};
				trainConfig.m_name = config->Attribute("name");
				trainConfig.m_populateTrainDist = config->IntAttribute("populate_train_dist", 0);
				trainConfig.m_announceStations = config->BoolAttribute("announce_stations", false);
				trainConfig.m_doorsBeep = config->BoolAttribute("doors_beep", false);
				trainConfig.m_carriagesHang = config->BoolAttribute("carriages_hang", false);
				trainConfig.m_carriagesSwing = config->BoolAttribute("carriages_swing", false);
				trainConfig.m_linkTracksWithAdjacentStations = config->BoolAttribute("link_tracks_with_adjacent_stations", true);
				trainConfig.m_noRandomSpawns = config->BoolAttribute("no_random_spawn", true);
				trainConfig.m_carriageGap = config->FloatAttribute("carriage_gap", 0.1);

				tinyxml2::XMLElement* carriage = config->FirstChildElement("carriage");
				while (carriage)
				{
					data::CTrainCarriageInfo carriageInfo{};
					carriageInfo.m_modelName = config->Attribute("model_name");
					carriageInfo.m_maxPedsPerCarriage = config->IntAttribute("max_peds_per_carriage", 0);
					carriageInfo.m_flipModelDir = config->BoolAttribute("flip_model_dir", false);
					carriageInfo.m_doInteriorLights = config->BoolAttribute("do_interior_lights", false);
					carriageInfo.m_carriageVertOffset = config->FloatAttribute("carriage_vert_offset", 1.0);
					carriageInfo.m_repeatCount = config->IntAttribute("repeat_count", 1);

					trainConfig.m_carriages.push_back(carriageInfo);

					carriage = carriage->NextSiblingElement("carriage");
				}

				trainConfigs.push_back(trainConfig);
				config = config->NextSiblingElement("train_config");
			}

			m_data = trainConfigs;
			return true;
		}
	};

	class CTrainTrackParser : public fwRefCountable, public data::Parser<std::vector<data::CTrainTrack>>
	{
		bool ParseFile(tinyxml2::XMLDocument& document)
		{
			tinyxml2::XMLElement* tracks = document.FirstChildElement("train_tracks");

			if (!tracks)
			{
				return false;
			}

			std::vector<data::CTrainTrack> trainTracks;
			tinyxml2::XMLElement* track = tracks->FirstChildElement("train_track");

			if (track == nullptr)
			{
				return false;
			}

			while (track)
			{
				data::CTrainTrack trainTrack{};
				trainTrack.m_trainConfigName = track->Attribute("trainConfigName");
				trainTrack.m_isPingPongTrack = track->BoolAttribute("isPingPongTrack", false);
				trainTrack.m_stopsAtStations = track->BoolAttribute("stopsAtStations", false);
				trainTrack.m_MPStopsAtStations = track->BoolAttribute("MPstopsAtStations", false);
				trainTrack.m_speed = track->IntAttribute("speed", 8);
				trainTrack.m_breakingDist = track->IntAttribute("brakingDist", 10);

				trainTracks.push_back(trainTrack);
				track = track->NextSiblingElement("train_track");
			}

			m_data = trainTracks;
			return true;
		}
	};
}

DECLARE_INSTANCE_TYPE(fx::CTrainConfigParser);
DECLARE_INSTANCE_TYPE(fx::CTrainTrackParser);
