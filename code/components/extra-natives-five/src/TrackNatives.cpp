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
#include <scrEngine.h>
#include <ScriptSerialization.h>
#include <queue>

class CTrain : public CVehicle
{
public:
	inline static ptrdiff_t kTrackIndexOffset;
	inline static ptrdiff_t kTrackNodeOffset;
	inline static ptrdiff_t kTrainFlagsOffset;
	inline static ptrdiff_t kTrainSpeedOffset;
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
		return (*(location) & 8) != 0;
	}

	inline bool IsEngine()
	{
		auto location = reinterpret_cast<uint8_t*>(this) + kTrainFlagsOffset;
		return (*(location) & 2) != 0;
	}

	inline float GetVelocity()
	{
		auto location = reinterpret_cast<uint8_t*>(this) + kTrainSpeedOffset;
		return *(float*)location;
	}

	inline void SetVelocity(float newVelocity)
	{
		auto location = reinterpret_cast<uint8_t*>(this) + kTrainSpeedOffset;
		*(float*)location = newVelocity;
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

struct CTrainJunction
{
	int8_t onTrack;
	uint32_t onNode;

	int8_t newTrack;
	uint32_t newNode;

	bool direction;
	bool isActive;

	CTrainJunction()
		: onTrack(0), onNode(0), newTrack(0), newNode(0), direction(false), isActive(false)
	{
	}

	CTrainJunction(int8_t track, uint32_t node, int8_t newTrack, uint32_t newNode, bool direction)
		: onTrack(track), onNode(node), newTrack(newTrack), newNode(newNode), direction(direction), isActive(true)
	{
	}

	~CTrainJunction()
	{
	}
};

struct scrTrackNodeInfo
{
	int32_t nodeIndex;
	int8_t trackId;

	MSGPACK_DEFINE_ARRAY(nodeIndex, trackId)
};

static std::unordered_map<int,CTrainJunction> g_trackJunctions;
static std::queue<int> g_freeTrackJunctions;
static std::mutex g_trackJunctionLock;
static const int g_maxTrackJunctions = 8912; // If this needs increasing, wtf are you doing?

static bool IsTrackJunctionIdValid(int junctionIndex)
{
	return junctionIndex > 0 && junctionIndex < g_maxTrackJunctions && g_trackJunctions.find(junctionIndex) != g_trackJunctions.end();
}

static float CalculateDistance(const float& x, const float& y, const float& z, const float& x1, const float& y1, const float& z1)
{
	return (x - x1) * (x - x1) + (y - y1) * (y - y1) + (z - z1) * (z - z1);
}

static hook::cdecl_stub<rage::CTrainTrack*(uint32_t)> CTrainTrack__getTrainTrack([]
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 33 DB 45 0F 57 DB"));
});

static int32_t FindClosestTrack(rage::Vector3& position, int8_t* outTrack)
{
	*outTrack = -1;
	float closestDistance = std::numeric_limits<float>::infinity();
	int closestNode = -1;

	for (int i = 0; i < rage::CTrainTrack::kMaxTracks; i++)
	{
		rage::CTrainTrack* track = CTrainTrack__getTrainTrack(i);

		if (!track || !track->m_enabled)
		{
			continue;
		}

		for (int n = 0; n < track->m_nodeCount; n++)
		{
			rage::CTrackNode node = track->m_nodes[n];

			float distance = CalculateDistance(position.x, position.y, position.z, node.m_x, node.m_y, node.m_z);

			if (distance < closestDistance)
			{
				closestNode = n;
				*outTrack = i;
				closestDistance = distance;
			}
		}
	}

	return closestNode;
}

static std::vector<scrTrackNodeInfo> GetTrackNodesInRadius(const float& x, const float& y, const float& z, float radius, bool includeDisabledTracks = false)
{
	std::vector<scrTrackNodeInfo> nearbyNodes;
	for (int8_t i = 0; i < rage::CTrainTrack::kMaxTracks; i++)
	{
		rage::CTrainTrack* track = CTrainTrack__getTrainTrack(i);

		if (!track || (includeDisabledTracks && !track->m_enabled))
		{
			continue;
		}

		for (int n = 0; n < track->m_nodeCount; n++)
		{
			rage::CTrackNode node = track->m_nodes[n];

			float distance = CalculateDistance(x, y, z, node.m_x, node.m_y, node.m_z);

			if (distance <= radius)
			{
				nearbyNodes.push_back({ n, i });
			}
		}
	}
	return nearbyNodes;
}

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
	if (!self->IsEngine())
	{
		return g_CTrain__Update(self, unk);
	}

	const std::lock_guard _(g_trackJunctionLock);
	for (const auto& [_, data] : g_trackJunctions)
	{
		if (!data.isActive)
		{
			continue;
		}

		if (self->GetTrackIndex() == data.onTrack
			&& self->GetTrackNode() == data.onNode
			&& self->GetDirection() == data.direction)
		{
			float velocity = self->GetVelocity();

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
			// Keep the train's velocity the same prior to switching tracks. Not doing so may cause negative velocity in some instances.
			self->SetVelocity(velocity);
			break;
		}
	}

	return g_CTrain__Update(self, unk);
}

template<int ArgumentIndex>
static rage::CTrainTrack* GetAndCheckTrack(fx::ScriptContext& context, std::string_view nn)
{
	int trackIndex = context.GetArgument<int>(ArgumentIndex);
	if (trackIndex < 0 || trackIndex > rage::CTrainTrack::kMaxTracks)
	{
		fx::scripting::Warningf("natives", "%s: Invalid track index %i\n", nn, trackIndex);
		context.SetResult(0);
		return NULL;
	}

	rage::CTrainTrack* track = CTrainTrack__getTrainTrack(trackIndex);

	if (!track || track->m_hash == 0)
	{
		fx::scripting::Warningf("natives", "%s: Track Index %i does not exist\n", nn, trackIndex);
		context.SetResult(0);
		return NULL;
	}

	return track;
}

static rage::CTrackNode* GetAndCheckTrackNode(rage::CTrainTrack* track, int8_t trackIndex, uint32_t trackNodeIndex, std::string_view nn)
{
	if (trackNodeIndex >= track->m_nodeCount)
	{
		fx::scripting::Warningf("natives", "%s: Invalid track node (%i) on track (%i), should be from 0 to %i\n", nn, trackNodeIndex, trackIndex, track->m_nodeCount - 1);
		return NULL;
	}

	return &track->m_nodes[trackNodeIndex];
}

bool rage::CTrainTrack::AreAllTracksDisabled()
{
	for (int i = 0; i < rage::CTrainTrack::kMaxTracks; i++)
	{
		CTrainTrack* track = CTrainTrack__getTrainTrack(i);

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

		if (rage::CTrainTrack* track = GetAndCheckTrack<0>(context, "SET_TRACK_MAX_SPEED"))
		{
			track->m_speed = maxSpeed;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TRACK_MAX_SPEED", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = GetAndCheckTrack<0>(context, "GET_TRACK_MAX_SPEED"))
		{
			context.SetResult<int>(track->m_speed);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TRACK_BRAKING_DISTANCE", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = GetAndCheckTrack<0>(context, "GET_TRACK_BRAKING_DISTANCE"))
		{
			context.SetResult<int>(track->m_brakeDistance);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_TRACK_BRAKING_DISTANCE", [](fx::ScriptContext& context)
	{
		int brakeDistance = context.CheckArgument<int>(1);
		if (rage::CTrainTrack* track = GetAndCheckTrack<0>(context, "SET_TRACK_BRAKING_DISTANCE"))
		{
			track->m_brakeDistance = brakeDistance;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_TRACK_ENABLED", [](fx::ScriptContext& context)
	{
		bool state = context.GetArgument<bool>(1);
		if (rage::CTrainTrack* track = GetAndCheckTrack<0>(context, "SET_TRACK_ENABLED"))
		{
			track->m_enabled = state;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_TRACK_ENABLED", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = GetAndCheckTrack<0>(context, "IS_TRACK_ENABLED"))
		{
			context.SetResult<bool>(track->m_enabled);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_TRACK_SWITCHED_OFF", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = GetAndCheckTrack<0>(context, "IS_TRACK_SWITCHED_OFF"))
		{
			context.SetResult<bool>(track->m_disableAmbientTrains);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ALL_TRACK_JUNCTIONS", [](fx::ScriptContext& context)
	{
		std::vector<int> junctionList;
		const std::lock_guard _(g_trackJunctionLock);

		for (auto& [index, _] : g_trackJunctions)
		{
			junctionList.push_back(index);
		}

		context.SetResult(fx::SerializeObject(junctionList));
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_TRACK_JUNCTION", [](fx::ScriptContext& context)
	{
		if (g_freeTrackJunctions.empty() || g_trackJunctions.size() >= g_maxTrackJunctions)
		{
			fx::scripting::Warningf("natives", "REGISTER_TRACK_JUNCTION: Reached the maximum track junctions limit (%i)\n", g_maxTrackJunctions);
			context.SetResult<int>(-1);
			return;
		}

		int8_t trackIndex = context.GetArgument<int8_t>(0);
		uint32_t trackNode = context.GetArgument<uint32_t>(1);

		int8_t newTrackIndex = context.GetArgument<int8_t>(2);
		uint32_t newTrackNode = context.GetArgument<uint32_t>(3);

		bool direction = context.GetArgument<bool>(4);

		rage::CTrainTrack* track = GetAndCheckTrack<0>(context, "REGISTER_TRACK_JUNCTION");
		rage::CTrainTrack* newTrack = GetAndCheckTrack<2>(context, "REGISTER_TRACK_JUNCTION");

		if (!track || !newTrack)
		{
			context.SetResult<int>(-1);
			return;
		}

		rage::CTrackNode* fromNode = GetAndCheckTrackNode(track, trackIndex, trackNode, "REGISTER_TRACK_JUNCTION");
		rage::CTrackNode* toNode = GetAndCheckTrackNode(newTrack, newTrackIndex, newTrackNode, "REGISTER_TRACK_JUNCTION");

		if (!fromNode || !toNode)
		{
			context.SetResult<int>(-1);
			return;
		}

		float dist = CalculateDistance(fromNode->m_x, fromNode->m_y, fromNode->m_z, toNode->m_x, toNode->m_y, toNode->m_z);

		if (dist > 15.0f)
		{
			fx::scripting::Warningf("natives", "REGISTER_TRACK_JUNCTION: the specified track nodes must overlap each other\n");
			context.SetResult<int>(-1);
			return;
		}

		const std::lock_guard _(g_trackJunctionLock);
		for (const auto& [_, data] : g_trackJunctions)
		{
			if (data.direction == direction && data.onTrack == trackIndex && data.newTrack == newTrackIndex && data.onNode == trackNode && data.newNode == newTrackNode)
			{
				fx::scripting::Warningf("natives", "REGISTER_TRACK_JUNCTION: Cannot register duplicate track junctions\n");
				context.SetResult<int>(-1);
				return;
			}
		}

		int junctionIndex = g_freeTrackJunctions.front();
		g_freeTrackJunctions.pop();

		context.SetResult<int>(junctionIndex);
		g_trackJunctions[junctionIndex] = CTrainJunction(trackIndex, trackNode, newTrackIndex, newTrackNode, direction);
		
	});

	fx::ScriptEngine::RegisterNativeHandler("REMOVE_TRACK_JUNCTION", [](fx::ScriptContext& context)
	{
		size_t junctionIndex = context.GetArgument<size_t>(0);

		const std::lock_guard _(g_trackJunctionLock);

		if (!IsTrackJunctionIdValid(junctionIndex))
		{
			fx::scripting::Warningf("natives", "REMOVE_TRACK_JUNCTION: Invalid junction id (%i) provided.", junctionIndex);
			context.SetResult<bool>(false);
			return;
		}

		g_trackJunctions.erase(junctionIndex);
		g_freeTrackJunctions.push(junctionIndex);

		context.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_TRACK_JUNCTION_ACTIVE", [](fx::ScriptContext& context)
	{
		size_t junctionIndex = context.GetArgument<size_t>(0);
		bool state = context.GetArgument<bool>(1);

		const std::lock_guard _(g_trackJunctionLock);

		if (!IsTrackJunctionIdValid(junctionIndex))
		{
			fx::scripting::Warningf("natives", "SET_TRACK_JUNCTION_ACTIVE: Invalid junction id (%i) provided.", junctionIndex);
			context.SetResult<bool>(false);
			return;
		}

		g_trackJunctions[junctionIndex].isActive = state;
		context.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TRACK_NODE_COORDS", [](fx::ScriptContext& context)
	{
		int8_t trackIndex = context.GetArgument<int8_t>(0);
		uint32_t trackNodeIndex = context.GetArgument<uint32_t>(1);

		rage::CTrainTrack* track = GetAndCheckTrack<0>(context, "GET_TRACK_NODE_COORDS");

		if (!track)
		{
			context.SetResult<bool>(false);
			return;
		}

		rage::CTrackNode* trackNode = GetAndCheckTrackNode(track, trackIndex, trackNodeIndex, "GET_TRACK_NODE_COORDS");

		if (!trackNode)
		{
			context.SetResult<bool>(false);
			return;
		}

		scrVector* nodeCoord = context.GetArgument<scrVector*>(2);

		nodeCoord->x = trackNode->m_x;
		nodeCoord->y = trackNode->m_y;
		nodeCoord->z = trackNode->m_z;

		context.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TRACK_NODE_COUNT", [](fx::ScriptContext& context)
	{
		if (rage::CTrainTrack* track = GetAndCheckTrack<0>(context, "GET_TRACK_NODE_COUNT"))
		{
			context.SetResult<int>(track->m_nodeCount);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CLOSEST_TRACK_NODES", [](fx::ScriptContext& context)
	{
		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);
		float z = context.GetArgument<float>(2);
		float radius = context.GetArgument<float>(3);
		bool includeDisabledTracks = context.GetArgument<bool>(4);

		context.SetResult(fx::SerializeObject(GetTrackNodesInRadius(x, y, z, radius, includeDisabledTracks)));
	});
});

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
		CTrain::kTrainSpeedOffset = *hook::get_pattern<uint32_t>("4C 89 AF ? ? ? ? 44 89 AF ? ? ? ? 4C 89 AF ? ? ? ? 49 8B 0E", 3);
	}

	g_CTrain__Update = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 44 8A B5 ? ? ? ? 48 85 F6")), CTrain__Update);

	
	for (int i = 1; i < g_maxTrackJunctions; i++)
	{
		g_freeTrackJunctions.push(i);
	}

	OnKillNetworkDone.Connect([]()
	{
		const std::lock_guard _(g_trackJunctionLock);
		g_trackJunctions.clear();
		g_freeTrackJunctions = std::queue<int>();
		for (int i = 1; i < g_maxTrackJunctions; i++)
		{
			g_freeTrackJunctions.push(i);
		}
	});
});
