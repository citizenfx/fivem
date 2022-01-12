#pragma once

#include "StateGuard.h"

#include <state/ServerGameState.h>

#include <array>
#include <bitset>
#include <variant>

#include <boost/type_index.hpp>

#include "SyncTrees_Header.h"

namespace fx::sync
{
struct CVehicleCreationDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CAutomobileCreationDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CGlobalFlagsDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CDynamicEntityGameStateDataNode : GenericSerializeDataNode<CDynamicEntityGameStateDataNode>
{
	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
		return true;
	}
};

struct CPhysicalGameStateDataNode : GenericSerializeDataNode<CPhysicalGameStateDataNode>
{
	bool isVisible;
	bool flag2;
	bool flag3;
	bool flag4;

	int val1;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
		s.Serialize(isVisible);
		s.Serialize(flag2);
		s.Serialize(flag3);
		s.Serialize(flag4);

		if (flag4)
		{
			s.Serialize(3, val1);
		}
		else
		{
			val1 = 0;
		}

		return true;
	}
};

struct CVehicleGameStateDataNode
{
	CVehicleGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		return true;
	}
};

struct CEntityScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CEntityScriptInfoDataNode
{
	uint32_t m_scriptHash;
	uint32_t m_timestamp;

	bool Parse(SyncParseState& state)
	{
		auto hasScript = state.buffer.ReadBit();

		if (hasScript) // Has script info
		{
			// deserialize CGameScriptObjInfo

			// -> CGameScriptId

			// ---> rage::scriptId
			m_scriptHash = state.buffer.Read<uint32_t>(32);
			// ---> end

			m_timestamp = state.buffer.Read<uint32_t>(32);

			if (state.buffer.ReadBit())
			{
				auto positionHash = state.buffer.Read<uint32_t>(32);
			}

			if (state.buffer.ReadBit())
			{
				auto instanceId = state.buffer.Read<uint32_t>(7);
			}

			// -> end

			auto scriptObjectId = state.buffer.Read<uint32_t>(32);

			auto hostTokenLength = state.buffer.ReadBit() ? 16 : 3;
			auto hostToken = state.buffer.Read<uint32_t>(hostTokenLength);

			// end
		}
		else
		{
			m_scriptHash = 0;
		}

		return true;
	}

	bool Unparse(sync::SyncUnparseState& state)
	{
		rl::MessageBuffer& buffer = state.buffer;

		if (m_scriptHash)
		{
			buffer.WriteBit(true);

			buffer.Write<uint32_t>(32, m_scriptHash);
			buffer.Write<uint32_t>(32, m_timestamp);

			buffer.WriteBit(false);
			buffer.WriteBit(false);

			buffer.Write<uint32_t>(32, 12);

			buffer.WriteBit(false);
			buffer.Write<uint32_t>(3, 0);
		}
		else
		{
			buffer.WriteBit(false);
		}

		return true;
	}
};

struct CPhysicalAttachDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CVehicleAppearanceDataNode
{
	CVehicleAppearanceNodeData data;

	bool Parse(SyncParseState& state)
	{
		return true;
	}
};

struct CVehicleDamageStatusDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleComponentReservationDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CVehicleHealthDataNode
{
	CVehicleHealthNodeData data;

	bool Parse(SyncParseState& state)
	{
		return true;
	}
};

struct CVehicleTaskDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CSectorDataNode
{
	int m_sectorX;
	int m_sectorY;
	int m_sectorZ;

	bool Parse(SyncParseState& state)
	{
		auto sectorX = state.buffer.Read<int>(10);
		auto sectorY = state.buffer.Read<int>(10);
		auto sectorZ = state.buffer.Read<int>(6);

		m_sectorX = sectorX;
		m_sectorY = sectorY;
		m_sectorZ = sectorZ;

		state.entity->syncTree->CalculatePosition();

		return true;
	}

	bool Unparse(sync::SyncUnparseState& state)
	{
		rl::MessageBuffer& buffer = state.buffer;

		buffer.Write<int>(10, m_sectorX);
		buffer.Write<int>(10, m_sectorY);
		buffer.Write<int>(6, m_sectorZ);

		return true;
	}
};

struct CSectorPositionDataNode
{
	float m_posX;
	float m_posY;
	float m_posZ;

	bool Parse(SyncParseState& state)
	{
		auto posX = state.buffer.ReadFloat(12, 54.0f);
		auto posY = state.buffer.ReadFloat(12, 54.0f);
		auto posZ = state.buffer.ReadFloat(12, 69.0f);

		m_posX = posX;
		m_posY = posY;
		m_posZ = posZ;

		state.entity->syncTree->CalculatePosition();

		return true;
	}

	bool Unparse(sync::SyncUnparseState& state)
	{
		rl::MessageBuffer& buffer = state.buffer;
		buffer.WriteFloat(12, 54.0f, m_posX);
		buffer.WriteFloat(12, 54.0f, m_posY);
		buffer.WriteFloat(12, 69.0f, m_posZ);

		return true;
	}
};

struct CPedCreationDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CPedGameStateDataNode
{
	CPedGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		return true;
	}
};

struct CEntityOrientationDataNode : GenericSerializeDataNode<CEntityOrientationDataNode>
{
	CEntityOrientationNodeData data;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
#if 0
		auto rotX = state.buffer.ReadSigned<int>(9) * 0.015625f;
		auto rotY = state.buffer.ReadSigned<int>(9) * 0.015625f;
		auto rotZ = state.buffer.ReadSigned<int>(9) * 0.015625f;

		data.rotX = rotX;
		data.rotY = rotY;
		data.rotZ = rotZ;
#else
		s.Serialize(2, data.quat.largest);
		s.Serialize(11, data.quat.integer_a);
		s.Serialize(11, data.quat.integer_b);
		s.Serialize(11, data.quat.integer_c);
#endif

		return true;
	}
};

struct CPhysicalVelocityDataNode
{
	CPhysicalVelocityNodeData data;

	bool Parse(SyncParseState& state)
	{
		auto velX = state.buffer.ReadSigned<int>(12) * 0.0625f;
		auto velY = state.buffer.ReadSigned<int>(12) * 0.0625f;
		auto velZ = state.buffer.ReadSigned<int>(12) * 0.0625f;

		data.velX = velX;
		data.velY = velY;
		data.velZ = velZ;

		return true;
	}
};

struct CVehicleAngVelocityDataNode
{
	CVehicleAngVelocityNodeData data;

	bool Parse(SyncParseState& state)
	{
		auto hasNoVelocity = state.buffer.ReadBit();

		if (!hasNoVelocity)
		{
			auto velX = state.buffer.ReadSigned<int>(10) * 0.03125f;
			auto velY = state.buffer.ReadSigned<int>(10) * 0.03125f;
			auto velZ = state.buffer.ReadSigned<int>(10) * 0.03125f;

			data.angVelX = velX;
			data.angVelY = velY;
			data.angVelZ = velZ;
		}
		else
		{
			data.angVelX = 0.0f;
			data.angVelY = 0.0f;
			data.angVelZ = 0.0f;

			state.buffer.ReadBit();
		}

		return true;
	}
};

struct CDoorCreationDataNode
{
	float m_posX;
	float m_posY;
	float m_posZ;

	bool Parse(SyncParseState& state)
	{
		auto positionX = state.buffer.ReadSignedFloat(31, 27648.0f);
		auto positionY = state.buffer.ReadSignedFloat(31, 27648.0f);
		auto positionZ = state.buffer.ReadFloat(31, 4416.0f) - 1700.0f;

		m_posX = positionX;
		m_posY = positionY;
		m_posZ = positionZ;

		auto modelHash = state.buffer.Read<uint32_t>(32);

		// ...

		return true;
	}
};

struct CVehicleSteeringDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleControlDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleGadgetDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CMigrationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalMigrationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalScriptMigrationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleProximityMigrationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CBikeGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CBoatGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDoorMovementDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDoorScriptInfoDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDoorScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CHeliHealthDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CHeliControlDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CObjectCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CObjectGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CObjectScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalHealthDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CObjectSectorPosNode : GenericSerializeDataNode<CObjectSectorPosNode>
{
	bool highRes;
	float m_posX;
	float m_posY;
	float m_posZ;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
		s.Serialize(highRes);

		int bits = (highRes) ? 20 : 12;

		s.Serialize(bits, 54.0f, m_posX);
		s.Serialize(bits, 54.0f, m_posY);
		s.Serialize(bits, 69.0f, m_posZ);

		if constexpr (Serializer::isReader)
		{
			s.state->entity->syncTree->CalculatePosition();
		}

		return true;
	}
};

struct CPhysicalAngVelocityDataNode
{
	CVehicleAngVelocityNodeData data;

	bool Parse(SyncParseState& state)
	{
		auto velX = state.buffer.ReadSigned<int>(10) * 0.03125f;
		auto velY = state.buffer.ReadSigned<int>(10) * 0.03125f;
		auto velZ = state.buffer.ReadSigned<int>(10) * 0.03125f;

		data.angVelX = velX;
		data.angVelY = velY;
		data.angVelZ = velZ;

		return true;
	}
};
//struct CPedCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedScriptCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
//struct CPedGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedComponentReservationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedAttachDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CPedHealthDataNode
{
	CPedHealthNodeData data;

	bool Parse(SyncParseState& state)
	{
		return true;
	}
};

struct CPedMovementGroupDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedAIDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedAppearanceDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CPedOrientationDataNode : GenericSerializeDataNode<CPedOrientationDataNode>
{
	CPedOrientationNodeData data;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
		s.SerializeSigned(8, 6.28318548f, data.currentHeading);
		s.SerializeSigned(8, 6.28318548f, data.desiredHeading);

		return true;
	}
};

struct CPedMovementDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedTaskTreeDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedTaskSpecificDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CPedSectorPosMapNode
{
	float m_posX;
	float m_posY;
	float m_posZ;

	bool Parse(SyncParseState& state)
	{
		auto posX = state.buffer.ReadFloat(12, 54.0f);
		auto posY = state.buffer.ReadFloat(12, 54.0f);
		auto posZ = state.buffer.ReadFloat(12, 69.0f);

		m_posX = posX;
		m_posY = posY;
		m_posZ = posZ;

		state.entity->syncTree->CalculatePosition();

		// more data follows

		return true;
	}
};

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
struct CTrainGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CPlayerAppearanceDataNode
{
	uint32_t model;

	bool Parse(SyncParseState& state)
	{
		model = state.buffer.Read<uint32_t>(32);

		return true;
	}
};

struct CPlayerPedGroupDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerAmbientModelStreamingNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerGamerDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerExtendedGameStateNode { bool Parse(SyncParseState& state) { return true; } };

struct CPlayerSectorPosNode
{
	float m_posX;
	float m_posY;
	float m_posZ;

	uint16_t m_standingOnHandle;
	float m_standingOffsetX;
	float m_standingOffsetY;
	float m_standingOffsetZ;

	bool isStandingOn;

	bool Parse(SyncParseState& state)
	{
		auto posX = state.buffer.ReadFloat(12, 54.0f);
		auto posY = state.buffer.ReadFloat(12, 54.0f);
		auto posZ = state.buffer.ReadFloat(12, 69.0f);

		m_posX = posX;
		m_posY = posY;
		m_posZ = posZ;
	
		// ...

		state.entity->syncTree->CalculatePosition();

		return true;
	}
};

struct CPlayerCameraDataNode
{
	CPlayerCameraNodeData data;

	bool Parse(SyncParseState& state)
	{
		bool freeCamOverride = state.buffer.ReadBit();

		if (freeCamOverride)
		{
			bool unk = state.buffer.ReadBit();

			float freeCamPosX = state.buffer.ReadSignedFloat(19, 27648.0f);
			float freeCamPosY = state.buffer.ReadSignedFloat(19, 27648.0f);
			float freeCamPosZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;

			// 2pi
			float cameraX = state.buffer.ReadSignedFloat(10, 6.2831855f);
			float cameraZ = state.buffer.ReadSignedFloat(10, 6.2831855f);

			data.camMode = 1;
			data.freeCamPosX = freeCamPosX;
			data.freeCamPosY = freeCamPosY;
			data.freeCamPosZ = freeCamPosZ;

			data.cameraX = cameraX;
			data.cameraZ = cameraZ;
		}
		else
		{
			bool hasPositionOffset = state.buffer.ReadBit();
			state.buffer.ReadBit();

			if (hasPositionOffset)
			{
				float camPosX = state.buffer.ReadSignedFloat(19, 16000.0f);
				float camPosY = state.buffer.ReadSignedFloat(19, 16000.0f);
				float camPosZ = state.buffer.ReadSignedFloat(19, 16000.0f);

				data.camMode = 2;

				data.camOffX = camPosX;
				data.camOffY = camPosY;
				data.camOffZ = camPosZ;
			}
			else
			{
				data.camMode = 0;
			}

			float cameraX = state.buffer.ReadSignedFloat(10, 6.2831855f);
			float cameraZ = state.buffer.ReadSignedFloat(10, 6.2831855f);

			data.cameraX = cameraX;
			data.cameraZ = cameraZ;

			// TODO
		}

		// TODO

		return true;
	}
};

struct CWorldProjectileDataNode
{
	int m_sectorX;
	int m_sectorY;
	int m_sectorZ;

	float m_posX;
	float m_posY;
	float m_posZ;

	bool Parse(SyncParseState& state)
	{
		bool isAttached = state.buffer.ReadBit();

		if (!isAttached)
		{
			m_sectorX = state.buffer.Read<int>(10);
			m_sectorY = state.buffer.Read<int>(10);
			m_sectorZ = state.buffer.Read<int>(6);

			m_posX = state.buffer.ReadFloat(20, 54.0f);
			m_posY = state.buffer.ReadFloat(20, 54.0f);
			m_posZ = state.buffer.ReadFloat(20, 69.0f);
		}
		else
		{
			bool unk = state.buffer.ReadBit();

			m_sectorX = 512;
			m_sectorY = 512;
			m_sectorZ = 0;

			m_posX = state.buffer.ReadFloat(16, 17.0f);
			m_posY = state.buffer.ReadFloat(16, 17.0f);
			m_posZ = state.buffer.ReadFloat(10, 8.0f);
		}

		// TODO

		return true;
	}
};

struct CHerdPositionNode
{
	float m_posX;
	float m_posY;
	float m_posZ;

	bool Parse(SyncParseState& state)
	{
		auto unk = state.buffer.Read<int>(19);
		bool unk2 = state.buffer.ReadBit();

		if (unk2)
		{
			auto unk3 = state.buffer.Read<uint8_t>(3);

			m_posX = state.buffer.ReadFloat(6, 5.0f);
			m_posY = state.buffer.ReadFloat(10, 300.0f);
			m_posZ = state.buffer.ReadFloat(6, 20.0f);
		}
		else
		{
			m_posX = 0.0f;
			m_posY = 0.0f;
			m_posZ = 0.0f;
		}

		return true;
	}
};

// REDM1S: unverified name
struct CObjectSectorDataNode
{
	int m_sectorX;
	int m_sectorY;
	int m_sectorZ;

	bool Parse(SyncParseState& state)
	{
		auto sectorX = state.buffer.Read<int>(10);
		auto sectorY = state.buffer.Read<int>(10);
		auto sectorZ = state.buffer.Read<int>(6);

		m_sectorX = sectorX;
		m_sectorY = sectorY;
		m_sectorZ = sectorZ;

		state.entity->syncTree->CalculatePosition();

		return true;
	}

	bool Unparse(sync::SyncUnparseState& state)
	{
		rl::MessageBuffer& buffer = state.buffer;

		buffer.Write<int>(10, m_sectorX);
		buffer.Write<int>(10, m_sectorY);
		buffer.Write<int>(6, m_sectorZ);

		return true;
	}
};

struct CPropSetCreationDataNode
{
	uint32_t m_modelHash;
	bool m_unk1;
	uint16_t m_unk2;

	bool m_rootPosition;
	
	float m_posX;
	float m_posY;
	float m_posZ;

	uint8_t m_variationIndex;
	float m_heading;
	float m_unk3;
	uint8_t m_unk4;
	bool m_unk5;

	bool m_hasParent;
	uint16_t m_parentId;
	uint32_t m_parentModelHash;
	bool m_unk6;
	uint8_t m_unk7;

	bool Parse(SyncParseState& state)
	{
		m_modelHash = state.buffer.Read<uint32_t>(32);
		m_unk1 = state.buffer.ReadBit();
		m_unk2 = (m_unk1) ? state.buffer.Read<uint16_t>(16) : 0;
		m_rootPosition = state.buffer.ReadBit();

		if (!m_rootPosition)
		{
			m_posX = state.buffer.ReadSignedFloat(31, 27648.0f);
			m_posY = state.buffer.ReadSignedFloat(31, 27648.0f);
			m_posZ = state.buffer.ReadFloat(31, 4416.0f) - 1700.0f;
		}
		else
		{
			m_posX = 0.0f;
			m_posY = 0.0f;
			m_posZ = 0.0f;
		}

		m_variationIndex = state.buffer.Read<uint8_t>(4);
		m_heading = state.buffer.ReadSignedFloat(31, 6.28318548f);
		m_unk3 = state.buffer.ReadSignedFloat(31, 1200.0f);
		m_unk4 = state.buffer.Read<uint8_t>(3);
		m_unk5 = state.buffer.ReadBit();

		m_hasParent = state.buffer.ReadBit();

		if (m_hasParent)
		{
			m_parentId = state.buffer.Read<uint16_t>(13);
			m_parentModelHash = state.buffer.Read<uint32_t>(32);
			m_unk6 = state.buffer.ReadBit();
			m_unk7 = (m_unk6) ? state.buffer.Read<uint8_t>(4) : -1;
		}
		else
		{
			m_parentId = 0;
			m_parentModelHash = 0;
			m_unk6 = 0;
			m_unk7 = -1;
		}

		// ...

		return true;
	}
};

struct CDraftVehCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CStatsTrackerGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CWorldStateBaseDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CIncidentCreateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CGuardzoneCreateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedGroupCreateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CAnimalCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CProjectileCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedStandingOnObjectDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CProjectileAttachNode { bool Parse(SyncParseState& state) { return true; } };
struct CHerdMemberDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CHerdGameDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CAnimSceneCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CAnimSceneFrequentDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CAnimSceneInfrequentDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CGroupScenarioFrequentDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CGroupScenarioEntitiesDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CGroupScenarioCreationDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerWeaponInventoryDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPropSetGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPropSetUncommonGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CIncidentStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CIncidentCrimeSceneDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CIncidentPointOfInterestFinderDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CIncidentDispatchDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CIncidentOrderDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CGuardZoneStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CGuardZoneGuardDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CGuardZonePointOfInterestFinderDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CCombatDirectorCreateUpdateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedWeaponDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedVehicleDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerCharacterCreatorDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerAmbientModelStreamingDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerVoiceDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerHealthDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerSpawnSearchDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerAudioScriptBankDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerGoalsDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPlayerCameraUncommonDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CObjectAITaskDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDraftVehControlDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDraftVehHorseHealthDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDraftVehHorseGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDraftVehGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CTrainControlDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CTrainGameStateUncommonDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CVehicleCommonDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDoorDamageDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedSectorPosNavMeshDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedScriptGameStateUncommonDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedFacialAppearanceDataNode { bool Parse(SyncParseState& state) { return true; } };

// REDM1S: unknown rdr3 data nodes (addresses are 1311.20)
struct DataNode_1435984c0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143598fb0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143598e20 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143598b00 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143594ab8 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359b8a8 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435992d0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359e920 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359e790 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143599dc0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435995f0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143599780 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143599910 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143599aa0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143599c30 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143599f50 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359a8b0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359aa40 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143598c90 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359eab0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359ec40 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359a590 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359abd0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359ad88 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359a270 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143594478 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143594dd8 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359a400 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359b588 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359ba38 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359bbc8 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359b0d8 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435a0a20 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359cd00 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359ce90 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359d020 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359db10 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359dfc0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359d660 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143595bf0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435929e0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143592b70 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143592e90 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_143592d00 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359bef0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359c080 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359c210 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_14359c3a0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435a1e78 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435a2010 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435a21a0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435a2330 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435a24c0 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435a2658 { bool Parse(SyncParseState& state) { return true; } };
struct DataNode_1435981a0 { bool Parse(SyncParseState& state) { return true; } };

template<typename TNode>
struct SyncTree : public SyncTreeBase
{
	TNode root;
	std::mutex mutex;

	template<typename TData>
	inline static constexpr size_t GetOffsetOf()
	{
		auto doff = TNode::template GetOffsetOf<TData>();

		return (doff) ? offsetof(SyncTree, root) + doff : 0;
	}

	template<typename TData>
	inline std::tuple<bool, TData*> GetData()
	{
		constexpr auto offset = GetOffsetOf<TData>();

		if constexpr (offset != 0)
		{
			return { true, (TData*)((uintptr_t)this + offset) };
		}

		return { false, nullptr };
	}

	template<typename TData>
	inline static constexpr size_t GetOffsetOfNode()
	{
		auto doff = TNode::template GetOffsetOfNode<TData>();

		return (doff) ? offsetof(SyncTree, root) + doff : 0;
	}

	template<typename TData>
	inline NodeWrapper<NodeIds<0, 0, 0>, TData>* GetNode()
	{
		constexpr auto offset = GetOffsetOfNode<TData>();

		if constexpr (offset != 0)
		{
			return (NodeWrapper<NodeIds<0, 0, 0>, TData>*)((uintptr_t)this + offset - 8);
		}

		return nullptr;
	}

	virtual void GetPosition(float* posOut) override
	{
		auto [hasSdn, secDataNode] = GetData<CSectorDataNode>();
		auto [hasSpdn, secPosDataNode] = GetData<CSectorPositionDataNode>();
		auto [hasWpdn, projectileDataNode] = GetData<CWorldProjectileDataNode>();
		auto [hasPspdn, playerSecPosDataNode] = GetData<CPlayerSectorPosNode>();
		auto [hasOsdn, objectSecDataNode] = GetData<CObjectSectorDataNode>();
		auto [hasOspdn, objectSecPosDataNode] = GetData<CObjectSectorPosNode>();
		auto [hasPspmdn, pedSecPosMapDataNode] = GetData<CPedSectorPosMapNode>();
		auto [hasDoor, doorCreationDataNode] = GetData<CDoorCreationDataNode>();
		auto [hasPropSet, propSetCreationDataNode] = GetData<CPropSetCreationDataNode>();
		auto [hasHpn, herdPosNode] = GetData<CHerdPositionNode>();

		auto sectorX =
			(hasSdn) ? secDataNode->m_sectorX :
				(hasOsdn) ? objectSecDataNode->m_sectorX :
					(hasWpdn) ? projectileDataNode->m_sectorX :
						512;

		auto sectorY =
			(hasSdn) ? secDataNode->m_sectorY :
				(hasOsdn) ? objectSecDataNode->m_sectorY :
					(hasWpdn) ? projectileDataNode->m_sectorY :
						512;

		auto sectorZ =
			(hasSdn) ? secDataNode->m_sectorZ :
				(hasOsdn) ? objectSecDataNode->m_sectorZ :
					(hasWpdn) ? projectileDataNode->m_sectorZ :
						0;

		auto sectorPosX =
			(hasSpdn) ? secPosDataNode->m_posX :
				(hasPspdn) ? playerSecPosDataNode->m_posX :
					(hasOspdn) ? objectSecPosDataNode->m_posX :
						(hasPspmdn) ? pedSecPosMapDataNode->m_posX :
							(hasWpdn) ? projectileDataNode->m_posX :
								(hasHpn) ? herdPosNode->m_posX :
									0.0f;

		auto sectorPosY =
			(hasSpdn) ? secPosDataNode->m_posY :
				(hasPspdn) ? playerSecPosDataNode->m_posY :
					(hasOspdn) ? objectSecPosDataNode->m_posY :
						(hasPspmdn) ? pedSecPosMapDataNode->m_posY :
							(hasWpdn) ? projectileDataNode->m_posY :
								(hasHpn) ? herdPosNode->m_posY :
									0.0f;

		auto sectorPosZ =
			(hasSpdn) ? secPosDataNode->m_posZ :
				(hasPspdn) ? playerSecPosDataNode->m_posZ :
					(hasOspdn) ? objectSecPosDataNode->m_posZ :
						(hasPspmdn) ? pedSecPosMapDataNode->m_posZ :
							(hasWpdn) ? projectileDataNode->m_posZ :
								(hasHpn) ? herdPosNode->m_posZ :
									0.0f;

		posOut[0] = ((sectorX - 512.0f) * 54.0f) + sectorPosX;
		posOut[1] = ((sectorY - 512.0f) * 54.0f) + sectorPosY;
		posOut[2] = ((sectorZ * 69.0f) + sectorPosZ) - 1700.0f;

		if (hasDoor)
		{
			posOut[0] = doorCreationDataNode->m_posX;
			posOut[1] = doorCreationDataNode->m_posY;
			posOut[2] = doorCreationDataNode->m_posZ;
		}

		if (hasPropSet)
		{
			// if prop set attached, get parent's position instead
			if (propSetCreationDataNode->m_hasParent && g_serverGameState)
			{
				auto entity = g_serverGameState->GetEntity(0, propSetCreationDataNode->m_parentId);

				if (entity && entity->type != fx::sync::NetObjEntityType::PropSet)
				{
					entity->syncTree->GetPosition(posOut);

					// apply attachment offset
					posOut[0] += propSetCreationDataNode->m_posX;
					posOut[1] += propSetCreationDataNode->m_posY;
					posOut[2] += propSetCreationDataNode->m_posZ;

				}
			}
			else
			{
				posOut[0] = propSetCreationDataNode->m_posX;
				posOut[1] = propSetCreationDataNode->m_posY;
				posOut[2] = propSetCreationDataNode->m_posZ;
			}
		}
	}

	virtual CDoorMovementDataNodeData* GetDoorMovement() override
	{
		return nullptr;
	}

	virtual CDoorScriptInfoDataNodeData* GetDoorScriptInfo() override
	{
		return nullptr;
	}

	virtual CDoorScriptGameStateDataNodeData* GetDoorScriptGameState() override
	{
		return nullptr;
	}

	virtual CHeliControlDataNodeData* GetHeliControl() override
	{
		return nullptr;
	}

	virtual CPlayerCameraNodeData* GetPlayerCamera() override
	{
		auto [hasCdn, cameraNode] = GetData<CPlayerCameraDataNode>();

		return (hasCdn) ? &cameraNode->data : nullptr;
	}

	virtual CPlayerWantedAndLOSNodeData* GetPlayerWantedAndLOS() override
	{
		return nullptr;
	}

	virtual CPedGameStateNodeData* GetPedGameState() override
	{
		auto [hasPdn, pedNode] = GetData<CPedGameStateDataNode>();

		return (hasPdn) ? &pedNode->data : nullptr;
	}

	virtual uint64_t GetPedGameStateFrameIndex() override
	{
		auto pedBase = GetNode<CPedGameStateDataNode>();

		return (pedBase) ? pedBase->frameIndex : 0;
	}

	virtual CVehicleGameStateNodeData* GetVehicleGameState() override
	{
		auto [hasVdn, vehNode] = GetData<CVehicleGameStateDataNode>();

		return (hasVdn) ? &vehNode->data : nullptr;
	}

	virtual CPedTaskTreeDataNodeData* GetPedTaskTree() override
	{
		return nullptr;
	}

	virtual CPlaneGameStateDataNodeData* GetPlaneGameState() override
	{
		return nullptr;
	}

	virtual CPlaneControlDataNodeData* GetPlaneControl() override
	{
		return nullptr;
	}

	virtual CTrainGameStateDataNodeData* GetTrainState() override
	{
		return nullptr;
	}

	virtual CPlayerGameStateNodeData* GetPlayerGameState() override
	{
		return nullptr;
	}

	virtual CVehicleAppearanceNodeData* GetVehicleAppearance() override
	{
		return nullptr;
	}

	virtual CPedHealthNodeData* GetPedHealth() override
	{
		return nullptr;
	}

	virtual CVehicleHealthNodeData* GetVehicleHealth() override
	{
		return nullptr;
	}

	virtual CPedOrientationNodeData* GetPedOrientation() override
	{
		auto [hasNode, node] = GetData<CPedOrientationDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CEntityOrientationNodeData* GetEntityOrientation() override
	{
		auto [hasNode, node] = GetData<CEntityOrientationDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CObjectOrientationNodeData* GetObjectOrientation() override
	{
#if 0
		auto [hasNode, node] = GetData<CObjectOrientationDataNode>();

		return (hasNode) ? &node->data : nullptr;
#endif

		return nullptr;
	}

	virtual CVehicleAngVelocityNodeData* GetAngVelocity() override
	{
		{
			auto [hasNode, node] = GetData<CVehicleAngVelocityDataNode>();

			if (hasNode)
			{
				return &node->data;
			}
		}

		auto [hasNode, node] = GetData<CPhysicalAngVelocityDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CPhysicalVelocityNodeData* GetVelocity() override
	{
		auto [hasNode, node] = GetData<CPhysicalVelocityDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CBaseAttachNodeData* GetAttachment() override
	{
		return nullptr;
	}

	virtual CObjectGameStateNodeData* GetObjectGameState() override
	{
		return nullptr;
	}

	virtual CDummyObjectCreationNodeData* GetDummyObjectState() override
	{
		return nullptr;
	}

	virtual CHeliHealthNodeData* GetHeliHealth() override
	{
		return nullptr;
	}

	virtual CVehicleDamageStatusNodeData* GetVehicleDamageStatus() override
	{
		return nullptr;
	}

	virtual void CalculatePosition() override
	{
		// TODO: cache it?
	}

	virtual bool GetPopulationType(ePopType* popType) override
	{
#if 0
		auto[hasVcn, vehCreationNode] = GetData<CVehicleCreationDataNode>();

		if (hasVcn)
		{
			*popType = vehCreationNode->m_popType;
			return true;
		}

		auto[hasPcn, pedCreationNode] = GetData<CPedCreationDataNode>();

		if (hasPcn)
		{
			*popType = pedCreationNode->m_popType;
			return true;
		}

		// TODO: objects(?)
#endif

		return false;
	}

	virtual bool GetModelHash(uint32_t* modelHash) override
	{
#if 0
		auto[hasVcn, vehCreationNode] = GetData<CVehicleCreationDataNode>();

		if (hasVcn)
		{
			*modelHash = vehCreationNode->m_model;
			return true;
		}

		auto[hasPan, playerAppearanceNode] = GetData<CPlayerAppearanceDataNode>();

		if (hasPan)
		{
			*modelHash = playerAppearanceNode->model;
			return true;
		}

		auto[hasPcn, pedCreationNode] = GetData<CPedCreationDataNode>();

		if (hasPcn)
		{
			*modelHash = pedCreationNode->m_model;
			return true;
		}

		auto[hasOcn, objectCreationNode] = GetData<CObjectCreationDataNode>();

		if (hasOcn)
		{
			*modelHash = objectCreationNode->m_model;
			return true;
		}
#endif

		return false;
	}

	virtual bool GetScriptHash(uint32_t* scriptHash) override
	{
		auto[hasSin, scriptInfoNode] = GetData<CEntityScriptInfoDataNode>();

		if (hasSin)
		{
			*scriptHash = scriptInfoNode->m_scriptHash;
			return true;
		}

		return false;
	}

	virtual bool IsEntityVisible(bool* visible) override
	{
		*visible = true;
		return true;
	}

	virtual void Parse(SyncParseState& state) final override
	{
		std::unique_lock<std::mutex> lock(mutex);

		//trace("parsing root\n");
		state.objType = 0;

		if (state.syncType == 2 || state.syncType == 4)
		{
			// mA0 flag
			state.objType = state.buffer.ReadBit();
		}

		// REDM1S: only RDR3
		state.buffer.ReadBit();

		root.Parse(state);
	}

	virtual bool Unparse(SyncUnparseState& state) final override
	{
		std::unique_lock<std::mutex> lock(mutex);

		state.objType = 0;

		if (state.syncType == 2 || state.syncType == 4)
		{
			state.objType = 1;

			state.buffer.WriteBit(1);
		}

		// REDM1S: only RDR3
		state.buffer.WriteBit(0);

		return root.Unparse(state);
	}

	virtual void Visit(const SyncTreeVisitor& visitor) final override
	{
		std::unique_lock<std::mutex> lock(mutex);

		root.Visit(visitor);
	}
};

using CAnimalSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CAnimalCreationDataNode>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435984c0>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedScriptGameStateUncommonDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435981a0>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598fb0>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598e20>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598b00>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359b8a8>
				>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, CPedScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, DataNode_1435992d0>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359e920>,
					NodeWrapper<NodeIds<127, 127, 1>, DataNode_14359e790>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, DataNode_143599dc0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435995f0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599780>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599910>
			>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599aa0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599c30>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599f50>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359a8b0>,
			NodeWrapper<NodeIds<86, 86, 0>, DataNode_14359aa40>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143598c90>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359eab0>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ec40>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a590>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedStandingOnObjectDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosMapNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosNavMeshDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a270>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>,
			NodeWrapper<NodeIds<4, 4, 1>, CPedTaskSequenceDataNode>
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
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode>
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
			NodeIds<127, 87, 0>,
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
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode>
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
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode>
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
		ParentNode<
			NodeIds<127, 127, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CDoorDamageDataNode>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CDoorMovementDataNode>
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
			NodeIds<127, 86, 0>,
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
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode>,
			NodeWrapper<NodeIds<86, 86, 0>, CHeliHealthDataNode>
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
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594478>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594dd8>,
					NodeWrapper<NodeIds<127, 127, 0>, CObjectAITaskDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CObjectAITaskDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, CObjectScriptGameStateDataNode>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CObjectSectorDataNode>,
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
			NodeIds<127, 86, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435984c0>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedScriptGameStateUncommonDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435981a0>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598fb0>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598e20>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598b00>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359b8a8>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, CPedScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, DataNode_1435992d0>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, DataNode_143599dc0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435995f0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599780>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599910>
			>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599aa0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599c30>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599f50>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359a8b0>,
			NodeWrapper<NodeIds<86, 86, 0>, DataNode_14359aa40>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143598c90>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a400>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedFacialAppearanceDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359b588>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedWeaponDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedVehicleDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ba38>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359bbc8>,
			NodeWrapper<NodeIds<87, 87, 1>, DataNode_14359b0d8>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedStandingOnObjectDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosMapNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosNavMeshDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a270>
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
				NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>,
				NodeWrapper<NodeIds<127, 127, 1>, CPickupScriptGameStateNode>,
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalGameStateDataNode>,
				NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>,
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>,
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalHealthDataNode>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode>
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
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435a0a20>
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
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode>
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
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435984c0>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedScriptGameStateUncommonDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435981a0>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598fb0>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598e20>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598b00>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359b8a8>
				>,
				ParentNode<
					NodeIds<127, 87, 0>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>,
					NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359cd00>,
					NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ce90>,
					NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359d020>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, DataNode_143599dc0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435995f0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599780>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599910>
			>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599aa0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599c30>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599f50>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359a8b0>,
			NodeWrapper<NodeIds<86, 86, 0>, DataNode_14359aa40>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143598c90>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerAppearanceDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerCharacterCreatorDataNode>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerAmbientModelStreamingDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerGamerDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359db10>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerVoiceDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerWeaponInventoryDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedWeaponDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedVehicleDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerHealthDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerSpawnSearchDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerAudioScriptBankDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359dfc0>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359dfc0>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359dfc0>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359dfc0>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359dfc0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerGoalsDataNode>,
			NodeWrapper<NodeIds<87, 87, 1>, DataNode_14359b0d8>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedFacialAppearanceDataNode>
		>,
		ParentNode<
			NodeIds<87, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedStandingOnObjectDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerSectorPosNode>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerCameraDataNode>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerCameraUncommonDataNode>,
			NodeWrapper<NodeIds<86, 86, 0>, DataNode_14359d660>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>
		>
	>
>;
using CTrailerSyncTree = SyncTree<
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
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode>
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
using CTrainSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode>
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
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CTrainGameStateUncommonDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CTrainGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CTrainControlDataNode>
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
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode>
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
using CDraftVehSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode>,
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode>,
			NodeWrapper<NodeIds<1, 0, 0>, CDraftVehCreationDataNode>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
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
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CDraftVehGameStateDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CDraftVehHorseGameStateDataNode>,
			NodeWrapper<NodeIds<86, 86, 0>, CDraftVehHorseHealthDataNode>
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
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CDraftVehControlDataNode>
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
using CStatsTrackerSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 127, 0>,
		NodeWrapper<NodeIds<127, 127, 0>, CStatsTrackerGameStateDataNode>
	>
>;
using CPropSetSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, CPropSetCreationDataNode>,
		NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CPropSetGameStateDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CPropSetUncommonGameStateDataNode>
	>
>;
using CAnimSceneSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<87, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CAnimSceneCreationDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CAnimSceneInfrequentDataNode>
		>,
		ParentNode<
			NodeIds<86, 86, 0>,
			NodeWrapper<NodeIds<86, 86, 0>, CAnimSceneFrequentDataNode>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>
		>
	>
>;
using CGroupScenarioSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<87, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CGroupScenarioCreationDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
			NodeWrapper<NodeIds<127, 127, 0>, CGroupScenarioEntitiesDataNode>
		>,
		ParentNode<
			NodeIds<86, 86, 0>,
			NodeWrapper<NodeIds<86, 86, 0>, CGroupScenarioFrequentDataNode>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>
		>
	>
>;
using CHerdSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>,
		NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
		NodeWrapper<NodeIds<87, 87, 0>, CHerdGameDataNode>,
		NodeWrapper<NodeIds<87, 87, 0>, CHerdMemberDataNode>,
		NodeWrapper<NodeIds<87, 87, 0>, CHerdPositionNode>
	>
>;
using CHorseSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CAnimalCreationDataNode>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435984c0>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedScriptGameStateUncommonDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435981a0>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598fb0>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598e20>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598b00>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359b8a8>
				>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, CPedScriptGameStateDataNode>,
					NodeWrapper<NodeIds<127, 127, 1>, DataNode_1435992d0>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359e920>,
					NodeWrapper<NodeIds<127, 127, 1>, DataNode_14359e790>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, DataNode_143599dc0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435995f0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599780>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599910>
			>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599aa0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599c30>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599f50>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359a8b0>,
			NodeWrapper<NodeIds<86, 86, 0>, DataNode_14359aa40>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143598c90>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359eab0>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ec40>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a590>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedStandingOnObjectDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosMapNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosNavMeshDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a270>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode>,
			NodeWrapper<NodeIds<4, 4, 1>, CPedTaskSequenceDataNode>
		>
	>
>;
using CWorldStateSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 87, 0>,
		NodeWrapper<NodeIds<87, 87, 0>, CWorldStateBaseDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>
	>
>;
using CWorldProjectileSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CProjectileCreationDataNode>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
			NodeWrapper<NodeIds<87, 87, 0>, CProjectileAttachNode>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CWorldProjectileDataNode>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>
		>
	>
>;
using CIncidentSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, CIncidentCreateDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentStateDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentCrimeSceneDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentPointOfInterestFinderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentDispatchDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode>,
		NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>
	>
>;
using CGuardzoneSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, CGuardzoneCreateDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZoneStateDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZoneGuardDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZonePointOfInterestFinderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZonePointOfInterestFinderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZonePointOfInterestFinderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZonePointOfInterestFinderDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143595bf0>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143595bf0>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143595bf0>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143595bf0>,
		NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>
	>
>;
using CPedGroupSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, CPedGroupCreateDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435929e0>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143592b70>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143592e90>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143592d00>,
		NodeWrapper<NodeIds<127, 127, 0>, CEntityScriptInfoDataNode>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>
	>
>;
using CCombatDirectorSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<87, 87, 0>, CCombatDirectorCreateUpdateDataNode>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359bef0>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359c080>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359c210>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359c3a0>,
		NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>,
		NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>
	>
>;
using CPedSharedTargetingSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_1435a1e78>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_1435a2010>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_1435a21a0>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_1435a2330>,
		NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode>,
		NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>
	>
>;
using CPersistentSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, DataNode_1435a24c0>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435a2658>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>
	>
>;
}
