/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ScriptEngine.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <limits>
#include <MinHook.h>
#include <rageVectors.h>
#include "ScriptWarnings.h"
#include <Train.h>
#include <GameInit.h>

static hook::cdecl_stub<rage::CTrainTrack* (uint32_t)> getTrainTrack([]
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 33 DB 45 0F 57 DB"));
});

static float calculateDistance(const rage::Vector3& point1, const float& x, const float& y, const float& z)
{
	return (point1.x - x) * (point1.x - x) + (point1.y - y) * (point1.y - y) + (point1.z - z) * (point1.z - z);
}

static float calculateDistance(const float& x, const float& y, const float& z, const float& x1, const float& y1, const float& z1)
{
	return (x - x1) * (x - x1) + (y - y1) * (y - y1) + (z - z1) * (z - z1);
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

struct CTrainJunction
{
	uint8_t onTrack;
	int onNode;

	uint8_t newTrack;
	int newNode;

	bool direction;
	bool isActive;

	CTrainJunction(uint8_t track, int node, uint8_t newTrack, int newNode, bool direction) : onTrack(track), onNode(node), newTrack(newTrack), newNode(newNode), direction(direction), isActive(true)
	{ }
};

static std::vector<CTrainJunction> g_trackSwaps;

class CTrain : public CVehicle
{
public:
	inline static ptrdiff_t kTrackIndexOffset;
	inline static ptrdiff_t kTrackNodeOffset;
	inline static ptrdiff_t kTrainFlagsOffset;
public:
	inline int8_t GetTrackIndex()
	{
		auto location = reinterpret_cast<uint8_t*>(this) + kTrackIndexOffset;
		return *reinterpret_cast<int8_t*>(location);
	}

	inline uint32_t GetTrackNode()
	{
		auto location = reinterpret_cast<uint8_t*>(this) + kTrackNodeOffset;
		return *reinterpret_cast<uint32_t*>(location);
	}

	inline bool GetDirection()
	{
		auto location = reinterpret_cast<uint8_t*>(this) + kTrainFlagsOffset;
		return (*(BYTE*)(this + kTrainFlagsOffset) & 8) != 0;
	}

	inline void SetTrackIndex(int8_t trackIndex)
	{
		auto location = reinterpret_cast<uint8_t*>(this) + kTrackIndexOffset;
		*(int8_t*)location = trackIndex;
	}

	inline void SetTrackNode(uint32_t trackNode)
	{
		auto location = reinterpret_cast<uint8_t*>(this) + kTrackNodeOffset;
		*(uint32_t*)location = trackNode;
	}
};

static hook::cdecl_stub<void(CVehicle*, int, int)> CTrain__SetTrainCoord([]()
{
	return hook::pattern("44 8B C2 48 83 C4 ? 5B").count(1).get(0).get<void>(8);
});

static hook::cdecl_stub<CTrain* (CTrain*)> CTrain__GetBackwardCarriage([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 39 03"));
});

static bool (*g_CTrain__Update)(CTrain*, float);
static bool CTrain__Update(CTrain* self, float unk)
{
	for (const auto& data : g_trackSwaps)
	{
		if (!data.isActive)
		{
			continue;
		}

		if (self->GetTrackIndex() == data.onTrack
			&& self->GetTrackNode() == data.onNode
			&& self->GetDirection() == data.direction)
		{
			// Iterate through all carriages to ensure the new trackIndex and node are applied
			CTrain* train;
			for (train = self; train; train = CTrain__GetBackwardCarriage(train))
			{
				train->SetTrackIndex(data.newTrack);
				train->SetTrackNode(data.newNode);
			}

			// Force the train to update coordinate
			// The original update function call should smooth over the rest of the transition
			CTrain__SetTrainCoord(self, data.newNode, -1);
		}
	}

	return g_CTrain__Update(self, unk);
}


static HookFunction hookFunction([]()
{
	MH_Initialize();
	// add missing enabled check for script created trains.
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 8B D8 E8 ? ? ? ? 44 8A CF")), FindClosestTrack, NULL);
	MH_EnableHook(MH_ALL_HOOKS);

	// Prevent game code from constantly setting the trains speed while in moving state if it has the "stopsAtStations" flag enabled from setting the train speed to the tracks max speed while moving.
	hook::nop(hook::get_pattern("F3 0F 10 75 ? 8B 55"), 5);
	// Extend metro vehicle types check to all vehicles 
	hook::put<uint8_t>(hook::get_pattern("83 BE ? ? ? ? ? 77 ? 44 21 65", 6), 0xF);

	{
		CTrain::kTrackNodeOffset = *hook::get_pattern<uint32_t>("E8 ? ? ? ? 40 8A F8 84 C0 75 ? 48 8B CB E8", -4);
		CTrain::kTrackIndexOffset = *hook::get_pattern<uint32_t>("88 87 ? ? ? ? 48 85 F6 75", 2);
		CTrain::kTrainFlagsOffset = *hook::get_pattern<uint32_t>("80 8B ? ? ? ? ? 8B 05 ? ? ? ? FF C8", 2);
	}

	g_CTrain__Update = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 44 8A B5 ? ? ? ? 48 85 F6")), CTrain__Update);

	OnKillNetworkDone.Connect([]()
	{
		g_trackSwaps.clear();
	});
});

template<int ArgumentIndex>
static rage::CTrainTrack* getAndCheckTrack(fx::ScriptContext& context, std::string_view nn)
{
	int trackIndex = context.GetArgument<int>(ArgumentIndex);
	if (trackIndex < 0 || trackIndex > rage::CTrainTrack::kMaxTracks)
	{
		fx::scripting::Warningf("natives", "%s: Invalid track index %i", nn, trackIndex);
		context.SetResult(0);
		return NULL;
	}

	rage::CTrainTrack* track = getTrainTrack(trackIndex);

	if (!track || track->m_hash == 0)
	{
		fx::scripting::Warningf("natives", "%s: Track Index %i does not exist", nn, trackIndex);
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

		if (rage::CTrainTrack* track = getAndCheckTrack<0>(context, "SET_TRACK_MAX_SPEED"))
		{
			track->m_speed = maxSpeed;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TRACK_MAX_SPEED", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = getAndCheckTrack<0>(context, "GET_TRACK_MAX_SPEED"))
		{
			context.SetResult<int>(track->m_speed);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TRACK_BRAKING_DISTANCE", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = getAndCheckTrack<0>(context, "GET_TRACK_BRAKING_DISTANCE"))
		{
			context.SetResult<int>(track->m_brakeDistance);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_TRACK_BRAKING_DISTANCE", [](fx::ScriptContext& context)
	{
		int brakeDistance = context.CheckArgument<int>(1);
		if (rage::CTrainTrack* track = getAndCheckTrack<0>(context, "SET_TRACK_BRAKING_DISTANCE"))
		{
			track->m_brakeDistance = brakeDistance;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_TRACK_ENABLED", [](fx::ScriptContext& context)
	{
		bool state = context.GetArgument<bool>(1);
		if (rage::CTrainTrack* track = getAndCheckTrack<0>(context, "SET_TRACK_ENABLED"))
		{
			track->m_enabled = state;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_TRACK_ENABLED", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = getAndCheckTrack<0>(context, "IS_TRACK_ENABLED"))
		{
			context.SetResult<bool>(track->m_enabled);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_TRACK_SWITCHED_OFF", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = getAndCheckTrack<0>(context, "IS_TRACK_SWITCHED_OFF"))
		{
			context.SetResult<bool>(track->m_disableAmbientTrains);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_TRACK_SWITCH", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = getAndCheckTrack<0>(context, "REGISTER_TRACK_SWITCH"))
		{
			int trackNode = context.GetArgument<int>(1);
			int newTrackNode = context.GetArgument<int>(3);

			if (track->m_nodeCount < trackNode)
			{
				fx::scripting::Warningf("natives", "REGISTER_TRACK_SWITCH: Invalid onTrackNode was passed (%i), should be from 0 to %i\n", trackNode, track->m_nodeCount);
				context.SetResult<int>(-1);
				return;
			}

			// get the track we want to register the switch to 
			rage::CTrainTrack* newTrack = getAndCheckTrack<2>(context, "REGISTER_TRACK_SWITCH");

			if (!newTrack)
			{
				fx::scripting::Warningf("natives", "REGISTER_TRACK_SWITCH: Track (%i) is invalid\n", context.GetArgument<uint8_t>(2));
				context.SetResult<int>(-1);
				return;
			}

			if (newTrack->m_nodeCount < newTrackNode)
			{
				fx::scripting::Warningf("natives", "REGISTER_TRACK_SWITCH: Invalid toNode was passed (%i), should be from 0 to %i\n", newTrackNode, newTrack->m_nodeCount);
				context.SetResult<int>(-1);
				return;
			}

			rage::CTrackNode onNode = track->m_nodes[trackNode];
			rage::CTrackNode toNode = newTrack->m_nodes[newTrackNode];

			float dist = calculateDistance(onNode.m_x, onNode.m_y, onNode.m_z, toNode.m_x, toNode.m_y, toNode.m_z);

			if (dist > 15.0f)
			{
				fx::scripting::Warningf("natives", "REGISTER_TRACK_SWITCH: the specified track nodes must overlap each other\n");
				context.SetResult<int>(-1);
				return;
			}

			// Ensure we don't allow duplicates
			uint8_t trackIndex = context.GetArgument<uint8_t>(0);
			uint8_t swapIndex = context.GetArgument<uint8_t>(2);

			bool direction = context.GetArgument<bool>(4);

			for (const auto& data : g_trackSwaps)
			{
				if (data.direction == direction &&
					data.onTrack == trackIndex &&
					data.newTrack == swapIndex &&
					data.onNode == trackNode &&
					data.newNode == newTrackNode)
				{
					fx::scripting::Warningf("natives", "REGISTER_TRACK_SWITCH: Cannot register duplicate track switches");
					context.SetResult<int>(-1);
					return;
				}
			}

			context.SetResult<int>(g_trackSwaps.size());
			g_trackSwaps.push_back(CTrainJunction(trackIndex, trackNode, swapIndex, newTrackNode, direction));
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_TRACK_SWITCH_ACTIVE", [](fx::ScriptContext& context)
	{
		int index = context.GetArgument<int>(0);
		bool state = context.GetArgument<bool>(1);

		if (index >= g_trackSwaps.size() || index < 0)
		{
			fx::scripting::Warningf("natives", "SET_TRACK_SWITCH_ACTIVE: attempted to set state of an invalid track switch (%i)\n", index);
			return;
		}

		g_trackSwaps[index].isActive = state;
	});
});
