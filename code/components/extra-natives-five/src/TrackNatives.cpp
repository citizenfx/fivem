/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ScriptEngine.h>
#include <Hooking.h>
#include <limits>
#include <MinHook.h>
#include <rageVectors.h>
#include "ScriptWarnings.h"
#include <Train.h>

static hook::cdecl_stub<rage::CTrainTrack* (uint32_t)> getTrainTrack([]
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 33 DB 45 0F 57 DB"));
});

static float calculateDistance(const rage::Vector3& point1, const float& x, const float& y, const float& z)
{
	return (point1.x - x) * (point1.x - x) + (point1.y - y) * (point1.y - y) + (point1.z - z) * (point1.z - z);
}

static int32_t FindClosestTrack(rage::Vector3& position, int8_t* outTrack)
{
	*outTrack = -1;
	float closestDistance = std::numeric_limits<float>::infinity();
	int closestNode = -1;

	for (int i = 0; i < rage::CTrainTrack::kMaxTracks; i++)
	{
		rage::CTrainTrack* track = getTrainTrack(i);

		// Skip if this track is a nullptr or is currently disabled. The game doesn't check for this.
		if (!track || !track->m_enabled)
		{
			continue;
		}

		for (int n = 0; n < track->m_nodeCount; n++)
		{
			rage::CTrackNode node = track->m_nodes[n];

			float Distance = calculateDistance(position, node.m_x, node.m_y, node.m_z);

			if (Distance < closestDistance)
			{
				closestNode = n;
				*outTrack = i;
				closestDistance = Distance;
			}
		}
	}

	return closestNode;
}

static HookFunction hookFunction([]()
{
	MH_Initialize();
	// add missing enabled check for script created trains.
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 8B D8 E8 ? ? ? ? 44 8A CF")), FindClosestTrack, NULL);
	MH_EnableHook(MH_ALL_HOOKS);

	// Prevent game code from constantly setting the trains speed while in moving state if it has the "stopsAtStations" flag enabled from setting the train speed to the tracks max speed while moving.
	hook::nop(hook::get_pattern("F3 0F 10 75 ? 8B 55"), 5);
});

static rage::CTrainTrack* getAndCheckTrack(fx::ScriptContext& context, std::string_view nn)
{
	int trackIndex = context.GetArgument<int>(0);
	if (trackIndex < 0 || trackIndex > rage::CTrainTrack::kMaxTracks)
	{
		trace("Invalid track index %i passed to %s\n", trackIndex, nn);
		context.SetResult(0);
		return NULL;
	}

	rage::CTrainTrack* track = getTrainTrack(trackIndex);

	if (!track || track->m_hash == 0)
	{
		trace("Track index %i passed to %s does not exist\n", trackIndex, nn);
		context.SetResult(0);
		return NULL;
	}

	return track;
}

bool rage::CTrainTrack::AreAllTracksDisabled()
{
	for (int i = 0; i < rage::CTrainTrack::kMaxTracks; i++)
	{
		CTrainTrack* track = getTrainTrack(i);

		if (track && track->m_enabled)
		{
			return false;
		}
	}

	return true;
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_TRACK_MAX_SPEED", [](fx::ScriptContext& context)
	{
		int maxSpeed = context.CheckArgument<int>(1);

		if (rage::CTrainTrack* track = getAndCheckTrack(context, "SET_TRACK_MAX_SPEED"))
		{
			track->m_speed = maxSpeed;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TRACK_MAX_SPEED", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = getAndCheckTrack(context, "GET_TRACK_MAX_SPEED"))
		{
			context.SetResult<int>(track->m_speed);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TRACK_BRAKING_DISTANCE", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = getAndCheckTrack(context, "GET_TRACK_BRAKING_DISTANCE"))
		{
			context.SetResult<int>(track->m_brakeDistance);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_TRACK_BRAKING_DISTANCE", [](fx::ScriptContext& context)
	{
		int brakeDistance = context.CheckArgument<int>(1);
		if (rage::CTrainTrack* track = getAndCheckTrack(context, "SET_TRACK_BRAKING_DISTANCE"))
		{
			track->m_brakeDistance = brakeDistance;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_TRACK_ENABLED", [](fx::ScriptContext& context)
	{
		bool state = context.GetArgument<bool>(1);
		if (rage::CTrainTrack* track = getAndCheckTrack(context, "SET_TRACK_ENABLED"))
		{
			track->m_enabled = state;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_TRACK_ENABLED", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = getAndCheckTrack(context, "IS_TRACK_ENABLED"))
		{
			context.SetResult<bool>(track->m_enabled);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_TRACK_SWITCHED_OFF", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = getAndCheckTrack(context, "IS_TRACK_SWITCHED_OFF"))
		{
			context.SetResult<bool>(track->m_disableAmbientTrains);
		}
	});
});
