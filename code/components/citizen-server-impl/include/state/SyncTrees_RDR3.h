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
struct CVehicleCreationDataNode : GenericSerializeDataNode<CVehicleCreationDataNode>
{ 
	uint32_t m_model;
	ePopType m_popType;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
		// model
		s.Serialize(32, m_model);

		// 4
		auto popType = (int)m_popType;
		s.Serialize(4, popType);
		m_popType = (ePopType)popType;

		return true; 
	} 
};

struct CAutomobileCreationDataNode { };

struct CGlobalFlagsDataNode { };

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
	bool flag;
	bool flag2;
	bool flag3;
	bool isVisible;
	bool flag5;
	bool flag6;
	bool flag7;
	bool flag8;

	uint8_t val1;

	template<typename Serializer>
	bool Serialize(Serializer& s)
    {
		s.Serialize(flag);
		s.Serialize(flag2);
		s.Serialize(flag3);
		s.Serialize(isVisible);
		s.Serialize(flag5);
		s.Serialize(flag6);
		s.Serialize(flag7);
		s.Serialize(flag8);

		if (flag8)
		{
			s.Serialize(4, val1);
		}
		else
		{
			val1 = 0;
		}

		// more data follows
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

struct CEntityScriptGameStateDataNode { };
struct CPhysicalScriptGameStateDataNode { };
struct CVehicleScriptGameStateDataNode { };

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

struct CPhysicalAttachDataNode { };

struct CVehicleAppearanceDataNode
{
	CVehicleAppearanceNodeData data;

	bool Parse(SyncParseState& state)
	{
		return true;
	}
};

struct CVehicleDamageStatusDataNode { };
struct CVehicleComponentReservationDataNode { };

struct CVehicleHealthDataNode
{
	CVehicleHealthNodeData data;

	bool Parse(SyncParseState& state)
	{
		return true;
	}
};

struct CVehicleTaskDataNode { };

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

struct CPedCreationDataNode : GenericSerializeDataNode<CPedCreationDataNode>
{ 
	uint32_t m_model;
	ePopType m_popType;

	template<typename TSerializer>
	bool Serialize(TSerializer& s)
	{ 
		// 4
		auto popType = (int)m_popType;
		s.Serialize(4, popType);
		m_popType = (ePopType)popType;

		// model
		s.Serialize(32, m_model);

		return true;
	}
};

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

		s.SerializeRotation(data.rotX, data.rotY, data.rotZ);
		s.Serialize(2, data.quat.largest);
		s.Serialize(11, data.quat.integer_a);
		s.Serialize(11, data.quat.integer_b);
		s.Serialize(11, data.quat.integer_c);

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

struct CVehicleSteeringDataNode
{
	CVehicleSteeringNodeData data;

	bool Parse(SyncParseState& state)
	{
		data.steeringAngle = state.buffer.ReadSignedFloat(10, 1.0f);

		return true;
	}
};

struct CVehicleControlDataNode { };
struct CVehicleGadgetDataNode { };
struct CMigrationDataNode { };
struct CPhysicalMigrationDataNode { };
struct CPhysicalScriptMigrationDataNode { };
struct CVehicleProximityMigrationDataNode { };
struct CBikeGameStateDataNode { };

struct CBoatGameStateDataNode
{
	CBoatGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		bool lockedToXY = state.buffer.ReadBit();

		if (lockedToXY)
		{
			float frontAnchorCoordsX = state.buffer.ReadSignedFloat(19, 27648.0f);
			float frontAnchorCoordsY = state.buffer.ReadSignedFloat(19, 27648.0f);
			float frontAnchorCoordsZ = state.buffer.ReadSignedFloat(19, 4416.0f) - 1700.0f;

			float backAnchorCoordsX = state.buffer.ReadSignedFloat(19, 27648.0f);
			float backAnchorCoordsY = state.buffer.ReadSignedFloat(19, 27648.0f);
			float backAnchorCoordsZ = state.buffer.ReadSignedFloat(19, 4416.0f) - 1700.0f;
		}

		int boatWreckedAction = state.buffer.Read<int>(2);
		bool forceLowLodAnchorMode = state.buffer.ReadBit(); // 0x75B49ACD73617437
		bool entityRequiresMoreExpensiveRiverCheck = state.buffer.ReadBit(); // 0x850C940EE3E7B8B5
		bool unk50 = state.buffer.ReadBit();
		bool unk55 = state.buffer.ReadBit();
		bool unk52 = state.buffer.ReadBit();
		bool forcedBoatLocationWhenAnchored = state.buffer.ReadBit();

		bool unk54 = state.buffer.ReadBit();

		bool movementResistant = state.buffer.ReadBit(); // resistance >= 0.0

		if (movementResistant)
		{
			bool fullMovementResistance = state.buffer.ReadBit(); // resistance > 1000.0

			if (!fullMovementResistance)
			{
				float movementResistance = state.buffer.ReadSignedFloat(16, 1000.0f);
			}
		}

		bool unk51 = state.buffer.ReadBit();

		// Related to move controls
		if (unk51)
		{
			int unk12 = state.buffer.Read<int>(4);
			int unk13 = state.buffer.Read<int>(2);
			bool unk57 = state.buffer.ReadBit();
			bool unk58 = state.buffer.ReadBit();
		}

		bool unk59 = state.buffer.ReadBit();

		if (unk59)
		{
			float sinkEndTime = state.buffer.ReadSignedFloat(8, 1.0f);
			bool isWrecked = state.buffer.ReadBit();

			data.sinkEndTime = sinkEndTime;
			data.isWrecked = isWrecked;
		}
		else
		{
			data.sinkEndTime = 0.0f;
			data.isWrecked = false;
		}

		data.lockedToXY = lockedToXY;

		return true;
	}
};

struct CDoorMovementDataNode { };
struct CDoorScriptInfoDataNode { };
struct CDoorScriptGameStateDataNode { };
struct CHeliHealthDataNode { };
struct CHeliControlDataNode { };

struct CObjectCreationDataNode : GenericSerializeDataNode<CObjectCreationDataNode>
{
	uint32_t m_unk2; // mostly 0
	uint32_t m_createdBy;
	uint32_t m_scriptHash;
	bool m_unk6; // mostly false
	bool m_unk7; // mostly false
	uint32_t m_model;
	bool m_hasInitPhysics;
	CDummyObjectCreationNodeData dummy;
	bool m_hasLodDist;
	uint16_t m_lodDist;
	uint16_t m_maxHealth;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
		bool unkBool = m_unk2 != 0;
		s.Serialize(unkBool);

		if (unkBool)
		{
			s.Serialize(32, m_unk2);
		}
		else
		{
			m_unk2 = 0;
		}

		s.Serialize(5, m_createdBy);

		bool hasScript = m_scriptHash != 0;
		s.Serialize(hasScript);

		if (hasScript)
		{
			s.Serialize(32, m_scriptHash);
		}
		else
		{
			m_scriptHash = 0;
		}

		s.Serialize(m_unk6);
		s.Serialize(m_unk7);

		if (m_createdBy != 3 && (m_createdBy > 0x12 || (409602 & (1 << m_createdBy)) == 0))
		{
			s.Serialize(32, m_model);
			s.Serialize(m_hasInitPhysics);
		}
		else
		{
			s.SerializePosition(31, dummy.dummyPosX, dummy.dummyPosY, dummy.dummyPosZ);

			s.Serialize(dummy.playerWantsControl);
			s.Serialize(dummy.hasFragGroup);
			s.Serialize(dummy.isBroken);
			s.Serialize(dummy.unk11);
			s.Serialize(dummy.hasExploded);
			s.Serialize(dummy._explodingEntityExploded);
			// s.Serialize(dummy.keepRegistered); // 1 bool between hasFragGroup & _hasRelatedDummy needs to be deleted
			s.Serialize(dummy._hasRelatedDummy);

			if (dummy.hasFragGroup)
			{
				s.Serialize(4, dummy.fragGroupIndex);
			}

			if (!dummy._hasRelatedDummy)
			{
				int ownershipToken = 0;
				s.Serialize(10, ownershipToken);

				float objectPosX = 0.0f, objectPosY = 0.0f, objectPosZ = 0.0f;
				s.SerializePosition(19, objectPosX, objectPosY, objectPosZ);

				float objectRotX = 0.0f, objectRotY = 0.0f, objectRotZ = 0.0f;
				s.SerializeRotation(objectRotX, objectRotY, objectRotZ);
			}
		}

		s.Serialize(m_hasLodDist);

		if (m_hasLodDist)
		{
			s.Serialize(16, m_lodDist);
		}

		bool hasMaxHealth = m_maxHealth != 0;
		s.Serialize(hasMaxHealth);

		if (hasMaxHealth)
		{
			s.Serialize(12, m_maxHealth);
		}
		else
		{
			m_maxHealth = 0;
		}

		return true;
	}
};

struct CObjectGameStateDataNode { };
struct CObjectScriptGameStateDataNode { };
struct CPhysicalHealthDataNode { };

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
//struct CPedCreationDataNode { };
struct CPedScriptCreationDataNode { };
//struct CPedGameStateDataNode { };
struct CPedComponentReservationDataNode { };
struct CPedScriptGameStateDataNode { };
struct CPedAttachDataNode { };

struct CPedHealthDataNode
{
	CPedHealthNodeData data;

	bool Parse(SyncParseState& state)
	{
		return true;
	}
};

struct CPedMovementGroupDataNode { };
struct CPedAIDataNode { };
struct CPedAppearanceDataNode { };

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

struct CPedMovementDataNode { };
struct CPedTaskTreeDataNode { };
struct CPedTaskSpecificDataNode { };

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

struct CPedSectorPosNavMeshNode { };
struct CPedInventoryDataNode { };
struct CPedTaskSequenceDataNode { };
struct CPickupCreationDataNode { };
struct CPickupScriptGameStateNode { };
struct CPickupSectorPosNode { };
struct CPickupPlacementCreationDataNode { };
struct CPickupPlacementStateDataNode { };
struct CPlaneGameStateDataNode { };
struct CPlaneControlDataNode { };
struct CSubmarineGameStateDataNode { };
struct CSubmarineControlDataNode { };
struct CTrainGameStateDataNode { };
struct CPlayerCreationDataNode { };
struct CPlayerGameStateDataNode { };

struct CPlayerAppearanceDataNode
{
	uint32_t model;

	bool Parse(SyncParseState& state)
	{
		model = state.buffer.Read<uint32_t>(32);

		return true;
	}
};

struct CPlayerPedGroupDataNode { };
struct CPlayerAmbientModelStreamingNode { };
struct CPlayerGamerDataNode { };
struct CPlayerExtendedGameStateNode { };

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

struct CDraftVehCreationDataNode { };
struct CStatsTrackerGameStateDataNode { };
struct CWorldStateBaseDataNode { };
struct CIncidentCreateDataNode { };
struct CGuardzoneCreateDataNode { };
struct CPedGroupCreateDataNode { };

struct CAnimalCreationDataNode : GenericSerializeDataNode<CAnimalCreationDataNode>
{
	ePopType m_popType;
	uint32_t m_model;
	uint16_t randomSeed;
	bool isStanding;

	template<typename TSerializer>
	bool Serialize(TSerializer& s)
	{
		uint32_t popType = (uint32_t)m_popType;
		s.Serialize(4, popType);
		m_popType = (ePopType)popType;

		s.Serialize(32, m_model);

		s.Serialize(16, randomSeed);
		s.Serialize(isStanding);

		return true;
	}
};

struct CProjectileCreationDataNode { };
struct CPedStandingOnObjectDataNode { };
struct CProjectileAttachNode { };
struct CHerdMemberDataNode { };
struct CHerdGameDataNode { };
struct CAnimSceneCreationDataNode { };
struct CAnimSceneFrequentDataNode { };
struct CAnimSceneInfrequentDataNode { };
struct CGroupScenarioFrequentDataNode { };
struct CGroupScenarioEntitiesDataNode { };
struct CGroupScenarioCreationDataNode { };
struct CPlayerWeaponInventoryDataNode { };
struct CPropSetGameStateDataNode { };
struct CPropSetUncommonGameStateDataNode { };
struct CIncidentStateDataNode { };
struct CIncidentCrimeSceneDataNode { };
struct CIncidentPointOfInterestFinderDataNode { };
struct CIncidentDispatchDataNode { };
struct CIncidentOrderDataNode { };
struct CGuardZoneStateDataNode { };
struct CGuardZoneGuardDataNode { };
struct CGuardZonePointOfInterestFinderDataNode { };
struct CCombatDirectorCreateUpdateDataNode { };
struct CPedWeaponDataNode { };
struct CPedVehicleDataNode : GenericSerializeDataNode<CPedVehicleDataNode>
{
	CPedVehicleNodeData data;

	bool wasInVehicle;
	bool wasOnHorse;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{

		bool wasInVehicle = data.curVehicle != 0;
		s.Serialize(wasInVehicle);

		if (wasInVehicle)
		{
			s.Serialize(data.inVehicle);
			s.Serialize(13, data.curVehicle);
		}
		else
		{
			data.curVehicle = 0;
			data.inVehicle = false;
		}

		bool wasOnHorse = data.curHorse != 0;
		s.Serialize(wasOnHorse);
		if (wasOnHorse)
		{
			s.Serialize(data.onHorse);
			s.Serialize(13, data.curHorse);
		}
		else
		{
			data.curHorse = 0;
			data.onHorse = false;
		}

		if (data.onHorse || data.inVehicle)
		{
			s.Serialize(5, data.curSeat);

			if (data.onHorse && data.lastHorsePedWasOn == 0)
			{
				data.lastHorsePedWasOn = data.curHorse;
			}

			if (data.inVehicle && data.lastVehiclePedWasIn == 0)
			{
				data.lastVehiclePedWasIn = data.curVehicle;
			}
		}
		else
		{
			data.curSeat = 0;

			if (!data.onHorse && data.lastHorsePedWasOn != data.curHorse)
			{
				data.lastHorsePedWasOn = data.curHorse;
			}

			if (!data.inVehicle && data.lastVehiclePedWasIn != data.curVehicle)
			{
				data.lastVehiclePedWasIn = data.curVehicle;
			}
		}

		return true;
	}
};
struct CPlayerCharacterCreatorDataNode { };
struct CPlayerAmbientModelStreamingDataNode { };
struct CPlayerVoiceDataNode { };
struct CPlayerHealthDataNode { };
struct CPlayerSpawnSearchDataNode { };
struct CPlayerAudioScriptBankDataNode { };
struct CPlayerGoalsDataNode { };
struct CPlayerCameraUncommonDataNode { };
struct CObjectAITaskDataNode { };
struct CDraftVehControlDataNode { };
struct CDraftVehHorseHealthDataNode { };
struct CDraftVehHorseGameStateDataNode { };
struct CDraftVehGameStateDataNode { };
struct CTrainControlDataNode { };
struct CTrainGameStateUncommonDataNode { };
struct CVehicleCommonDataNode { };
struct CDoorDamageDataNode { };
struct CPedSectorPosNavMeshDataNode { };
struct CPedScriptGameStateUncommonDataNode { };
struct CPedFacialAppearanceDataNode { };

// REDM1S: unknown rdr3 data nodes (addresses are 1311.20)
struct DataNode_1435984c0 { };
struct DataNode_143598fb0 { };
struct DataNode_143598e20 { };
struct DataNode_143598b00 { };
struct DataNode_143594ab8 { };
struct DataNode_14359b8a8 { };
struct DataNode_1435992d0 { };
struct DataNode_14359e920 { };
struct DataNode_14359e790 { };
struct DataNode_143599dc0 { };
struct DataNode_1435995f0 { };
struct DataNode_143599780 { };
struct DataNode_143599910 { };
struct DataNode_143599aa0 { };
struct DataNode_143599c30 { };
struct DataNode_143599f50 { };
struct DataNode_14359a8b0 { };
struct DataNode_14359aa40 { };
struct DataNode_143598c90 { };
struct DataNode_14359eab0 { };
struct DataNode_14359ec40 { };
struct DataNode_14359a590 { };
struct DataNode_14359abd0 { };
struct DataNode_14359ad88 { };
struct DataNode_14359a270 { };
struct DataNode_143594478 { };
struct DataNode_143594dd8 { };
struct DataNode_14359a400 { };
struct DataNode_14359b588 { };
struct DataNode_14359ba38 { };
struct DataNode_14359bbc8 { };
struct DataNode_14359b0d8 { };
struct DataNode_1435a0a20 { };
struct DataNode_14359cd00 { };
struct DataNode_14359ce90 { };
struct DataNode_14359d020 { };
struct DataNode_14359db10 { };
struct DataNode_14359dfc0 { };
struct DataNode_14359d660 { };
struct DataNode_143595bf0 { };
struct DataNode_1435929e0 { };
struct DataNode_143592b70 { };
struct DataNode_143592e90 { };
struct DataNode_143592d00 { };
struct DataNode_14359bef0 { };
struct DataNode_14359c080 { };
struct DataNode_14359c210 { };
struct DataNode_14359c3a0 { };
struct DataNode_1435a1e78 { };
struct DataNode_1435a2010 { };
struct DataNode_1435a21a0 { };
struct DataNode_1435a2330 { };
struct DataNode_1435a24c0 { };
struct DataNode_1435a2658 { };
struct DataNode_1435981a0 { };

template<typename TNode>
struct SyncTree : public SyncTreeBaseImpl<TNode, true>
{
	virtual void GetPosition(float* posOut) override
	{
		auto [hasSdn, secDataNode] = this->template GetData<CSectorDataNode>();
		auto [hasSpdn, secPosDataNode] = this->template GetData<CSectorPositionDataNode>();
		auto [hasWpdn, projectileDataNode] = this->template GetData<CWorldProjectileDataNode>();
		auto [hasPspdn, playerSecPosDataNode] = this->template GetData<CPlayerSectorPosNode>();
		auto [hasOsdn, objectSecDataNode] = this->template GetData<CObjectSectorDataNode>();
		auto [hasOspdn, objectSecPosDataNode] = this->template GetData<CObjectSectorPosNode>();
		auto [hasPspmdn, pedSecPosMapDataNode] = this->template GetData<CPedSectorPosMapNode>();
		auto [hasDoor, doorCreationDataNode] = this->template GetData<CDoorCreationDataNode>();
		auto [hasPropSet, propSetCreationDataNode] = this->template GetData<CPropSetCreationDataNode>();
		auto [hasHpn, herdPosNode] = this->template GetData<CHerdPositionNode>();

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
		auto [hasCdn, cameraNode] = this->template GetData<CPlayerCameraDataNode>();

		return (hasCdn) ? &cameraNode->data : nullptr;
	}

	virtual CPlayerWantedAndLOSNodeData* GetPlayerWantedAndLOS() override
	{
		return nullptr;
	}

	virtual CPedGameStateNodeData* GetPedGameState() override
	{
		auto [hasPdn, pedNode] = this->template GetData<CPedGameStateDataNode>();

		return (hasPdn) ? &pedNode->data : nullptr;
	}

	virtual uint64_t GetPedGameStateFrameIndex() override
	{
		auto pedBase = this->template GetNode<CPedGameStateDataNode>();

		return (pedBase) ? pedBase->frameIndex : 0;
	}

	virtual CVehicleGameStateNodeData* GetVehicleGameState() override
	{
		auto [hasVdn, vehNode] = this->template GetData<CVehicleGameStateDataNode>();

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
		auto [hasNode, node] = this->template GetData<CPedOrientationDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CEntityOrientationNodeData* GetEntityOrientation() override
	{
		auto [hasNode, node] = this->template GetData<CEntityOrientationDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CObjectOrientationNodeData* GetObjectOrientation() override
	{
#if 0
		auto [hasNode, node] = this->template GetData<CObjectOrientationDataNode>();

		return (hasNode) ? &node->data : nullptr;
#endif

		return nullptr;
	}

	virtual CVehicleAngVelocityNodeData* GetAngVelocity() override
	{
		{
			auto [hasNode, node] = this->template GetData<CVehicleAngVelocityDataNode>();

			if (hasNode)
			{
				return &node->data;
			}
		}

		auto [hasNode, node] = this->template GetData<CPhysicalAngVelocityDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CPhysicalVelocityNodeData* GetVelocity() override
	{
		auto [hasNode, node] = this->template GetData<CPhysicalVelocityDataNode>();

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

	virtual CVehicleSteeringNodeData* GetVehicleSteeringData() override
	{
		auto [hasNode, node] = this->template GetData<CVehicleSteeringDataNode>();

		return hasNode ? &node->data : nullptr;
	}

	virtual CEntityScriptGameStateNodeData* GetEntityScriptGameState() override
	{
		return nullptr;
	}

	virtual CVehicleDamageStatusNodeData* GetVehicleDamageStatus() override
	{
		return nullptr;
	}

	virtual CBoatGameStateNodeData* GetBoatGameState() override
	{
		auto [hasNode, node] = this->template GetData<CBoatGameStateDataNode>();

		return hasNode ? &node->data : nullptr;
	}

	virtual CPedMovementGroupNodeData* GetPedMovementGroup() override
	{
		return nullptr;
	}

	virtual CPedAINodeData* GetPedAI() override
	{
		return nullptr;
	}

	virtual void CalculatePosition() override
	{
		// TODO: cache it?
	}

	virtual bool GetPopulationType(ePopType* popType) override
	{
		auto[hasVcn, vehCreationNode] = this->template GetData<CVehicleCreationDataNode>();

		if (hasVcn)
		{
			*popType = vehCreationNode->m_popType;
			return true;
		}

		auto[hasPcn, pedCreationNode] = this->template GetData<CPedCreationDataNode>();

		if (hasPcn)
		{
			*popType = pedCreationNode->m_popType;
			return true;
		}

		// TODO: objects(?)

		return false;
	}

	virtual bool GetModelHash(uint32_t* modelHash) override
	{
		auto[hasVcn, vehCreationNode] = this->template GetData<CVehicleCreationDataNode>();

		if (hasVcn)
		{
			*modelHash = vehCreationNode->m_model;
			return true;
		}

		auto[hasPcn, pedCreationNode] = this->template GetData<CPedCreationDataNode>();

		if (hasPcn)
		{
			*modelHash = pedCreationNode->m_model;
			return true;
		}

		auto[hasOcn, objectCreationNode] = this->template GetData<CObjectCreationDataNode>();

		if (hasOcn)
		{
			*modelHash = objectCreationNode->m_model;
			return true;
		}

		auto[hasPan, playerAppearanceNode] = this->template GetData<CPlayerAppearanceDataNode>();

		if (hasPan)
		{
			*modelHash = playerAppearanceNode->model;
			return true;
		}

		auto [hasAcn, animalCreationNode] = this->template GetData<CAnimalCreationDataNode>();

		if (hasAcn)
		{
			*modelHash = animalCreationNode->m_model;
			return true;
		}

		return false;
	}

	virtual bool GetScriptHash(uint32_t* scriptHash) override
	{
		auto[hasSin, scriptInfoNode] = this->template GetData<CEntityScriptInfoDataNode>();

		if (hasSin)
		{
			*scriptHash = scriptInfoNode->m_scriptHash;
			return true;
		}

		return false;
	}

	virtual bool IsEntityVisible(bool* visible) override
	{
		auto [hasNode, node] = this->template GetData<CPhysicalGameStateDataNode>();

		if (hasNode)
		{
			*visible = node->isVisible;
			return true;
		}

		return false;
	}

	virtual CPedVehicleNodeData* GetPedVehicleData() override
	{
		auto [hasNode, node] = this->template GetData<CPedVehicleDataNode>();

		return hasNode ? &node->data : nullptr;
	}
};

using CAnimalSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CAnimalCreationDataNode, 27>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435984c0, 19>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedScriptGameStateUncommonDataNode, 262>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435981a0, 63>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598fb0, 101>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598e20, 33>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598b00, 207>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359b8a8, 44>
				>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CPedScriptGameStateDataNode, 280>,
					NodeWrapper<NodeIds<127, 127, 1>, DataNode_1435992d0, 68>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359e920, 31>,
					NodeWrapper<NodeIds<127, 127, 1>, DataNode_14359e790, 61>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, DataNode_143599dc0, 23>,
			ParentNode<
				NodeIds<127, 127, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435995f0, 21>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599780, 74>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599910, 36>
			>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599aa0, 21>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599c30, 26>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599f50, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359a8b0, 288>,
			NodeWrapper<NodeIds<86, 86, 0>, DataNode_14359aa40, 31>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143598c90, 42>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359eab0, 8>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ec40, 57>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a590, 290>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedStandingOnObjectDataNode, 13>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode, 3>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode, 47>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 253>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 94>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 50>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosMapNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosNavMeshDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a270, 53>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 4, 1>, CPedTaskSequenceDataNode, 4029>
		>
	>
>;
using CAutomobileSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 21>,
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode, 4>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 52>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 55>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 126>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode, 6>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 15>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 101>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 44>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode, 35>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 8>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 68>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 55>
		>
	>
>;
using CBikeSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 21>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 52>,
					NodeWrapper<NodeIds<127, 127, 0>, CBikeGameStateDataNode, 1>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 55>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 126>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode, 6>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 15>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 101>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 44>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode, 35>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 8>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 68>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 55>
		>
	>
>;
using CBoatSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 21>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 87, 0>,
				ParentNode<
					NodeIds<127, 87, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 52>,
					NodeWrapper<NodeIds<87, 87, 0>, CBoatGameStateDataNode, 21>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 55>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 126>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode, 6>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 15>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 101>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 44>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode, 35>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 8>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 68>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 55>
		>
	>
>;
using CDoorSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CDoorCreationDataNode, 19>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
			NodeWrapper<NodeIds<127, 127, 0>, CDoorDamageDataNode, 34>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CDoorMovementDataNode, 2>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>
		>
	>
>;
using CHeliSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 21>,
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode, 4>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 52>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 55>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 126>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode, 6>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 15>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 101>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 44>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode, 35>,
			NodeWrapper<NodeIds<86, 86, 0>, CHeliHealthDataNode, 3>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 8>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 68>,
				NodeWrapper<NodeIds<86, 86, 0>, CHeliControlDataNode, 5>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 55>
		>
	>
>;
using CObjectSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CObjectCreationDataNode, 108>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594478, 18>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594dd8, 89>,
					NodeWrapper<NodeIds<127, 127, 0>, CObjectAITaskDataNode, 79>,
					NodeWrapper<NodeIds<127, 127, 0>, CObjectAITaskDataNode, 79>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>,
					NodeWrapper<NodeIds<127, 127, 1>, CObjectScriptGameStateDataNode, 15>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CObjectSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CObjectSectorPosNode, 8>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalAngVelocityDataNode, 4>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>
		>
	>
>;
using CPedSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CPedCreationDataNode, 29>,
			NodeWrapper<NodeIds<1, 0, 1>, CPedScriptCreationDataNode, 1>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435984c0, 19>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedScriptGameStateUncommonDataNode, 262>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435981a0, 63>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598fb0, 101>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598e20, 33>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598b00, 207>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359b8a8, 44>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CPedScriptGameStateDataNode, 280>,
					NodeWrapper<NodeIds<127, 127, 1>, DataNode_1435992d0, 68>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, DataNode_143599dc0, 23>,
			ParentNode<
				NodeIds<127, 127, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435995f0, 21>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599780, 74>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599910, 36>
			>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599aa0, 21>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599c30, 26>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599f50, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359a8b0, 288>,
			NodeWrapper<NodeIds<86, 86, 0>, DataNode_14359aa40, 31>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143598c90, 42>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a400, 950>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedFacialAppearanceDataNode, 9>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359b588, 1226>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedWeaponDataNode, 85>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedVehicleDataNode, 6>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ba38, 337>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359bbc8, 140>,
			NodeWrapper<NodeIds<87, 87, 1>, DataNode_14359b0d8, 46>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedStandingOnObjectDataNode, 13>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode, 3>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode, 47>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 253>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 94>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 50>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosMapNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosNavMeshDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a270, 53>
		>,
		ParentNode<
			NodeIds<5, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<5, 0, 0>, CPedInventoryDataNode, 94>,
			NodeWrapper<NodeIds<4, 4, 1>, CPedTaskSequenceDataNode, 4029>
		>
	>
>;
using CPickupSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CPickupCreationDataNode, 227>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
				NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>
			>,
			ParentNode<
				NodeIds<127, 127, 1>,
				NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>,
				NodeWrapper<NodeIds<127, 127, 1>, CPickupScriptGameStateNode, 22>,
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalGameStateDataNode, 15>,
				NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
				NodeWrapper<NodeIds<127, 127, 1>, DataNode_143594478, 18>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPickupSectorPosNode, 8>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalAngVelocityDataNode, 4>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>
		>
	>
>;
using CPickupPlacementSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, CPickupPlacementCreationDataNode, 77>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
		NodeWrapper<NodeIds<127, 127, 0>, CPickupPlacementStateDataNode, 7>
	>
>;
using CPlaneSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 21>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 52>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 55>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 126>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode, 6>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 15>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 101>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 44>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode, 35>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435a0a20, 19>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 8>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 68>,
				NodeWrapper<NodeIds<86, 86, 0>, CPlaneControlDataNode, 5>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 55>
		>
	>
>;
using CSubmarineSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 21>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 87, 0>,
				ParentNode<
					NodeIds<127, 87, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 52>,
					NodeWrapper<NodeIds<87, 87, 0>, CSubmarineGameStateDataNode, 1>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 55>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 126>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode, 6>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 15>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 101>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 44>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode, 35>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 8>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 68>,
				NodeWrapper<NodeIds<86, 86, 0>, CSubmarineControlDataNode, 4>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 55>
		>
	>
>;
using CPlayerSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CPlayerCreationDataNode, 13>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			ParentNode<
				NodeIds<127, 87, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435984c0, 19>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedScriptGameStateUncommonDataNode, 262>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435981a0, 63>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598fb0, 101>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598e20, 33>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598b00, 207>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359b8a8, 44>
				>,
				ParentNode<
					NodeIds<127, 87, 0>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359cd00, 12>,
					NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ce90, 48>,
					NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359d020, 306>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, DataNode_143599dc0, 23>,
			ParentNode<
				NodeIds<127, 127, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435995f0, 21>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599780, 74>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599910, 36>
			>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599aa0, 21>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599c30, 26>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599f50, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359a8b0, 288>,
			NodeWrapper<NodeIds<86, 86, 0>, DataNode_14359aa40, 31>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143598c90, 42>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerAppearanceDataNode, 1854>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerCharacterCreatorDataNode, 395>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerAmbientModelStreamingDataNode, 3>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerGamerDataNode, 352>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359db10, 9>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerVoiceDataNode, 19>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerWeaponInventoryDataNode, 1330>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedWeaponDataNode, 85>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedVehicleDataNode, 6>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerHealthDataNode, 3>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerSpawnSearchDataNode, 67>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerAudioScriptBankDataNode, 40>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359dfc0, 420>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359dfc0, 420>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359dfc0, 420>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359dfc0, 420>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359dfc0, 420>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerGoalsDataNode, 186>,
			NodeWrapper<NodeIds<87, 87, 1>, DataNode_14359b0d8, 46>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedFacialAppearanceDataNode, 9>
		>,
		ParentNode<
			NodeIds<87, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedStandingOnObjectDataNode, 13>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode, 3>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode, 47>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 253>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 94>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 50>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerSectorPosNode, 10>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerCameraDataNode, 16>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerCameraUncommonDataNode, 9>,
			NodeWrapper<NodeIds<86, 86, 0>, DataNode_14359d660, 636>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>
		>
	>
>;
using CAutomobileSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 21>,
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode, 4>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 52>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 55>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 126>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode, 6>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 15>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 101>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 44>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode, 35>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 8>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 68>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 55>
		>
	>
>;
using CTrainSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 21>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 52>,
					NodeWrapper<NodeIds<127, 127, 0>, CTrainGameStateUncommonDataNode, 10>,
					NodeWrapper<NodeIds<127, 127, 0>, CTrainGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 0>, CTrainControlDataNode, 22>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 55>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 126>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode, 6>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 15>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 101>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 44>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode, 35>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 8>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 68>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 55>
		>
	>
>;
using CDraftVehSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 21>,
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode, 4>,
			NodeWrapper<NodeIds<1, 0, 0>, CDraftVehCreationDataNode, 33>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 52>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 55>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 32>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 126>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleCommonDataNode, 6>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 15>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 101>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 44>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleTaskDataNode, 35>,
			NodeWrapper<NodeIds<127, 127, 0>, CDraftVehGameStateDataNode, 1>,
			NodeWrapper<NodeIds<127, 127, 0>, CDraftVehHorseGameStateDataNode, 851>,
			NodeWrapper<NodeIds<86, 86, 0>, CDraftVehHorseHealthDataNode, 227>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 8>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 68>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CDraftVehControlDataNode, 24>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 55>
		>
	>
>;
using CStatsTrackerSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 127, 0>,
		NodeWrapper<NodeIds<127, 127, 0>, CStatsTrackerGameStateDataNode, 129>
	>
>;
using CPropSetSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, CPropSetCreationDataNode, 215>,
		NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode, 270>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
		NodeWrapper<NodeIds<127, 127, 0>, CPropSetGameStateDataNode, 115>,
		NodeWrapper<NodeIds<127, 127, 0>, CPropSetUncommonGameStateDataNode, 112>
	>
>;
using CAnimSceneSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<87, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CAnimSceneCreationDataNode, 27>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode, 270>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
			NodeWrapper<NodeIds<127, 127, 0>, CAnimSceneInfrequentDataNode, 700>
		>,
		ParentNode<
			NodeIds<86, 86, 0>,
			NodeWrapper<NodeIds<86, 86, 0>, CAnimSceneFrequentDataNode, 9>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>
		>
	>
>;
using CGroupScenarioSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<87, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CGroupScenarioCreationDataNode, 20>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode, 270>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
			NodeWrapper<NodeIds<127, 127, 0>, CGroupScenarioEntitiesDataNode, 38>
		>,
		ParentNode<
			NodeIds<86, 86, 0>,
			NodeWrapper<NodeIds<86, 86, 0>, CGroupScenarioFrequentDataNode, 1>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>
		>
	>
>;
using CHerdSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
		NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode, 270>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
		NodeWrapper<NodeIds<87, 87, 0>, CHerdGameDataNode, 2>,
		NodeWrapper<NodeIds<87, 87, 0>, CHerdMemberDataNode, 76>,
		NodeWrapper<NodeIds<87, 87, 0>, CHerdPositionNode, 18>
	>
>;
using CAnimalSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CAnimalCreationDataNode, 27>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 173>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 15>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435984c0, 19>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedScriptGameStateUncommonDataNode, 262>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435981a0, 63>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598fb0, 101>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598e20, 33>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143598b00, 207>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_143594ab8, 3>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359b8a8, 44>
				>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CPedScriptGameStateDataNode, 280>,
					NodeWrapper<NodeIds<127, 127, 1>, DataNode_1435992d0, 68>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 270>,
					NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359e920, 31>,
					NodeWrapper<NodeIds<127, 127, 1>, DataNode_14359e790, 61>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, DataNode_143599dc0, 23>,
			ParentNode<
				NodeIds<127, 127, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435995f0, 21>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599780, 74>,
				NodeWrapper<NodeIds<127, 127, 0>, DataNode_143599910, 36>
			>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599aa0, 21>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599c30, 26>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143599f50, 3>,
			NodeWrapper<NodeIds<127, 127, 0>, DataNode_14359a8b0, 288>,
			NodeWrapper<NodeIds<86, 86, 0>, DataNode_14359aa40, 31>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_143598c90, 42>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359eab0, 8>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ec40, 57>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a590, 290>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedStandingOnObjectDataNode, 13>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode, 3>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode, 47>,
			ParentNode<
				NodeIds<87, 87, 0>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 253>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 94>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359abd0, 50>,
				NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359ad88, 406>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosMapNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosNavMeshDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359a270, 53>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 7>,
			NodeWrapper<NodeIds<4, 4, 1>, CPedTaskSequenceDataNode, 4029>
		>
	>
>;
using CWorldStateSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 87, 0>,
		NodeWrapper<NodeIds<87, 87, 0>, CWorldStateBaseDataNode, 128>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>
	>
>;
using CWorldProjectileSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CProjectileCreationDataNode, 47>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
			NodeWrapper<NodeIds<87, 87, 0>, CProjectileAttachNode, 36>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CWorldProjectileDataNode, 16>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>
		>
	>
>;
using CIncidentSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, CIncidentCreateDataNode, 18>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentStateDataNode, 25>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentCrimeSceneDataNode, 169>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentPointOfInterestFinderDataNode, 556>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentDispatchDataNode, 88>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, CIncidentOrderDataNode, 31>,
		NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode, 270>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>
	>
>;
using CGuardzoneSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, CGuardzoneCreateDataNode, 9>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZoneStateDataNode, 136>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZoneGuardDataNode, 71>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZonePointOfInterestFinderDataNode, 556>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZonePointOfInterestFinderDataNode, 556>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZonePointOfInterestFinderDataNode, 556>,
		NodeWrapper<NodeIds<127, 127, 0>, CGuardZonePointOfInterestFinderDataNode, 556>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143595bf0, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143595bf0, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143595bf0, 31>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143595bf0, 31>,
		NodeWrapper<NodeIds<87, 87, 0>, CEntityScriptInfoDataNode, 270>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>
	>
>;
using CPedGroupSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, CPedGroupCreateDataNode, 1>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435929e0, 61>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143592b70, 354>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143592e90, 471>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_143592d00, 286>,
		NodeWrapper<NodeIds<127, 127, 0>, CEntityScriptInfoDataNode, 270>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>
	>
>;
using CCombatDirectorSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<87, 87, 0>, CCombatDirectorCreateUpdateDataNode, 17>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359bef0, 71>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359c080, 129>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359c210, 221>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_14359c3a0, 444>,
		NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
		NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>
	>
>;
using CPedSharedTargetingSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_1435a1e78, 1>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_1435a2010, 44>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_1435a21a0, 4>,
		NodeWrapper<NodeIds<87, 87, 0>, DataNode_1435a2330, 546>,
		NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
		NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>
	>
>;
using CPersistentSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, DataNode_1435a24c0, 24>,
		NodeWrapper<NodeIds<127, 127, 0>, DataNode_1435a2658, 163>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 17>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>
	>
>;
