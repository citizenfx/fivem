#pragma once

#include <state/ServerGameState.h>

#include <array>
#include <bitset>
#include <variant>

#include <boost/type_index.hpp>

namespace fx::sync
{
template<int Id1, int Id2, int Id3>
struct NodeIds
{
	inline static std::tuple<int, int, int> GetIds()
	{
		return { Id1, Id2, Id3 };
	}
};

inline bool shouldRead(SyncParseState& state, const std::tuple<int, int, int>& ids)
{
	if ((std::get<0>(ids) & state.syncType) == 0)
	{
		return false;
	}

	// because we hardcode this sync type to 0 (mA0), we can assume it's not used
	if (std::get<2>(ids) != 0)
	{
		return false;
	}

	if ((std::get<1>(ids) & state.syncType) != 0)
	{
		if (!state.buffer.ReadBit())
		{
			return false;
		}
	}

	return true;
}

inline bool shouldWrite(SyncUnparseState& state, const std::tuple<int, int, int>& ids, bool defaultValue = true)
{
	if ((std::get<0>(ids) & state.syncType) == 0)
	{
		return false;
	}

	// because we hardcode this sync type to 0 (mA0), we can assume it's not used
	if (std::get<2>(ids) != 0)
	{
		return false;
	}

	if ((std::get<1>(ids) & state.syncType) != 0)
	{
		state.buffer.WriteBit(defaultValue);

		return defaultValue;
	}

	return true;
}

// from https://stackoverflow.com/a/26902803
template<class F, class...Ts, std::size_t...Is>
void for_each_in_tuple(std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>) {
	using expander = int[];
	(void)expander {
		0, ((void)func(std::get<Is>(tuple)), 0)...
	};
}

template<class F, class...Ts>
void for_each_in_tuple(std::tuple<Ts...> & tuple, F func) {
	for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

template<typename TIds, typename... TChildren>
struct ParentNode : public NodeBase
{
	std::tuple<TChildren...> children;

	virtual bool Parse(SyncParseState& state) override
	{
		if (shouldRead(state, TIds::GetIds()))
		{
			for_each_in_tuple(children, [&](auto& child)
			{
				child.Parse(state);
			});
		}

		return true;
	}

	virtual bool Unparse(SyncUnparseState& state) override
	{
		bool should = false;

		if (shouldWrite(state, TIds::GetIds()))
		{
			for_each_in_tuple(children, [&](auto& child)
			{
				bool thisShould = child.Unparse(state);

				should = should || thisShould;
			});
		}

		return should;
	}

	virtual bool Visit(const SyncTreeVisitor& visitor) override
	{
		visitor(*this);

		for_each_in_tuple(children, [&](auto& child)
		{
			child.Visit(visitor);
		});

		return true;
	}
};

template<typename TIds, typename TNode, typename = void>
struct NodeWrapper : public NodeBase
{
	std::array<uint8_t, 256> data;
	uint32_t length;

	TNode node;

	NodeWrapper()
		: length(0)
	{
		ackedPlayers.set();
	}
	
	virtual bool Parse(SyncParseState& state) override
	{
		auto isWrite = state.buffer.ReadBit();

		if (!isWrite)
		{
			return true;
		}

		auto length = state.buffer.Read<uint32_t>(11);
		auto curBit = state.buffer.GetCurrentBit();

		if (shouldRead(state, TIds::GetIds()))
		{
			// read into data array
			auto endBit = state.buffer.GetCurrentBit();
			auto leftoverLength = length - (endBit - curBit);

			auto oldData = data;

			this->length = leftoverLength;
			state.buffer.ReadBits(data.data(), leftoverLength);

			state.buffer.SetCurrentBit(endBit);

			// parse
			node.Parse(state);

			//if (memcmp(oldData.data(), data.data(), data.size()) != 0)
			{
				//trace("resetting acks on node %s\n", boost::typeindex::type_id<TNode>().pretty_name());
				frameIndex = state.frameIndex;

				ackedPlayers.reset();
			}
		}

		state.buffer.SetCurrentBit(curBit + length);

		return true;
	}

	virtual bool Unparse(SyncUnparseState& state) override
	{
		bool hasData = (length > 0);

		//state.buffer.Write(8, 0x5A);

		if (shouldWrite(state, TIds::GetIds(), (hasData && !ackedPlayers.test(state.client->GetSlotId()))))
		{
			//trace("writing out node %s\n", boost::typeindex::type_id<TNode>().pretty_name());

			state.buffer.WriteBits(data.data(), length);

			return true;
		}

		return false;
	}

	virtual bool Visit(const SyncTreeVisitor& visitor) override
	{
		visitor(*this);

		return true;
	}
};

template<typename TNode>
struct SyncTree : public SyncTreeBase
{
	TNode root;

	virtual void Parse(SyncParseState& state) override
	{
		//trace("parsing root\n");

		if (state.syncType == 2 || state.syncType == 4)
		{
			// mA0 flag
			state.buffer.ReadBit();
		}

		root.Parse(state);
	}

	virtual bool Unparse(SyncUnparseState& state) override
	{
		if (state.syncType == 2 || state.syncType == 4)
		{
			state.buffer.WriteBit(0);
		}

		return root.Unparse(state);
	}

	virtual void Visit(const SyncTreeVisitor& visitor) override
	{
		root.Visit(visitor);
	}
};

struct CVehicleCreationDataNode
{
	bool Parse(SyncParseState& state)
	{
		uint32_t model = state.buffer.Read<uint32_t>(32);
		return true;
	}
};

struct CAutomobileCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CGlobalFlagsDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDynamicEntityGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CEntityScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CEntityScriptInfoDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalAttachDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleAppearanceDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleDamageStatusDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleComponentReservationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleHealthDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleTaskDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CSectorDataNode
{
	bool Parse(SyncParseState& state)
	{
		auto sectorX = state.buffer.Read<int>(10);
		auto sectorY = state.buffer.Read<int>(10);
		auto sectorZ = state.buffer.Read<int>(6);

		state.entity->data["sectorX"] = sectorX;
		state.entity->data["sectorY"] = sectorY;
		state.entity->data["sectorZ"] = sectorZ;

		state.entity->CalculatePosition();

		return true;
	}
};

struct CSectorPositionDataNode
{
	bool Parse(SyncParseState& state)
	{
		auto posX = state.buffer.ReadFloat(12, 54.0f);
		auto posY = state.buffer.ReadFloat(12, 54.0f);
		auto posZ = state.buffer.ReadFloat(12, 69.0f);

		state.entity->data["sectorPosX"] = posX;
		state.entity->data["sectorPosY"] = posY;
		state.entity->data["sectorPosZ"] = posZ;

		state.entity->CalculatePosition();

		return true;
	}
};

struct CPedCreationDataNode
{
	bool Parse(SyncParseState& state)
	{
		auto isRespawnObjectId = state.buffer.ReadBit();
		auto respawnFlaggedForRemoval = state.buffer.ReadBit();

		auto popType = state.buffer.Read<int>(4);
		auto model = state.buffer.Read<int>(32);

		auto randomSeed = state.buffer.Read<int>(16);
		auto inVehicle = state.buffer.ReadBit();
		auto unkVal = state.buffer.Read<int>(32);

		uint16_t vehicleId = 0;
		int vehicleSeat = 0;

		if (inVehicle)
		{
			vehicleId = state.buffer.Read<int>(13);
			vehicleSeat = state.buffer.Read<int>(5);
		}

		auto hasProp = state.buffer.ReadBit();

		if (hasProp)
		{
			auto prop = state.buffer.Read<int>(32);
		}

		auto isStanding = state.buffer.ReadBit();
		auto hasAttDamageToPlayer = state.buffer.ReadBit();

		if (hasAttDamageToPlayer)
		{
			auto attributeDamageToPlayer = state.buffer.Read<int>(5);
		}

		auto maxHealth = state.buffer.Read<int>(13);
		auto unkBool = state.buffer.ReadBit();

		return true;
	}
};

struct CPedGameStateDataNode
{
	bool Parse(SyncParseState& state)
	{
		auto bool1 = state.buffer.ReadBit();
		auto bool2 = state.buffer.ReadBit();
		auto arrestState = state.buffer.Read<int>(1);
		auto deathState = state.buffer.Read<int>(2);

		auto hasWeapon = state.buffer.ReadBit();
		int weapon = 0;

		if (hasWeapon)
		{
			weapon = state.buffer.Read<int>(32);
		}

		auto weaponExists = state.buffer.ReadBit();
		auto weaponVisible = state.buffer.ReadBit();
		auto weaponHasAmmo = state.buffer.ReadBit();
		auto weaponAttachLeft = state.buffer.ReadBit();
		auto weaponUnk = state.buffer.ReadBit();

		auto hasTint = state.buffer.ReadBit();

		if (hasTint)
		{
			auto tintIndex = state.buffer.Read<int>(5);
		}

		auto numWeaponComponents = state.buffer.Read<int>(4);

		for (int i = 0; i < numWeaponComponents; i++)
		{
			auto componentHash = state.buffer.Read<int>(32);
		}

		auto numGadgets = state.buffer.Read<int>(2);

		for (int i = 0; i < numGadgets; i++)
		{
			auto gadgetHash = state.buffer.Read<int>(32);
		}

		auto inVehicle = state.buffer.ReadBit();
		uint16_t vehicleId = 0;
		int vehicleSeat = 0;

		if (inVehicle)
		{
			vehicleId = state.buffer.Read<int>(13);

			auto inSeat = state.buffer.ReadBit();

			if (inSeat)
			{
				vehicleSeat = state.buffer.Read<int>(5);
			}
		}

		// TODO

		return true;
	}
};

struct CEntityOrientationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalVelocityDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleAngVelocityDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleSteeringDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleControlDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleGadgetDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CMigrationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalMigrationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalScriptMigrationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleProximityMigrationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CBikeGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CBoatGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDoorCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDoorMovementDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDoorScriptInfoDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDoorScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CHeliHealthDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CHeliControlDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CObjectCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CObjectGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CObjectScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalHealthDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CObjectSectorPosNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalAngVelocityDataNode { bool Parse(SyncParseState& state) { return true; } };
//struct CPedCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedScriptCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
//struct CPedGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedComponentReservationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedAttachDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedHealthDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedMovementGroupDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedAIDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedAppearanceDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedOrientationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedMovementDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedTaskTreeDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedTaskSpecificDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedSectorPosMapNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedSectorPosNavMeshNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedInventoryDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedTaskSequenceDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPickupCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPickupScriptGameStateNode { bool Parse(SyncParseState& state) { return true; } };
struct CPickupSectorPosNode { bool Parse(SyncParseState& state) { return true; } };
struct CPickupPlacementCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPickupPlacementStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlaneGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlaneControlDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CSubmarineGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CSubmarineControlDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerAppearanceDataNode { bool Parse(SyncParseState& state) { trace("PlayerAppearanceDataNode!\n"); return true; } };
struct CPlayerPedGroupDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerAmbientModelStreamingNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerGamerDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerExtendedGameStateNode { bool Parse(SyncParseState& state) { return true; } };

struct CPlayerSectorPosNode
{
	bool Parse(SyncParseState& state)
	{
		// extra data
		if (state.buffer.ReadBit())
		{
			// unknown fields
			state.buffer.ReadBit();
			state.buffer.ReadBit();

			// is standing on?
			bool isStandingOn = state.buffer.ReadBit();
			if (isStandingOn)
			{
				state.buffer.Read<int>(13); // Standing On
				state.buffer.ReadFloat(12, 16.0f); // Standing On Local Offset X
				state.buffer.ReadFloat(12, 16.0f); // Standing On Local Offset Y
				state.buffer.ReadFloat(9, 4.0f); // Standing On Local Offset Z
			}

			state.entity->data["isStandingOn"] = isStandingOn;
		}

		auto posX = state.buffer.ReadFloat(12, 54.0f);
		auto posY = state.buffer.ReadFloat(12, 54.0f);
		auto posZ = state.buffer.ReadFloat(12, 69.0f);

		state.entity->data["sectorPosX"] = posX;
		state.entity->data["sectorPosY"] = posY;
		state.entity->data["sectorPosZ"] = posZ;

		state.entity->CalculatePosition();

		return true;
	}
};

struct CPlayerCameraDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerWantedAndLOSDataNode { bool Parse(SyncParseState& state) { return true; } };

using CAutomobileSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode>, 
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 127, 0>, 
			ParentNode<
				NodeIds<127, 127, 0>, 
				ParentNode<
					NodeIds<127, 127, 0>, 
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode>
				>, 
				ParentNode<
					NodeIds<127, 127, 1>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>
				>
			>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode>
		>, 
		ParentNode<
			NodeIds<127, 86, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode>, 
			ParentNode<
				NodeIds<127, 86, 0>, 
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode>, 
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode>
			>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode>
		>
	>
>;
using CBikeSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 127, 0>, 
			ParentNode<
				NodeIds<127, 127, 0>, 
				ParentNode<
					NodeIds<127, 127, 0>, 
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CBikeGameStateDataNode>
				>, 
				ParentNode<
					NodeIds<127, 127, 1>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>
				>
			>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode>
		>, 
		ParentNode<
			NodeIds<127, 86, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode>, 
			ParentNode<
				NodeIds<127, 86, 0>, 
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode>, 
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode>
			>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode>
		>
	>
>;
using CBoatSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 87, 0>, 
			ParentNode<
				NodeIds<127, 87, 0>, 
				ParentNode<
					NodeIds<127, 87, 0>, 
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode>, 
					NodeWrapper<NodeIds<87, 87, 0>, CBoatGameStateDataNode>
				>, 
				ParentNode<
					NodeIds<127, 127, 1>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>
				>
			>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode>
		>, 
		ParentNode<
			NodeIds<127, 86, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode>, 
			ParentNode<
				NodeIds<127, 86, 0>, 
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode>, 
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode>
			>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode>
		>
	>
>;
using CDoorSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CDoorCreationDataNode>
		>, 
		NodeWrapper<NodeIds<86, 86, 0>, CDoorMovementDataNode>, 
		ParentNode<
			NodeIds<127, 127, 0>, 
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
			NodeWrapper<NodeIds<127, 127, 1>, CDoorScriptInfoDataNode>, 
			NodeWrapper<NodeIds<127, 127, 1>, CDoorScriptGameStateDataNode>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>
		>
	>
>;
using CHeliSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode>, 
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 87, 0>, 
			ParentNode<
				NodeIds<127, 127, 0>, 
				ParentNode<
					NodeIds<127, 127, 0>, 
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode>
				>, 
				ParentNode<
					NodeIds<127, 127, 1>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>
				>
			>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CHeliHealthDataNode>
		>, 
		ParentNode<
			NodeIds<127, 86, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode>, 
			ParentNode<
				NodeIds<127, 86, 0>, 
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode>, 
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode>, 
				NodeWrapper<NodeIds<86, 86, 0>, CHeliControlDataNode>
			>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode>
		>
	>
>;
using CObjectSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CObjectCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 127, 0>, 
			ParentNode<
				NodeIds<127, 127, 0>, 
				ParentNode<
					NodeIds<127, 127, 0>, 
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CObjectGameStateDataNode>
				>, 
				ParentNode<
					NodeIds<127, 127, 1>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CObjectScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>
				>
			>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalHealthDataNode>
		>, 
		ParentNode<
			NodeIds<87, 87, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CObjectSectorPosNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalAngVelocityDataNode>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>
		>
	>
>;
using CPedSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CPedCreationDataNode>, 
			NodeWrapper<NodeIds<1, 0, 1>, CPedScriptCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 87, 0>, 
			ParentNode<
				NodeIds<127, 127, 0>, 
				ParentNode<
					NodeIds<127, 127, 0>, 
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPedGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPedComponentReservationDataNode>
				>, 
				ParentNode<
					NodeIds<127, 127, 1>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CPedScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>
				>
			>, 
			NodeWrapper<NodeIds<127, 127, 1>, CPedAttachDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPedHealthDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementGroupDataNode>, 
			NodeWrapper<NodeIds<127, 127, 1>, CPedAIDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPedAppearanceDataNode>
		>, 
		ParentNode<
			NodeIds<127, 87, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode>, 
			ParentNode<
				NodeIds<127, 87, 0>, 
				NodeWrapper<NodeIds<127, 127, 0>, CPedTaskTreeDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>
			>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosMapNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosNavMeshNode>
		>, 
		ParentNode<
			NodeIds<5, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>, 
			NodeWrapper<NodeIds<5, 0, 0>, CPedInventoryDataNode>, 
			NodeWrapper<NodeIds<4, 4, 1>, CPedTaskSequenceDataNode>
		>
	>
>;
using CPickupSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CPickupCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 127, 0>, 
			ParentNode<
				NodeIds<127, 127, 0>, 
				NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
				NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>
			>, 
			ParentNode<
				NodeIds<127, 127, 1>, 
				NodeWrapper<NodeIds<127, 127, 1>, CPickupScriptGameStateNode>, 
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalGameStateDataNode>, 
				NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>, 
				NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>, 
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalHealthDataNode>
			>, 
			NodeWrapper<NodeIds<127, 127, 1>, CPhysicalAttachDataNode>
		>, 
		ParentNode<
			NodeIds<87, 87, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPickupSectorPosNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalAngVelocityDataNode>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>
		>
	>
>;
using CPickupPlacementSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		NodeWrapper<NodeIds<1, 0, 0>, CPickupPlacementCreationDataNode>, 
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
		NodeWrapper<NodeIds<127, 127, 0>, CPickupPlacementStateDataNode>
	>
>;
using CPlaneSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 127, 0>, 
			ParentNode<
				NodeIds<127, 127, 0>, 
				ParentNode<
					NodeIds<127, 127, 0>, 
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode>
				>, 
				ParentNode<
					NodeIds<127, 127, 1>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>
				>
			>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPlaneGameStateDataNode>
		>, 
		ParentNode<
			NodeIds<127, 86, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode>, 
			ParentNode<
				NodeIds<127, 86, 0>, 
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode>, 
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode>, 
				NodeWrapper<NodeIds<86, 86, 0>, CPlaneControlDataNode>
			>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode>
		>
	>
>;
using CSubmarineSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 87, 0>, 
			ParentNode<
				NodeIds<127, 87, 0>, 
				ParentNode<
					NodeIds<127, 87, 0>, 
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode>, 
					NodeWrapper<NodeIds<87, 87, 0>, CSubmarineGameStateDataNode>
				>, 
				ParentNode<
					NodeIds<127, 127, 1>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>
				>
			>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode>
		>, 
		ParentNode<
			NodeIds<127, 86, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode>, 
			ParentNode<
				NodeIds<127, 86, 0>, 
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode>, 
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode>, 
				NodeWrapper<NodeIds<86, 86, 0>, CSubmarineControlDataNode>
			>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode>
		>
	>
>;
using CPlayerSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CPlayerCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 86, 0>, 
			ParentNode<
				NodeIds<127, 87, 0>, 
				ParentNode<
					NodeIds<127, 127, 0>, 
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPedGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPedComponentReservationDataNode>
				>, 
				ParentNode<
					NodeIds<127, 87, 0>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<87, 87, 0>, CPlayerGameStateDataNode>
				>
			>, 
			NodeWrapper<NodeIds<127, 127, 1>, CPedAttachDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPedHealthDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementGroupDataNode>, 
			NodeWrapper<NodeIds<127, 127, 1>, CPedAIDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerAppearanceDataNode>, 
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerPedGroupDataNode>, 
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerAmbientModelStreamingNode>, 
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerGamerDataNode>, 
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerExtendedGameStateNode>
		>, 
		ParentNode<
			NodeIds<127, 86, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode>, 
			ParentNode<
				NodeIds<127, 87, 0>, 
				NodeWrapper<NodeIds<127, 127, 0>, CPedTaskTreeDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode>
			>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerSectorPosNode>, 
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerCameraDataNode>, 
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerWantedAndLOSDataNode>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>
		>
	>
>;
using CAutomobileSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>, 
		ParentNode<
			NodeIds<1, 0, 0>, 
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode>, 
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode>
		>, 
		ParentNode<
			NodeIds<127, 127, 0>, 
			ParentNode<
				NodeIds<127, 127, 0>, 
				ParentNode<
					NodeIds<127, 127, 0>, 
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode>
				>, 
				ParentNode<
					NodeIds<127, 127, 1>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode>, 
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>
				>
			>, 
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>, 
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode>
		>, 
		ParentNode<
			NodeIds<127, 86, 0>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>, 
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode>, 
			ParentNode<
				NodeIds<127, 86, 0>, 
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode>, 
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode>, 
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode>
			>
		>, 
		ParentNode<
			NodeIds<4, 0, 0>, 
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>, 
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode>
		>
	>
>;
}
