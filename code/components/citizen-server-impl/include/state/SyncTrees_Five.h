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
	int m_randomSeed;
	bool m_carBudget;
	int m_maxHealth;
	int m_vehicleStatus;
	uint32_t m_creationToken;
	bool m_needsToBeHotwired;
	bool m_tyresDontBurst;
	bool m_usesSpecialFlightMode;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
		s.Serialize(32, m_model);
		s.Serialize(4, (uint8_t&)m_popType);

		s.Serialize(16, m_randomSeed);

		if (m_popType - 6 <= 1)
		{
			s.Serialize(m_carBudget);
		}

		s.Serialize(19, m_maxHealth);
		s.Serialize(3, m_vehicleStatus);
		s.Serialize(32, m_creationToken);
		s.Serialize(m_needsToBeHotwired);
		s.Serialize(m_tyresDontBurst);
		s.Serialize(m_usesSpecialFlightMode);

		return true;
	}
};

struct CAutomobileCreationDataNode
{
	bool allDoorsClosed;
	bool doorsClosed[10];

	bool Parse(SyncParseState& state)
	{
		allDoorsClosed = state.buffer.ReadBit();

		if (!allDoorsClosed)
		{
			for (int i = 0; i < 10; i++)
			{
				doorsClosed[i] = state.buffer.ReadBit();
			}
		}

		return true;
	}

	bool Unparse(SyncUnparseState& state)
	{
		rl::MessageBuffer& buffer = state.buffer;
		buffer.WriteBit(allDoorsClosed);

		if (!allDoorsClosed)
		{
			for (auto closed : doorsClosed)
			{
				buffer.WriteBit(closed);
			}
		}

		return true;
	}
};

struct CGlobalFlagsDataNode
{
	uint32_t globalFlags;
	uint32_t token;

	bool Parse(SyncParseState& state)
	{
		globalFlags = state.buffer.Read<uint32_t>(8);
		token = state.buffer.Read<uint32_t>(5);

		return true;
	}

	bool Unparse(SyncUnparseState& state)
	{
		rl::MessageBuffer& buffer = state.buffer;

		buffer.Write<uint32_t>(8, globalFlags);
		buffer.Write<uint32_t>(5, token);

		return true;
	}
};

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
	bool has_unk204;
	uint16_t unk204;

	int val1;

	bool unk5;

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
			if (Is2060())
			{
				s.Serialize(has_unk204);
				if (has_unk204)
				{
					s.Serialize(16, unk204);
				}
			}
		}
		else
		{
			val1 = 0;
		}

		if (Is2545())
		{
			s.Serialize(unk5);
		}

		return true;
	}
};

struct CVehicleGameStateDataNode
{
	CVehicleGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		int radioStation; 
		if (Is2545())
		{
			radioStation = state.buffer.Read<int>(7);
		}
		else
		{
			radioStation = state.buffer.Read<int>(6);
		}
		bool unk1 = state.buffer.ReadBit();
		int isEngineOn = state.buffer.ReadBit();
		int isEngineStarting = state.buffer.ReadBit();
		bool unk4 = state.buffer.ReadBit();
		int handbrake = state.buffer.ReadBit();
		bool unk6 = state.buffer.ReadBit();
		bool unk7 = state.buffer.ReadBit();
		int unk8 = state.buffer.ReadBit();

		data.radioStation = radioStation;
		data.isEngineOn = isEngineOn;
		data.isEngineStarting = isEngineStarting;
		data.handbrake = handbrake;

		if (!unk8)
		{
			int defaultHeadlights = state.buffer.ReadBit();
			data.defaultHeadlights = defaultHeadlights;

			if (!defaultHeadlights)
			{
				// NOTE: Even if xenon lights are not enabled, this will still work.
				int headlightsColour = state.buffer.Read<int>(8);
				data.headlightsColour = headlightsColour;
			}
			else
			{
				data.headlightsColour = 0;
			}

			int sirenOn = state.buffer.ReadBit();
			data.sirenOn = sirenOn;

			if (Is3407())
			{
				state.buffer.ReadBit();
			}

			bool unk12 = state.buffer.ReadBit();

			if (unk12)
			{
				bool unk13 = state.buffer.ReadBit();
			}

			bool unk14 = state.buffer.ReadBit();
			int unk15 = state.buffer.ReadBit();

			if (unk15)
			{
				uint8_t lockStatus = state.buffer.Read<uint8_t>(5);
				data.lockStatus = lockStatus;

				auto unk17 = state.buffer.Read<int>(7);
				auto unbreakableDoors = state.buffer.Read<int>(7);

				auto doorsOpen = state.buffer.Read<int>(7);
				data.doorsOpen = doorsOpen;

				int v20 = 0;
				do
				{
					if ((1 << v20) & doorsOpen)
					{
						auto doorPosition = state.buffer.Read<int>(4); // Status 0->7 7 == completely open
						data.doorPositions[v20] = doorPosition;
					}
					v20++;
				} while (v20 < 7);

				auto unk21 = state.buffer.Read<int>(8);

				int v22 = 0;
				do
				{
					if ((1 << v22) & unk21)
					{
						auto unk22 = state.buffer.Read<int>(5);
					}
					v22++;
				} while (v22 < 7);
			}
			else
			{
				data.lockStatus = 0;
				data.doorsOpen = 0;
			}

			bool anyWindowsOpen = state.buffer.ReadBit();

			if (anyWindowsOpen)
			{
				auto openWindows = state.buffer.Read<int>(6);
			}

			bool unk25 = state.buffer.ReadBit();
			bool unk26 = state.buffer.ReadBit();
			int isStationary = state.buffer.ReadBit();
			data.isStationary = isStationary;
			bool isParked = state.buffer.ReadBit();
			bool unk29 = state.buffer.ReadBit();
			bool unk30 = state.buffer.ReadBit();
			bool unk31 = state.buffer.ReadBit();

			if (unk31)
			{
				float unk32 = state.buffer.ReadFloat(10, 3000);
			}
		}
		else
		{
			data.isStationary = 0;
			data.defaultHeadlights = 1;
			data.headlightsColour = 0;
			data.sirenOn = 0;
			data.lockStatus = 0;
			data.doorsOpen = 0;
		}

		bool unk33 = state.buffer.ReadBit();

		if (unk33)
		{
			uint32_t unk34 = state.buffer.Read<uint32_t>(32);

			short unk35 = state.buffer.Read<short>(13);
		}

		bool unk36 = state.buffer.ReadBit();

		int v15 = 0x0;
		if (unk36)
		{
			v15 = 0x02;
			do
			{
				auto unk37 = state.buffer.Read<short>(13);
				v15--;
			} while (v15);
		}

		bool unk38 = state.buffer.ReadBit();

		if (unk38)
		{
			auto unk39 = state.buffer.Read<short>(13);
		}

		int lightsOn = state.buffer.ReadBit();
		data.lightsOn = lightsOn;

		int highbeamsOn = state.buffer.ReadBit();
		data.highbeamsOn = highbeamsOn;

		auto lightState = state.buffer.Read<int>(3); // SetVehicleLights

		bool unk43 = state.buffer.ReadBit();
		bool unk44 = state.buffer.ReadBit();
		bool unk45 = state.buffer.ReadBit();
		bool unk46 = state.buffer.ReadBit();
		bool unk47 = state.buffer.ReadBit();
		bool unk48 = state.buffer.ReadBit();
		auto unk49 = state.buffer.Read<int>(32);
		auto unk50 = state.buffer.Read<int>(3);
		bool unk51 = state.buffer.ReadBit();
		int hasBeenOwnedByPlayer = state.buffer.ReadBit();
		data.hasBeenOwnedByPlayer = hasBeenOwnedByPlayer;

		bool unk53 = state.buffer.ReadBit();
		bool unk54 = state.buffer.ReadBit();
		bool unk55 = state.buffer.ReadBit();
		bool unk56 = state.buffer.ReadBit();
		bool unk57 = state.buffer.ReadBit();
		bool unk58 = state.buffer.ReadBit();

		if (Is2699())
		{
			bool unk59 = state.buffer.ReadBit();
			bool unk60 = state.buffer.ReadBit();
		}

		int hasLock = state.buffer.ReadBit();
		data.hasLock = hasLock;

		if (hasLock != v15)
		{
			int lockedPlayers = state.buffer.Read<int>(32);
			data.lockedPlayers = lockedPlayers;
		}

		bool unk61 = state.buffer.ReadBit();
		int unk62 = state.buffer.ReadBit();

		if (unk62 != v15)
		{
			auto unk62_1 = state.buffer.Read<int>(32);
		}

		bool unk63 = state.buffer.ReadBit();
		bool unk64 = state.buffer.ReadBit();
		int unk65 = state.buffer.ReadBit();

		if (unk65 != v15)
		{
			auto unk66 = state.buffer.ReadFloat(8, 16);
		}

		bool unk67 = state.buffer.ReadBit();
		bool unk68 = state.buffer.ReadBit();

		if (Is2372())
		{
			auto v44 = state.buffer.ReadBit();
			if (v44 != v15)
			{
				uint8_t unk252 = state.buffer.Read<uint8_t>(7);
			}

			auto unk138 = state.buffer.ReadBit();
			if (unk138 != v15)
			{
				uint32_t unk240 = state.buffer.Read<uint32_t>(32);
			}

			auto unk139 = state.buffer.ReadBit();
			auto v46 = state.buffer.ReadBit();
			if (v46 != v15)
			{
				// Defaults to 1.0
				auto unk204 = state.buffer.ReadFloat(10, 2.f);
				auto unk200 = state.buffer.ReadFloat(10, 2.f);
			}
		}

		return true;
	}
};

struct CEntityScriptGameStateDataNode
{
	CEntityScriptGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		data.usesCollision = state.buffer.ReadBit();
		data.isFixed = state.buffer.ReadBit();

		bool completelyDisabledCollision = state.buffer.ReadBit();

		return true;
	}
};

struct CPhysicalScriptGameStateDataNode { };

struct CVehicleScriptGameStateDataNode
{
	ePopType m_popType;

	bool Parse(SyncParseState& state)
	{
		// Strings pulled from X360 TU0
		state.buffer.ReadBit(); // Has Freebies
		state.buffer.ReadBit(); // Can Be Visibly Damaged
		state.buffer.ReadBit(); // Is Drowning
		state.buffer.ReadBit(); // Part Of Convoy
		state.buffer.ReadBit(); // Vehicle Can Be Targeted
		state.buffer.ReadBit(); // Take Less Damage
		// Removed: Considered by Player
		state.buffer.ReadBit(); // Locked For Non Script Players
		state.buffer.ReadBit(); // Respect Locks When Has Driver
		state.buffer.ReadBit(); // Lock doors on cleanup
		state.buffer.ReadBit(); // Should Fix If No Collision
		state.buffer.ReadBit(); // Automatically attaches
		state.buffer.ReadBit(); // Scans with non-player driver
		state.buffer.ReadBit(); // Disable explode on contact

		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		state.buffer.ReadBit();
		if (Is2060()) // #Note: Ordering has not been verified.
		{
			state.buffer.ReadBit();
		}
		if (Is2944())
		{
			state.buffer.ReadBit();
		}
		if (Is3095())
		{
			state.buffer.ReadBit();
		}

		state.buffer.ReadBit(); // "Is Vehicle In Air"
		bool isParachuting = state.buffer.ReadBit();
		if (isParachuting)
		{
			state.buffer.ReadSignedFloat(8, 1.0f);
			state.buffer.ReadSignedFloat(8, 1.0f);
		}

		uint8_t popType = state.buffer.Read<uint8_t>(4);
		m_popType = static_cast<ePopType>(popType);

		// ...

		return true;
	}
};

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

struct CPhysicalAttachDataNode
{
	struct CPhysicalAttachNodeData : public CBaseAttachNodeData
	{
		bool hasParentOffset;
		float px, py, pz;

		bool hasAttachBones;
		uint16_t otherAttachBone;

		bool allowInitialSeparation;
		float unk_0x10c;
		float unk_0x110;
		bool isCargoVehicle;
		bool activatePhysicsWhenDetached;
	} data;

	bool Parse(SyncParseState& state)
	{
		data.attached = state.buffer.ReadBit();
		if (data.attached)
		{
			data.attachedTo = state.buffer.Read<uint16_t>(13);

			data.hasOffset = state.buffer.ReadBit();
			if (data.hasOffset) // Divisor 0x42200000
			{
				data.x = state.buffer.ReadSignedFloat(15, 40.f);
				data.y = state.buffer.ReadSignedFloat(15, 40.f);
				data.z = state.buffer.ReadSignedFloat(15, 40.f);
			}

			data.hasOrientation = state.buffer.ReadBit();
			if (data.hasOrientation) // Divisor 0x3F800000
			{
				data.qx = state.buffer.ReadSignedFloat(16, 40.f);
				data.qy = state.buffer.ReadSignedFloat(16, 40.f);
				data.qz = state.buffer.ReadSignedFloat(16, 40.f);
				data.qw = state.buffer.ReadSignedFloat(16, 40.f);
			}
			else
			{
				data.qw = 1.f; // Ensure identity quaternion is set.
				data.qx = data.qy = data.qz = 0.f;
			}

			data.hasParentOffset = state.buffer.ReadBit();
			if (data.hasParentOffset) // Divisor 0x40800000
			{
				data.px = state.buffer.ReadSignedFloat(15, 4.f);
				data.py = state.buffer.ReadSignedFloat(15, 4.f);
				data.pz = state.buffer.ReadSignedFloat(15, 4.f);
			}

			data.hasAttachBones = state.buffer.ReadBit();
			if (data.hasAttachBones)
			{
				data.otherAttachBone = state.buffer.Read<uint16_t>(8);
				data.attachBone = state.buffer.Read<uint16_t>(8);
			}
			else
			{
				data.attachBone = 0xFFFF;
			}

			data.attachmentFlags = state.buffer.Read<uint32_t>(19);
			data.allowInitialSeparation = state.buffer.ReadBit();
			data.unk_0x10c = state.buffer.ReadFloat(5, 1.f); // Divisor 0x3F800000
			data.unk_0x110 = state.buffer.ReadFloat(5, 1.f);
			data.isCargoVehicle = state.buffer.ReadBit();
		}
		else
		{
			data.activatePhysicsWhenDetached = false; // @TODO:
		}
		return true;
	}
};

struct CVehicleAppearanceDataNode : GenericSerializeDataNode<CVehicleAppearanceDataNode>
{
    CVehicleAppearanceNodeData data;

    template<typename Serializer>
    bool Serialize(Serializer& s)
    {
        s.Serialize(8, data.primaryColour);
        s.Serialize(8, data.secondaryColour);
        s.Serialize(8, data.pearlColour);
        s.Serialize(8, data.wheelColour);
        s.Serialize(8, data.interiorColour);
        s.Serialize(8, data.dashboardColour);

        s.Serialize(data.isPrimaryColourRGB);

        if (data.isPrimaryColourRGB)
        {
            s.Serialize(8, data.primaryRedColour);
            s.Serialize(8, data.primaryGreenColour);
            s.Serialize(8, data.primaryBlueColour);
        }

        s.Serialize(data.isSecondaryColourRGB);

        if (data.isSecondaryColourRGB)
        {
            s.Serialize(8, data.secondaryRedColour);
            s.Serialize(8, data.secondaryGreenColour);
            s.Serialize(8, data.secondaryBlueColour);
        }

        s.Serialize(8, data.envEffScale);

        s.Serialize(data.hasExtras);

        if (data.hasExtras)
        {
            s.Serialize(5, data.dirtLevel);
            s.Serialize(16, data.extras);

            s.Serialize(data.hasCustomLivery);

            if (data.hasCustomLivery)
            {
                s.Serialize(5, data.liveryIndex);
            }
            else
            {
                data.liveryIndex = 0;
            }

            s.Serialize(data.hasCustomRoofLivery);

            if (data.hasCustomRoofLivery)
            {
                s.Serialize(5, data.roofLiveryIndex);
            }
            else
            {
                data.roofLiveryIndex = 0;
            }
        }
        else
        {
            data.dirtLevel = 1;
            data.hasCustomLivery = false;
            data.hasCustomLiveryIndex = false;
            data.liveryIndex = -1;
            data.roofLiveryIndex = -1;
            data.extras = 0;
        }

        s.Serialize(2, data.kitIndex);

        if (data.kitIndex != 0)
        {
            for (int i = 0; i < 13; i++)
            {
                s.Serialize(data.hasMod);

                if (data.hasMod)
                {
                    s.Serialize(32, data.kitMods[i]);
                }
                else
                {
                    data.kitMods[i] = 0;
                }
            }

            s.Serialize(data.hasToggleMods);

            if (data.hasToggleMods)
            {
                s.Serialize(6, data.toggleMods);
            }
            else
            {
                data.toggleMods = 0;
            }

            s.Serialize(8, data.wheelChoice);
            s.Serialize(4, data.wheelType);

            s.Serialize(data.hasDifferentRearWheel);

            if (data.hasDifferentRearWheel)
            {
                s.Serialize(8, data.rearWheelChoice);
            }
            else
            {
                data.rearWheelChoice = 0;
            }

            s.Serialize(data.hasCustomTires);
            s.Serialize(data.hasWheelVariation1);
        }
        else
        {
            for (int i = 0; i < 13; i++)
            {
                data.kitMods[i] = 0;
            }
            data.toggleMods = 0;
            data.wheelChoice = 0;
            data.rearWheelChoice = 0;
            data.wheelType = 255;
            data.hasCustomTires = false;
            data.hasWheelVariation1 = false;
        }

        s.Serialize(data.hasWindowTint);

        if (data.hasWindowTint)
        {
            s.Serialize(8, data.windowTintIndex);
        }
        else
        {
            data.windowTintIndex = 0;
        }

        s.Serialize(data.hasTyreSmokeColours);

        if (data.hasTyreSmokeColours)
        {
            s.Serialize(8, data.tyreSmokeRedColour);
            s.Serialize(8, data.tyreSmokeGreenColour);
            s.Serialize(8, data.tyreSmokeBlueColour);
        }
        else
        {
            data.tyreSmokeRedColour = 255;
            data.tyreSmokeGreenColour = 255;
            data.tyreSmokeBlueColour = 255;
        }

        s.Serialize(data.hasPlate);

        for (int i = 0; i < 8; i++)
        {
            if (data.hasPlate)
            {
                s.Serialize(7, data.plate[i]);
            }
            else
            {
                data.plate[i] = ' ';
            }
        }

        s.Serialize(32, data.numberPlateTextIndex);
        s.Serialize(32, data.hornTypeHash);
        s.Serialize(data.hasEmblems);

        if (data.hasEmblems)
        {
            s.Serialize(data.isEmblem);

            if (!data.isEmblem)
            {
                s.Serialize(1, data.emblemType);
                s.Serialize(32, data.emblemId);
                s.Serialize(data.isSizeModified);
                s.Serialize(3, data.emblemSize);
            }
            else
            {
                s.Serialize(32, data.txdName);
                s.Serialize(32, data.textureName);
                data.emblemType = -1;
                data.emblemId = 0;
                data.emblemSize = 2;
            }

            for (int i = 0; i < 4; i++)
            {
                s.Serialize(data.hasBadge[i]);

                if (data.hasBadge[i])
                {
                    s.Serialize(10, data.badgeBoneIndex[i]);
                    s.Serialize(8, data.badgeAlpha[i]);

                    s.Serialize(14, data.badgeOffsetX[i]);
                    s.Serialize(14, data.badgeOffsetY[i]);
                    s.Serialize(10, data.badgeOffsetZ[i]);

                    s.Serialize(14, data.badgeDirX[i]);
                    s.Serialize(14, data.badgeDirY[i]);
                    s.Serialize(10, data.badgeDirZ[i]);

                    s.Serialize(14, data.badgeSideX[i]);
                    s.Serialize(14, data.badgeSideY[i]);
                    s.Serialize(10, data.badgeSideZ[i]);

                    s.Serialize(16, data.badgeSize[i]);
                }
            }
        }
        else
        {
            for (int i = 0; i < 13; i++)
            {
                data.hasBadge[i] = false;
            }
        }

        s.Serialize(data.hasNeonLights);

        if (data.hasNeonLights)
        {
            s.Serialize(8, data.neonRedColour);
            s.Serialize(8, data.neonGreenColour);
            s.Serialize(8, data.neonBlueColour);

            s.Serialize(data.neonLeftOn);
            s.Serialize(data.neonRightOn);
            s.Serialize(data.neonFrontOn);
            s.Serialize(data.neonBackOn);

            if (Is2372())
            {
                s.Serialize(data.isNeonSuppressed);
            }
        }

        return true;
    }
};

struct CVehicleDamageStatusDataNode
{
	CVehicleDamageStatusNodeData data;

	bool Parse(SyncParseState& state)
	{
		bool anyBodyDeformation = state.buffer.ReadBit();

		if (anyBodyDeformation)
		{
			uint8_t frontDamageLevel = state.buffer.Read<uint8_t>(2);
			uint8_t rearDamageLevel = state.buffer.Read<uint8_t>(2);
			uint8_t leftDamageLevel = state.buffer.Read<uint8_t>(2);
			uint8_t rightDamageLevel = state.buffer.Read<uint8_t>(2);
			uint8_t rearLeftLevel = state.buffer.Read<uint8_t>(2);
			uint8_t rearRightLevel = state.buffer.Read<uint8_t>(2);
		}

		data.damagedByBullets = state.buffer.ReadBit();

		if (data.damagedByBullets)
		{
			for (int i = 0; i < 6; i++)
			{
				uint8_t bulletsCount = state.buffer.Read<uint8_t>(8);
			}
		}

		bool anyBumperBroken = state.buffer.ReadBit();

		if (anyBumperBroken)
		{
			uint8_t frontBumperState = state.buffer.Read<uint8_t>(2);
			uint8_t rearBumperState = state.buffer.Read<uint8_t>(2);
		}

		bool anyLightBroken = state.buffer.ReadBit();

		if (anyLightBroken)
		{
			for (int i = 0; i < 22; i++)
			{
				bool lightBroken = state.buffer.ReadBit();
			}
		}

		data.anyWindowBroken = state.buffer.ReadBit();

		for (int i = 0; i < 8; i++)
		{
			data.windowsState[i] = (data.anyWindowBroken) ? state.buffer.ReadBit() : false;
		}

		bool unk = state.buffer.ReadBit();

		if (unk)
		{
			for (int i = 0; i < 8; i++)
			{
				float unk2 = state.buffer.ReadSignedFloat(10, 100.0f);

				if (unk2 != 100.0f)
				{
					int unk3 = state.buffer.Read<int>(8);
				}
			}
		}

		bool anySirenBroken = state.buffer.ReadBit();

		if (anySirenBroken)
		{
			for (int i = 0; i < 20; i++)
			{
				bool sirenBroken = state.buffer.ReadBit();
			}
		}

		return true;
	}
};

struct CVehicleComponentReservationDataNode { };

struct CVehicleHealthDataNode
{
	CVehicleHealthNodeData data;

	bool Parse(SyncParseState& state)
	{
		bool unk0 = state.buffer.ReadBit();
		bool unk1 = state.buffer.ReadBit();
		bool engineDamaged = state.buffer.ReadBit();
		bool petrolTankDamaged = state.buffer.ReadBit();

		if (engineDamaged)
		{
			auto engineHealth = state.buffer.ReadSigned<int>(19);
			data.engineHealth = engineHealth;
		}
		else
		{
			data.engineHealth = 1000;
		}

		if (petrolTankDamaged)
		{
			auto petrolTankHealth = state.buffer.ReadSigned<int>(19);
			data.petrolTankHealth = petrolTankHealth;
		}
		else
		{
			data.petrolTankHealth = 1000;
		}

		bool tyresFine = state.buffer.ReadBit();
		data.tyresFine = tyresFine;

		bool unk7 = state.buffer.ReadBit();

		if (!tyresFine || !unk7)
		{
			int totalWheels = state.buffer.Read<int>(4);

			if (!tyresFine)
			{
				for (int i = 0; i < totalWheels; i++)
				{
					// open wheel heat?
					if (Is2060())
					{
						if (state.buffer.ReadBit())
						{
							state.buffer.Read<int>(8);
						}
					}

					bool bursted = state.buffer.ReadBit();
					bool onRim = state.buffer.ReadBit();
					auto unk11 = state.buffer.ReadBit();
					auto unk12 = state.buffer.ReadBit();

					data.tyreStatus[i] = onRim ? 2 : (bursted ? 1 : 0);
				}
			}

			if (!unk7)
			{
				for (int i = 0; i < totalWheels; i++)
				{
					bool unk13 = state.buffer.ReadBit();

					if (unk13)
					{
						int unk14 = state.buffer.Read<int>(10); // Maximum 10000.0
					}
				}
			}
		}

		bool isFine = state.buffer.ReadBit();

		if (!isFine)
		{
			auto health = state.buffer.ReadSigned<int>(19);
			data.health = health;
		}
		else
		{
			data.health = 1000;
		}

		bool bodyHealthFine = state.buffer.ReadBit();

		if (!bodyHealthFine)
		{
			auto bodyHealth = state.buffer.ReadSigned<int>(19);
			data.bodyHealth = bodyHealth;
		}
		else
		{
			data.bodyHealth = 1000;
		}

		bool unk18 = state.buffer.ReadBit();

		if (unk18)
		{
			auto unk19 = state.buffer.Read<uint16_t>(13); // damage entity
			int lastDamageSource = state.buffer.Read<int>(32);
		}

		int unk21 = state.buffer.Read<int>(4);
		data.totalRepairs = state.buffer.Read<int>(4); // maximum 15
		auto unk23 = state.buffer.ReadBit();

		if (unk23)
		{
			int unk24 = state.buffer.Read<int>(64);
		}

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

	bool isRespawnObjectId;
	bool respawnFlaggedForRemoval;
	
	uint16_t randomSeed;

	uint32_t voiceHash;

	uint16_t vehicleId;
	int vehicleSeat;

	uint32_t prop;

	bool isStanding;
	int attributeDamageToPlayer;

	int maxHealth;
	bool unkBool;

	template<typename TSerializer>
	bool Serialize(TSerializer& s)
	{
		// false
		s.Serialize(isRespawnObjectId);

		// false
		s.Serialize(respawnFlaggedForRemoval);

		// 7(?)
		auto popType = (int)m_popType;
		s.Serialize(4, popType);
		m_popType = (ePopType)popType;

		// model
		s.Serialize(32, m_model);

		// 6841
		s.Serialize(16, randomSeed);

		// false
		bool inVehicle = vehicleId != 0;
		s.Serialize(inVehicle);

		// NO_VOICE -> 0x87BFF09A
		s.Serialize(32, voiceHash);

		if (inVehicle)
		{
			s.Serialize(13, vehicleId);
			s.Serialize(5, vehicleSeat);
		}
		else
		{
			vehicleId = 0;
			vehicleSeat = 0;
		}

		// false
		auto hasProp = prop != 0;
		s.Serialize(hasProp);

		if (hasProp)
		{
			s.Serialize(32, prop);
		}
		else
		{
			prop = 0;
		}

		// true
		s.Serialize(isStanding);

		// false
		auto hasAttDamageToPlayer = attributeDamageToPlayer >= 0;
		s.Serialize(hasAttDamageToPlayer);

		if (hasAttDamageToPlayer)
		{
			s.Serialize(5, attributeDamageToPlayer);
		}
		else
		{
			attributeDamageToPlayer = -1;
		}

		// 200
		s.Serialize(13, maxHealth);

		// false
		s.Serialize(unkBool);

		return true;
	}
};

struct CPedGameStateDataNode
{
	CPedGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		auto keepTasksAfterCleanup = state.buffer.ReadBit();
		auto bool1 = state.buffer.ReadBit();
		auto bool2 = state.buffer.ReadBit();
		auto bool3 = state.buffer.ReadBit();
		auto bool4 = state.buffer.ReadBit();
		auto bool5 = state.buffer.ReadBit();

		if (Is2060())
		{
			state.buffer.ReadBit();
			state.buffer.ReadBit();

			if (Is2189())
			{
				state.buffer.ReadBit();
			}

			if (Is2372())
			{
				state.buffer.ReadBit();
			}

			if (Is3407())
			{
				state.buffer.ReadBit();
				state.buffer.ReadBit();
			}
		}

		auto arrestState = state.buffer.Read<int>(1);
		auto deathState = state.buffer.Read<int>(2);

		auto hasWeapon = state.buffer.ReadBit();
		int weapon = 0;

		if (hasWeapon)
		{
			weapon = state.buffer.Read<int>(32);
			if (Is3258())
			{
				auto weaponState = state.buffer.Read<uint8_t>(3);
			}
		}

		data.curWeapon = weapon;

		if (Is2060())
		{
			state.buffer.ReadBit();
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
			if (Is2372())
			{
				auto v15 = state.buffer.ReadBit();
				if (v15)
				{
					auto unk192 = state.buffer.Read<uint8_t>(5);
				}
			}
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

			data.curVehicle = int32_t(vehicleId);
			data.curVehicleSeat = int32_t(-2);

			auto inSeat = state.buffer.ReadBit();

			if (inSeat)
			{
				vehicleSeat = state.buffer.Read<int>(5);
				data.curVehicleSeat = int32_t(vehicleSeat);
			}
			else
			{
				if (data.curVehicle != NULL && data.curVehicle != -1) {
					data.lastVehiclePedWasIn = data.curVehicle;
				}

				data.curVehicle = -1;
				data.curVehicleSeat = -1;
			}
		}
		else
		{
			if (data.curVehicle != NULL && data.curVehicle != -1) {
				data.lastVehiclePedWasIn = data.curVehicle;
			}

			data.curVehicle = -1;
			data.curVehicleSeat = -1;
		}

		bool bool6 = state.buffer.ReadBit();

		if (bool6)
		{
			bool bool7 = state.buffer.ReadBit();
		}

		bool hasCustodianOrArrestFlags = state.buffer.ReadBit();

		if (hasCustodianOrArrestFlags)
		{
			uint16_t custodianId = state.buffer.Read<uint16_t>(13);
			bool isHandcuffed = state.buffer.ReadBit();
			bool canPerformArrest = state.buffer.ReadBit();
			bool canPerformUncuff = state.buffer.ReadBit();
			bool canBeArrested = state.buffer.ReadBit();
			bool isInCustody = state.buffer.ReadBit();

			data.isHandcuffed = isHandcuffed;
		}
		else
		{
			data.isHandcuffed = false;
		}

		bool isFlashLightOn = state.buffer.ReadBit();
		bool actionModeEnabled = state.buffer.ReadBit();
		bool stealthModeEnabled = state.buffer.ReadBit();

		if (actionModeEnabled || stealthModeEnabled)
		{
			uint32_t actionModeOverride = state.buffer.Read<uint32_t>(32);
		}

		bool killedByStealth = state.buffer.ReadBit();
		bool killedByTakedown = state.buffer.ReadBit();

		data.actionModeEnabled = actionModeEnabled;
		data.isFlashlightOn = isFlashLightOn;

		// TODO

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

struct CObjectOrientationDataNode : GenericSerializeDataNode<CObjectOrientationDataNode>
{
	CObjectOrientationNodeData data;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
		s.Serialize(data.highRes);

		if (data.highRes)
		{
			const float divisor = glm::pi<float>() * 4;

			s.SerializeSigned(20, divisor, data.rotX);
			s.SerializeSigned(20, divisor, data.rotY);
			s.SerializeSigned(20, divisor, data.rotZ);
		}
		else
		{
			s.Serialize(2, data.quat.largest);
			s.Serialize(11, data.quat.integer_a);
			s.Serialize(11, data.quat.integer_b);
			s.Serialize(11, data.quat.integer_c);
		}

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

struct CVehicleGadgetDataNode
{
	bool Parse(SyncParseState& state) { return true; }

	// Avoid unnecessary CPU cycles parsing something unused.
#if 0
	enum class VehicleGadgetType : uint8_t
	{
		CVehicleGadgetForks = 0,
		CSearchLight = 1,
		CVehicleGadgetPickUpRopeWithHook = 2,
		CVehicleGadgetDiggerArm = 3,
		CVehicleGadgetHandlerFrame = 4,
		CVehicleGadgetPickUpRopeWithMagnet = 5,
		CVehicleGadgetBombBay = 6,
		Unknown = 255,
	};

	bool hasParentVehicleOffset;
	float parentVehicleOffsetX;
	float parentVehicleOffsetY;
	float parentVehicleOffsetZ;

	uint8_t count;
	struct {
		VehicleGadgetType type;
		union
		{
			struct { float desiredPosition; float desiredAcceleration; } fork;
			struct { bool searchlightOn; uint16_t searchlightTarget; } searchlight;
			struct { float offsetX, offsetY, offsetZ; uint16_t object; } pickupRope;
			struct { float m_fJointPositionRatio; } diggerArm;
			struct { float desiredPosition; float desiredAcceleration; uint16_t attachEntity; } handlerFrame;
			struct { bool unk_0x45; } bombBay;
			struct
			{
				float offsetX, offsetY, offsetZ;
				uint16_t object;

				uint16_t magnetId;;
				uint16_t attractedId;
				bool unk_0x161;

				bool unk_0x014;
				float unk_0x144, unk_0x148, unk_0x14c;
				float unk_0x150, unk_0x154, unk_0x158;
				float unk_0x15c;
				bool unk_0x140;
			} pickupMagnet;
		};

		bool Parse(VehicleGadgetType g_type, rl::MessageBuffer& buffer)
		{
			type = g_type;
			switch (type)
			{
				case VehicleGadgetType::CVehicleGadgetForks:
				{
					fork.desiredPosition = buffer.ReadFloat(8, 1.7);
					fork.desiredAcceleration = buffer.ReadSignedFloat(8, 1.0);
					break;
				}
				case VehicleGadgetType::CSearchLight:
				{
					searchlight.searchlightOn = buffer.ReadBit();
					searchlight.searchlightTarget = buffer.Read<uint16_t>(13);
					break;
				}
				case VehicleGadgetType::CVehicleGadgetPickUpRopeWithHook:
				{
					pickupRope.offsetX = buffer.ReadFloat(8, 100.f); // Divisor 0x42C80000
					pickupRope.offsetY = buffer.ReadFloat(8, 100.f);
					pickupRope.offsetZ = buffer.ReadFloat(8, 100.f);
					pickupRope.object = buffer.Read<uint16_t>(13);
					break;
				}
				case VehicleGadgetType::CVehicleGadgetDiggerArm:
				{
					diggerArm.m_fJointPositionRatio = buffer.ReadFloat(8, 1.7);
					break;
				}
				case VehicleGadgetType::CVehicleGadgetHandlerFrame:
				{
					handlerFrame.desiredPosition = buffer.ReadFloat(8, 5.5999999f); // Divisor 0x40B33333
					handlerFrame.desiredAcceleration = buffer.ReadSignedFloat(8, 1.f);
					handlerFrame.attachEntity = buffer.Read<uint16_t>(13);
					break;
				}
				case VehicleGadgetType::CVehicleGadgetBombBay:
				{
					bombBay.unk_0x45 = buffer.ReadBit();
					break;
				}
				case VehicleGadgetType::CVehicleGadgetPickUpRopeWithMagnet:
				{
					pickupMagnet.offsetX = buffer.ReadFloat(8, 100.f);
					pickupMagnet.offsetY = buffer.ReadFloat(8, 100.f);
					pickupMagnet.offsetZ = buffer.ReadFloat(8, 100.f);
					pickupMagnet.object = buffer.Read<uint16_t>(13);

					pickupMagnet.magnetId = buffer.Read<uint16_t>(13);
					pickupMagnet.attractedId = buffer.Read<uint16_t>(13);

					pickupMagnet.unk_0x161 = buffer.ReadBit();

					pickupMagnet.unk_0x014 = buffer.ReadBit();
					pickupMagnet.unk_0x144 = buffer.ReadSignedFloat(4, 10.f); // Divisor 0x41200000
					pickupMagnet.unk_0x148 = buffer.ReadSignedFloat(4, 10.f);
					pickupMagnet.unk_0x14c = buffer.ReadSignedFloat(4, 10.f);
					pickupMagnet.unk_0x150 = buffer.ReadSignedFloat(4, 10.f);
					pickupMagnet.unk_0x154 = buffer.ReadSignedFloat(4, 10.f);
					pickupMagnet.unk_0x158 = buffer.ReadSignedFloat(4, 10.f);
					pickupMagnet.unk_0x15c = buffer.ReadSignedFloat(4, 100.f); // Divisor 0x42C80000

					pickupMagnet.unk_0x140 = buffer.ReadBit();
				}
				default:
				{
					type = VehicleGadgetType::Unknown;
					break;
				}
			}
			return true;
		}
	} gadgets[4];

	bool Parse(SyncParseState& state)
	{
		hasParentVehicleOffset = state.buffer.ReadBit();
		if (hasParentVehicleOffset)
		{
			parentVehicleOffsetX = state.buffer.ReadSignedFloat(14, 24.f); // Divisor = 0x41C00000
			parentVehicleOffsetY = state.buffer.ReadSignedFloat(14, 24.f);
			parentVehicleOffsetZ = state.buffer.ReadSignedFloat(14, 24.f);
		}

		count = state.buffer.Read<uint8_t>(2);
		for (uint8_t i = 0; i < count; ++i)
		{
			uint8_t block[12] = { 0 };

			VehicleGadgetType type = (VehicleGadgetType)state.buffer.Read<uint8_t>(3);
			if (i == (count - 1))
			{
				gadgets[i].Parse(type, state.buffer);
			}
			// SerialiseDataBlock
			else if (state.buffer.ReadBits(block, 94))
			{
				rl::MessageBuffer buffer(block, sizeof(block));
				gadgets[i].Parse(type, state.buffer);
			}
			else
			{
				count = 0;
				return true;
			}
		}
		return true;
	}
#endif
};

struct CMigrationDataNode { };
struct CPhysicalMigrationDataNode { };
struct CPhysicalScriptMigrationDataNode { };

struct CVehicleProximityMigrationDataNode
{
// The vehicles population type can change on migrate. This is not something
// supported by FXServer at the moment, see GH-2097.
#if 0
	ePopType m_popType;

	bool Parse(SyncParseState& state)
	{
		uint32_t maxOccupants = state.buffer.Read<uint32_t>(5);
		if (maxOccupants > 16)
		{
			maxOccupants = 16;
		}

		for (uint32_t i = 0; i < maxOccupants; ++i)
		{
			bool hasOccupant = state.buffer.ReadBit();
			if (hasOccupant)
			{
				state.buffer.Read<uint16_t>(13); // Occupant
			}
		}

		bool hasPopType = state.buffer.ReadBit();
		if (hasPopType)
		{
			uint8_t popType = state.buffer.Read<uint8_t>(4);
			m_popType = static_cast<ePopType>(popType);
		}
		else
		{
			m_popType = ePopType::POPTYPE_UNKNOWN;
		}

		// ...

		return true;
	}
#endif
};

struct CBikeGameStateDataNode { };

struct CBoatGameStateDataNode
{
	CBoatGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		bool lockedToXY = state.buffer.ReadBit();
		int boatWreckedAction = state.buffer.Read<int>(2);
		bool forcedBoatLocationWhenAnchored = state.buffer.ReadBit();

		bool unk = state.buffer.ReadBit();
		bool interiorLightEnabled = state.buffer.ReadBit();

		float sinkEndTime = state.buffer.ReadSignedFloat(14, 1.0f);
		bool movementResistant = state.buffer.ReadBit(); // movementResistance >= 0.0

		if (movementResistant)
		{
			bool unk3 = state.buffer.ReadBit(); // movementResistance > 1000.0

			if (!unk3)
			{
				float movementResistance = state.buffer.ReadSignedFloat(16, 1000.0f);
			}
		}

		data.lockedToXY = lockedToXY;
		data.sinkEndTime = sinkEndTime;
		data.wreckedAction = boatWreckedAction;

		return true;
	}
};

struct CDoorCreationDataNode
{
	float m_posX;
	float m_posY;
	float m_posZ;
	float m_unkFloat;
	uint32_t m_unkHash;

	bool Parse(SyncParseState& state)
	{
		auto modelHash = state.buffer.Read<uint32_t>(32);

		float positionX = state.buffer.ReadSignedFloat(19, 27648.0f);
		float positionY = state.buffer.ReadSignedFloat(19, 27648.0f);
		float positionZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;

		m_posX = positionX;
		m_posY = positionY;
		m_posZ = positionZ;

		bool scriptDoor = state.buffer.ReadBit();

		if (Is3258())
		{
			bool unkBool = state.buffer.ReadBit();
			if (unkBool)
			{
				m_unkFloat = state.buffer.ReadFloat(8, 3.1415927f);
				m_unkHash = state.buffer.Read<uint32_t>(32);
			}
			else
			{
				m_unkFloat = 0.f;
				m_unkHash = 0;
			}
		}

		if (!scriptDoor)
		{
			bool playerWantsControl = state.buffer.ReadBit();
		}

		return true;
	}
};

struct CDoorMovementDataNode
{
	CDoorMovementDataNodeData data;

	bool Parse(SyncParseState& state)
	{
		// Can be changed by N_0xa85a21582451e951. Guessed name
		data.isManualDoor = state.buffer.ReadBit();
		if (data.isManualDoor)
		{
			data.openRatio = state.buffer.ReadSignedFloat(8, 1.0f);
		}
		else
		{
			data.opening = state.buffer.ReadBit();
			// Not accurate for all gates. Only checks '.m128_f32[0] > 0.99000001', some may be -1.0 when open
			data.fullyOpen = state.buffer.ReadBit();
			data.closed = state.buffer.ReadBit();
		}

		return true;
	}
};

struct CDoorScriptInfoDataNode
{
	CDoorScriptInfoDataNodeData data;

	bool Parse(SyncParseState& state)
	{
		bool hasScript = state.buffer.ReadBit();
		if (hasScript)
		{
			data.scriptHash = state.buffer.Read<uint32_t>(32);
			uint32_t timestamp = state.buffer.Read<uint32_t>(32);

			if (state.buffer.ReadBit())
			{
				uint32_t positionHash = state.buffer.Read<uint32_t>(32);
			}

			if (state.buffer.ReadBit())
			{
				uint32_t instanceId = state.buffer.Read<uint32_t>(7);
			}

			uint32_t scriptObjectId = state.buffer.Read<uint32_t>(32);

			int hostTokenLength = state.buffer.ReadBit() ? 16 : 3;
			uint32_t hostToken = state.buffer.Read<uint32_t>(hostTokenLength);

			data.doorSystemHash = state.buffer.Read<uint32_t>(32);
			bool existingDoorSystemEntry = state.buffer.ReadBit();
		}

		return true;
	}
};

struct CDoorScriptGameStateDataNode
{
	CDoorScriptGameStateDataNodeData data;

	bool Parse(SyncParseState& state)
	{
		data.doorSystemState = state.buffer.Read<uint32_t>(3);

		bool hasAutomaticInfo = state.buffer.ReadBit();
		if (hasAutomaticInfo)
		{
			float automaticDistance = state.buffer.ReadSignedFloat(9, 100.0f);
			float slideRate = state.buffer.ReadSignedFloat(9, 30.0f);
		}

		bool hasBrokenFlags = state.buffer.ReadBit();
		if (hasBrokenFlags)
		{
			int brokenFlags = state.buffer.Read<int>(18);
		}

		bool hasDamagedFlags = state.buffer.ReadBit();
		if (hasDamagedFlags)
		{
			int damagedFlags = state.buffer.Read<int>(18);
		}

		data.holdOpen = state.buffer.ReadBit();

		return true;
	}
};

struct CHeliHealthDataNode : GenericSerializeDataNode<CHeliHealthDataNode>
{
	CHeliHealthNodeData data;

	template<typename Serializer>
    bool Serialize(Serializer& s)
	{
		s.Serialize(17, data.mainRotorHealth);
		s.Serialize(17, data.rearRotorHealth);

		s.Serialize(data.boomBroken);
		s.Serialize(data.canBoomBreak);
		s.Serialize(data.hasCustomHealth);

		if (data.hasCustomHealth)
		{
			s.Serialize(17, data.bodyHealth);
			s.Serialize(17, data.gasTankHealth);
			s.Serialize(17, data.engineHealth);
		}

		s.SerializeSigned(11, 100.0f, data.mainRotorDamage);
		s.SerializeSigned(11, 100.0f, data.rearRotorDamage);
		s.SerializeSigned(11, 100.0f, data.tailRotorDamage);

		s.Serialize(data.disableExplosionFromBodyDamage);

		return true;
	}
};

struct CHeliControlDataNode : GenericSerializeDataNode<CHeliControlDataNode>
{
    CHeliControlDataNodeData data;

    template<typename Serializer>
    bool Serialize(Serializer& s)
    {
        s.SerializeSigned(8, 1.0f, data.yawControl);
        s.SerializeSigned(8, 1.0f, data.pitchControl);
        s.SerializeSigned(8, 1.0f, data.rollControl);
        s.Serialize(8, 2.0f, data.throttleControl);

        s.Serialize(data.engineOff);

        s.Serialize(data.hasLandingGear);
        if (data.hasLandingGear)
        {
            s.Serialize(3, data.landingGearState);
        }

        s.Serialize(data.isThrusterModel);
        if (data.isThrusterModel)
        {
            s.SerializeSigned(9, 1.0f, data.thrusterSideRCSThrottle);
            s.SerializeSigned(9, 1.0f, data.thrusterThrottle);
        }

        s.Serialize(data.hasVehicleTask);
        s.Serialize(data.lockedToXY);

        return true;
    }
};

struct CObjectCreationDataNode
{
	int m_createdBy;
	uint32_t m_model;
	bool m_hasInitPhysics;
	CDummyObjectCreationNodeData dummy;

	// #TODO: universal serializer
	bool Unparse(SyncUnparseState& state)
	{
		state.buffer.Write<int>(5, 4); // ENTITY_OWNEDBY_SCRIPT
		state.buffer.Write<uint32_t>(32, m_model);
		state.buffer.WriteBit(m_hasInitPhysics);
		state.buffer.WriteBit(false);
		state.buffer.WriteBit(false);

		state.buffer.WriteBit(false);
		state.buffer.WriteBit(false);
		state.buffer.WriteBit(false);

		if (Is2944())
		{
			state.buffer.WriteBit(false);
		}

		return true;
	}

	bool Parse(SyncParseState& state)
	{
		/*
			Probably a subsystem ID
			If it's 0 or 2, it's a dummy object

			Enum from X360:
			0: ENTITY_OWNEDBY_RANDOM
			1: ENTITY_OWNEDBY_TEMP
			2: ENTITY_OWNEDBY_FRAGMENT_CACHE
			3: ENTITY_OWNEDBY_GAME
			4: ENTITY_OWNEDBY_SCRIPT
			5: ENTITY_OWNEDBY_AUDIO
			6: ENTITY_OWNEDBY_CUTSCENE
			7: ENTITY_OWNEDBY_DEBUG
			8: ENTITY_OWNEDBY_OTHER
			9: ENTITY_OWNEDBY_PROCEDURAL
			10: ENTITY_OWNEDBY_POPULATION
			11: ENTITY_OWNEDBY_STATICBOUNDS
			12: ENTITY_OWNEDBY_PHYSICS
			13: ENTITY_OWNEDBY_IPL
			14: ENTITY_OWNEDBY_VFX
			15: ENTITY_OWNEDBY_NAVMESHEXPORTER
			16: ENTITY_OWNEDBY_INTERIOR
			17: ENTITY_OWNEDBY_COMPENTITY
		*/
		m_createdBy = state.buffer.Read<int>(5);
		if (m_createdBy != 0 && m_createdBy != 2)
		{
			m_model = state.buffer.Read<uint32_t>(32);

			m_hasInitPhysics = state.buffer.ReadBit();
			bool scriptGrabbedFromWorld = state.buffer.ReadBit();
			bool noReassign = state.buffer.ReadBit();

			if (scriptGrabbedFromWorld)
			{
				float scriptGrabPosX = state.buffer.ReadSignedFloat(19, 27648.0f);
				float scriptGrabPosY = state.buffer.ReadSignedFloat(19, 27648.0f);
				float scriptGrabPosZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;

				auto scriptGrabRadius = state.buffer.ReadFloat(8, 20.f); // 0x41A00000
			}
		}
		else
		{
			dummy.dummyPosX = state.buffer.ReadSignedFloat(31, 27648.0f);
			dummy.dummyPosY = state.buffer.ReadSignedFloat(31, 27648.0f);
			dummy.dummyPosZ = state.buffer.ReadFloat(31, 4416.0f) - 1700.0f;

			dummy.playerWantsControl = state.buffer.ReadBit();
			dummy.hasFragGroup = state.buffer.ReadBit();
			dummy.isBroken = state.buffer.ReadBit();
			dummy.unk11 = state.buffer.ReadBit();
			dummy.hasExploded = state.buffer.ReadBit();
			dummy._explodingEntityExploded = state.buffer.ReadBit();
			dummy.keepRegistered = state.buffer.ReadBit();

			if (dummy.hasFragGroup)
			{
				dummy.fragGroupIndex = state.buffer.Read<int>(5);
			}

			dummy._hasRelatedDummy = state.buffer.ReadBit();

			if (!dummy._hasRelatedDummy)
			{
				auto ownershipToken = state.buffer.Read<int>(10);
				float objectPosX = state.buffer.ReadSignedFloat(19, 27648.0f);
				float objectPosY = state.buffer.ReadSignedFloat(19, 27648.0f);
				float objectPosZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;
				
				auto objectRotX = state.buffer.ReadSigned<int>(9) * 0.015625f;
				auto objectRotY = state.buffer.ReadSigned<int>(9) * 0.015625f;
				auto objectRotZ = state.buffer.ReadSigned<int>(9) * 0.015625f;
			}
		}

		bool unk20 = state.buffer.ReadBit();

		if (unk20)
		{
			auto unk21 = state.buffer.Read<uint16_t>(13);
		}

		bool unk22 = state.buffer.ReadBit();

		if (unk22)
		{
			auto unk23 = state.buffer.Read<int>(16);
		}

		bool unk24 = state.buffer.ReadBit();

		if (Is2944())
		{
			bool unk25 = state.buffer.ReadBit();

			if (unk25)
			{
				auto unk26 = state.buffer.Read<int>(32);
				auto unk27 = state.buffer.Read<int>(32);
				auto unk28 = state.buffer.Read<int>(32);
				auto unk29 = state.buffer.ReadBit();
			}
		}

		return true;
	}
};

struct CObjectGameStateDataNode
{
	CObjectGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		data.hasTask = state.buffer.ReadBit();
		if (data.hasTask)
		{
			data.taskType = state.buffer.Read<uint16_t>(10);
			data.taskDataSize = state.buffer.Read<uint16_t>(8);
		}
		else
		{
			data.taskDataSize = 0;
		}

		// Bypass SerialiseDataBlock.
		state.buffer.SetCurrentBit(state.buffer.GetCurrentBit() + data.taskDataSize);

		data.isBroken = state.buffer.ReadBit();
		if (data.isBroken)
		{
			data.brokenFlags = state.buffer.Read<uint32_t>(32);
		}

		if (Is2060()) // Introduced 1868, see sub_141147D48 (2372)
		{
			auto v11 = state.buffer.ReadBit();
			if (v11)
			{
				uint8_t f_10 = state.buffer.Read<uint32_t>(8); // max 128
				for (uint8_t i = 0; i < (f_10 / 8); ++i)
				{
					state.buffer.Read<uint32_t>(8);
				}
			}
		}

		data.hasExploded = state.buffer.ReadBit();
		data.hasAddedPhysics = state.buffer.ReadBit();
		data.isVisible = state.buffer.ReadBit();
		data.unk_0x165 = state.buffer.ReadBit();
		data.unk_0x166 = state.buffer.ReadBit();

		return true;
	}
};

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

struct CPedScriptGameStateDataNode
{
	ePopType m_popType;

	bool Parse(SyncParseState& state)
	{
		uint8_t popType = state.buffer.Read<uint8_t>(4);
		m_popType = static_cast<ePopType>(popType);

		// ...

		return true;
	}
};

struct CPedAttachDataNode : GenericSerializeDataNode<CPedAttachDataNode>
{
	struct CPedAttachNodeData : public CBaseAttachNodeData
	{
		bool attachedToGround;       // unk_0x241
		bool hasHeading;
		float heading;               // heading_1
		float headingLimit;          // heading_2
	} data;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
		s.Serialize(data.attached);
		if (data.attached)
		{
			s.Serialize(13, data.attachedTo);
			s.Serialize(data.attachedToGround);

			s.Serialize(data.hasOffset);
			if (data.hasOffset) // Divisor 0x42340000
			{
				s.SerializeSigned(15, 45.f, data.x);
				s.SerializeSigned(15, 45.f, data.y);
				s.SerializeSigned(15, 45.f, data.z);
			}
			else
			{
				data.x = data.y = data.z = 0;
			}

			s.Serialize(data.hasOrientation);
			if (data.hasOrientation) // Divisor 0x3F8147AE
			{
				s.SerializeSigned(16, 1.01f, data.qx);
				s.SerializeSigned(16, 1.01f, data.qy);
				s.SerializeSigned(16, 1.01f, data.qz);
				s.SerializeSigned(16, 1.01f, data.qw);
			}
			else
			{
				data.qw = 1.f; // Ensure identity quaternion is set.
				data.qx = data.qy = data.qz = 0.f;
			}

			s.Serialize(8, data.attachBone);
			s.Serialize(17, data.attachmentFlags);
			s.Serialize(data.hasHeading);
			if (data.hasHeading) // Divisor 0x40C90FDB
			{
				s.SerializeSigned(8, 6.28319f, data.heading);
				s.SerializeSigned(8, 6.28319f, data.headingLimit);
			}
		}
		return true;
	}
};

struct CPedHealthDataNode
{
	CPedHealthNodeData data;

	bool Parse(SyncParseState& state)
	{
		bool isFine = state.buffer.ReadBit();
		auto maxHealthChanged = state.buffer.ReadBit();

		int maxHealth = (data.maxHealth == 0) ? 200 : data.maxHealth;

		if (maxHealthChanged)
		{
			maxHealth = state.buffer.Read<int>(13);
		}

		data.maxHealth = maxHealth;

		if (!isFine)
		{
			int pedHealth = state.buffer.Read<int>(13);
			auto killedWithHeadshot = state.buffer.ReadBit();
			auto killedWithMelee = state.buffer.ReadBit();

			data.health = pedHealth;
		}
		else
		{
			data.health = maxHealth;
		}

		bool noArmour = state.buffer.ReadBit();

		if (!noArmour)
		{
			int pedArmour = state.buffer.Read<int>(13);
			data.armour = pedArmour;
		}
		else
		{
			data.armour = 0;
		}

		if (Is2060())
		{
			bool hasUnk1 = state.buffer.ReadBit();
			bool hasUnk2 = state.buffer.ReadBit();

			if (hasUnk2)
			{
				state.buffer.Read<int>(13);
			}

			if (!hasUnk1)
			{
				state.buffer.Read<int>(13);
			}
		}


		auto hasSource = state.buffer.ReadBit();

		if (hasSource)
		{
			int damageEntity = state.buffer.Read<int>(13);
			data.sourceOfDamage = damageEntity;
		}
		else 
		{
			data.sourceOfDamage = 0;
		}

		int causeOfDeath = state.buffer.Read<int>(32);
		data.causeOfDeath = causeOfDeath;

		auto hurtStarted = state.buffer.ReadBit();

		int hurtEndTime = state.buffer.Read<int>(2);

		auto hasWeaponDamageComponent = state.buffer.ReadBit();

		if (hasWeaponDamageComponent)
		{
			int weaponDamageComponent = state.buffer.Read<int>(8);
		}

		return true;
	}
};

struct CPedMovementGroupDataNode
{
	CPedMovementGroupNodeData data;

	bool Parse(SyncParseState& state)
	{
		uint32_t motionGroup = state.buffer.Read<uint32_t>(32);
		auto defaultActionMode = state.buffer.ReadBit();

		if (!defaultActionMode)
		{
			uint8_t moveBlendType = state.buffer.Read<uint8_t>(3);
			int moveBlendState = state.buffer.Read<int>(5);

			auto overiddenWeaponGroup = state.buffer.Read<uint32_t>(32);
			auto isCrouching = state.buffer.ReadBit();

			data.isStealthy = state.buffer.ReadBit();
			data.isStrafing = state.buffer.ReadBit();
			data.isRagdolling = state.buffer.ReadBit();

			auto isRagdollConstraintAnkleActive = state.buffer.ReadBit();
			auto isRagdollConstraintWristActive = state.buffer.ReadBit();
		}
		else
		{
			data.isStealthy = false;
			data.isStrafing = false;
			data.isRagdolling = false;
		}

		return true;
	}
};

struct CPedAIDataNode : GenericSerializeDataNode<CPedAIDataNode>
{
	CPedAINodeData data;

	template<typename Serializer>
	bool Serialize(Serializer& s)
	{
		s.Serialize(32, data.relationShip);
		s.Serialize(32, data.decisionMaker);
		return true;
	}
};

struct CPedAppearanceDataNode
{
	bool Parse(SyncParseState& state)
	{
#if 0
		DeserializePedProps(state);

		DeserializePedComponents(state);

		int phoneMode = state.buffer.Read<int>(2);

		bool isTakingOffVehicleHelmet = state.buffer.ReadBit();
		bool isPuttingOnVehicleHelmet = state.buffer.ReadBit();
		bool hasVehicleHelmet = state.buffer.ReadBit();
		bool isHelmetVisorUp = state.buffer.ReadBit();
		bool hasHelmetVisorPropIndices = state.buffer.ReadBit();
		bool isSwitchingHelmetVisor = state.buffer.ReadBit();

		int unk_0x1E1 = state.buffer.Read<int>(8); // 0x98 of CTaskTakeOffHelmet (b2545)

		if (isPuttingOnVehicleHelmet || hasVehicleHelmet || isSwitchingHelmetVisor)
		{
			int vehicleHelmetTextureId = state.buffer.Read<int>(8);
			int vehicleHelmetPropId = state.buffer.Read<int>(10);
			int helmetVisorDownPropId = state.buffer.Read<int>(10);
			int helmetVisorUpPropId = state.buffer.Read<int>(10);
		}

		bool hasParachuteTintIndex = state.buffer.ReadBit();
		if (hasParachuteTintIndex)
		{
			int parachuteTintIndex = state.buffer.Read<int>(4);
			int parachutePackTintIndex = state.buffer.Read<int>(4);
		}

		bool hasFacialClipset = state.buffer.ReadBit();
		if (hasFacialClipset)
		{
			uint32_t facialClipsetName = state.buffer.Read<uint32_t>(32);
			uint32_t facialClipsetDict = state.buffer.Read<uint32_t>(32);
		}

		bool hasFacialIdleAnim = state.buffer.ReadBit();
		if (hasFacialIdleAnim)
		{
			uint32_t facialIdleAnim = state.buffer.Read<uint32_t>(32);
		}

		bool hasFacialIdleAnimDict = state.buffer.ReadBit();
		if (hasFacialIdleAnimDict)
		{
			uint32_t facialIdleAnimDict = state.buffer.Read<uint32_t>(32);
		}
#endif

		return true;
	}

	static void DeserializePedProps(SyncParseState& state)
	{
		for (int i = 0; i < 6; i++)
		{
			uint32_t dlcName = state.buffer.Read<int>(32);
			int packedProp = state.buffer.Read<int>(19);
		}
	}

	static void DeserializePedComponents(SyncParseState& state)
	{
		bool hasComponents = state.buffer.ReadBit();
		if (hasComponents)
		{
			int componentBitfield = state.buffer.Read<int>(12);
			bool useLargeReads = state.buffer.ReadBit();

			for (int i = 0; i < 12; i++)
			{
				if ((componentBitfield >> i) & 1)
				{
					int drawableId = state.buffer.Read<int>(useLargeReads ? 7 : 4);

					bool hasTexture = state.buffer.ReadBit();
					if (hasTexture)
					{
						int textureId = state.buffer.Read<int>(5);
					}

					bool hasPalette = state.buffer.ReadBit();
					if (hasPalette)
					{
						int paletteId = state.buffer.Read<int>(4);
					}

					bool isDlcItem = state.buffer.ReadBit();
					if (isDlcItem)
					{
						uint32_t dlcName = state.buffer.Read<uint32_t>(32);
					}
				}
			}
		}
	}
};

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

struct CPedTaskTreeDataNode
{
	CPedTaskTreeDataNodeData data;

	bool Parse(SyncParseState& state)
	{
		bool hasScriptTask = state.buffer.ReadBit();
		if (hasScriptTask)
		{
			data.scriptCommand = state.buffer.Read<uint32_t>(32);
			data.scriptTaskStage = state.buffer.Read<uint32_t>(3);
		}
		else
		{
			data.scriptCommand = 0x811E343C;
			data.scriptTaskStage = 3;
		}

		data.specifics = state.buffer.Read<int>(8);
		for (int i = 0; i < 8; i++)
		{
			auto& task = data.tasks[i];

			if ((data.specifics >> i) & 1)
			{
				task.type = state.buffer.Read<uint32_t>(10);
				task.active = state.buffer.ReadBit();
				task.priority = state.buffer.Read<uint32_t>(3);
				task.treeDepth = state.buffer.Read<uint32_t>(3);
				task.sequenceId = state.buffer.Read<uint32_t>(5);
			}
			else
			{
				task.type = Is2060() ? 531 : 530;
			}
		}

		return true;
	}
};

struct CPedTaskSpecificDataNode { };

struct CPedSectorPosMapNode : GenericSerializeDataNode<CPedSectorPosMapNode>
{
	float m_posX;
	float m_posY;
	float m_posZ;

	bool isStandingOn;
	bool isNM;

	uint16_t standingOn;
	float standingOnOffset[3];

	template<typename TSerializer>
	bool Serialize(TSerializer& s)
	{
		s.Serialize(12, 54.0f, m_posX);
		s.Serialize(12, 54.0f, m_posY);
		s.Serialize(12, 69.0f, m_posZ);

		if constexpr (TSerializer::isReader)
		{
			s.state->entity->syncTree->CalculatePosition();
		}

		bool hasExtraData = (isStandingOn || isNM);
		s.Serialize(hasExtraData);

		if (hasExtraData)
		{
			s.Serialize(isNM);
			s.Serialize(isStandingOn);

			if (isStandingOn)
			{
				s.Serialize(13, standingOn);
				s.SerializeSigned(12, 16.0f, standingOnOffset[0]); // Standing On Local Offset X
				s.SerializeSigned(12, 16.0f, standingOnOffset[1]); // Standing On Local Offset Y
				s.SerializeSigned(10, 4.0f, standingOnOffset[2]); // Standing On Local Offset Z
			}
		}

		return true;
	}
};

struct CPedSectorPosNavMeshNode { };
struct CPedInventoryDataNode { };
struct CPedTaskSequenceDataNode { };
struct CPickupCreationDataNode { };
struct CPickupScriptGameStateNode { };
struct CPickupSectorPosNode { };

struct CPickupPlacementCreationDataNode
{
	float posX = 0.0f;
	float posY = 0.0f;
	float posZ = 0.0f;

	bool Parse(SyncParseState& state)
	{
		// omit flag
		if (!state.buffer.ReadBit())
		{
			// Pickup pos
			posX = state.buffer.ReadSignedFloat(19, 27648.0f);
			posY = state.buffer.ReadSignedFloat(19, 27648.0f);
			posZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;

			// TODO: read more node including fwScriptId
		}

		return true;
	}
};

struct CPickupPlacementStateDataNode { };

struct CPlaneGameStateDataNode
{
	CPlaneGameStateDataNodeData data;

	bool Parse(SyncParseState& state)
	{
		data.landingGearState = state.buffer.Read<uint32_t>(3);

		bool hasDamagedSections = state.buffer.ReadBit();
		bool hasBrokenSections = state.buffer.ReadBit();
		bool hasBrokenRotors = state.buffer.ReadBit();
		bool hasRotors = state.buffer.ReadBit();

		float unk6 = state.buffer.ReadSignedFloat(7, 1.0f);

		bool unk7 = state.buffer.ReadBit();
		if (unk7)
		{
			float unk8[13];
			for (int i = 0; i < 13; i++)
			{
				unk8[i] = state.buffer.ReadSignedFloat(7, 1.0f);
			}
		}

		bool unk9 = state.buffer.ReadBit();
		if (unk9)
		{
			float unk10[13];
			for (int i = 0; i < 13; i++)
			{
				unk10[i] = state.buffer.ReadSignedFloat(7, 1.0f);
			}
		}

		if (hasDamagedSections)
		{
			int damagedSections = state.buffer.Read<int>(13);

			float damagedSectionsHealth[13];
			for (int i = 0; i < 13; i++)
			{
				if ((damagedSections >> i) & 1)
				{
					damagedSectionsHealth[i] = state.buffer.ReadSignedFloat(6, 1.0f);
				}
			}
		}

		if (hasBrokenSections)
		{
			/*
				1: Left Wing
				2: Right Wing
				4: Vertical Stabiliser
				32: Left Elevator
				64: Right Elevator
				128: Left Aileron
				256: Right Aileron
				512: Rudder
			*/
			int brokenSections = state.buffer.Read<int>(13);
		}

		if (hasBrokenRotors)
		{
			// Bitfield
			int brokenRotors = state.buffer.Read<int>(8);
		}

		if (hasRotors)
		{
			// Bitfield
			int enabledRotors = state.buffer.Read<int>(8);
		}

		bool isLockedOn = state.buffer.ReadBit();
		bool unk17 = state.buffer.ReadBit();
		bool unk18 = state.buffer.ReadBit();

		if (isLockedOn)
		{
			data.lockOnEntity = state.buffer.Read<uint16_t>(13);
			data.lockOnState = state.buffer.Read<uint32_t>(2);
		}
		else
		{
			data.lockOnEntity = 0;
			data.lockOnState = 0;
		}

		bool isVisible = state.buffer.ReadBit();
		if (isVisible)
		{
			data.visibleDistance = state.buffer.Read<uint32_t>(12);
		}
		else
		{
			data.visibleDistance = 0;
		}

		bool unk23 = state.buffer.ReadBit();
		bool unk24 = state.buffer.ReadBit();
		bool unk25 = state.buffer.ReadBit();
		bool unk26 = state.buffer.ReadBit();

		return true;
	}
};

struct CPlaneControlDataNode
{
	CPlaneControlDataNodeData data;

	bool Parse(SyncParseState& state)
	{
		float yawControl = state.buffer.ReadSignedFloat(8, 1.0f);
		float pitchControl = state.buffer.ReadSignedFloat(8, 1.0f);
		float rollControl = state.buffer.ReadSignedFloat(8, 1.0f);
		float throttleControl = state.buffer.ReadFloat(8, 2.0f);

		bool hasVehicleTask = state.buffer.ReadBit();

		bool isThrottleReversed = state.buffer.ReadBit();
		if (isThrottleReversed)
		{
			float reverseThrottleControl = state.buffer.ReadSignedFloat(8, 1.0f);
		}

		bool hasModifiedNozzelPosition = state.buffer.ReadBit();
		if (hasModifiedNozzelPosition)
		{
			data.nozzlePosition = state.buffer.ReadFloat(8, 1.0f);
		}
		else
		{
			data.nozzlePosition = 0.0f;
		}

		return true;
	}
};

struct CSubmarineGameStateDataNode { };

struct CSubmarineControlDataNode
{
	bool Parse(SyncParseState& state)
	{
		float yawControl = state.buffer.ReadSignedFloat(8, 1.0f);
		float pitchControl = state.buffer.ReadSignedFloat(8, 1.0f);
		float ascentControl = state.buffer.ReadSignedFloat(8, 1.0f);

		return true;
	}
};

struct CTrainGameStateDataNode : GenericSerializeDataNode<CTrainGameStateDataNode>
{
	CTrainGameStateDataNodeData data;

	template<typename TSerializer>
	bool Serialize(TSerializer& s)
	{
		// the object ID of the 'engine' carriage
		s.Serialize(13, data.engineCarriage);

		// the object ID of the carriage attached to this carriage
		s.Serialize(13, data.linkedToBackwardId);

		// the object ID of the carriage this carriage is attached to
		s.Serialize(13, data.linkedToForwardId);

		// Offset from the engine carriage?
		s.SerializeSigned(32, 1000.0f, data.distanceFromEngine);

		s.Serialize(8, data.trainConfigIndex);
		s.Serialize(8, data.carriageIndex);

		// 0 = Main Line, 3 = Metro line
		s.Serialize(8, data.trackId);

		s.SerializeSigned(8, 30.0f, data.cruiseSpeed);

		// 0 = Moving, 1 = Slowing down, 2 = Doors opening, 3 = Stopped, 4 = Doors closing, 5 = Before depart
		s.Serialize(3, data.trainState);

		s.Serialize(data.isEngine);

		//2372 inserted a bool between isEngine and isCaboose
		if (Is2372())
		{
			//Modified by 0x2310A8F9421EBF43
			s.Serialize(data.allowRemovalByPopulation);
		}

		s.Serialize(data.isCaboose);
		s.Serialize(data.isMissionTrain);
		s.Serialize(data.direction);
		s.Serialize(data.hasPassengerCarriages);
		s.Serialize(data.renderDerailed);

		s.Serialize(data.forceDoorsOpen);

		if (Is2372())
		{ 
			// Set on population trains or with SET_TRAIN_STOP_AT_STATIONS
			s.Serialize(data.stopAtStations);

			// Modified by _NETWORK_USE_HIGH_PRECISION_VEHICLE_BLENDING
			s.Serialize(data.highPrecisionBlending);
		}

		return true;
	}
};

struct CPlayerCreationDataNode { };

struct CPlayerGameStateDataNode
{
	CPlayerGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		int playerState = state.buffer.Read<int>(3);
		auto controlsDisabledByScript = state.buffer.ReadBit(); // SET_PLAYER_CONTROL
		int playerTeam = state.buffer.Read<int>(6);
		data.playerTeam = playerTeam;

		if (Is2699())
		{
			auto hasMobileRingState = state.buffer.ReadBit();

			if (hasMobileRingState)
			{
				int mobileRingState = state.buffer.Read<int>(8);
			}
		}
		else
		{
			int mobileRingState = state.buffer.Read<int>(8);
		}

		auto isAirDragMultiplierDefault = state.buffer.ReadBit();

		if (!isAirDragMultiplierDefault)
		{
			float airDragMultiplier = state.buffer.ReadFloat(7, 50.0f);
			data.airDragMultiplier = airDragMultiplier;
		}
		else
		{
			data.airDragMultiplier = 1.0f;
		}

		auto isMaxHealthAndMaxArmourDefault = state.buffer.ReadBit();

		if (isMaxHealthAndMaxArmourDefault)
		{
			int maxHealth = state.buffer.Read<int>(13);
			int maxArmour = state.buffer.Read<int>(12);

			data.maxHealth = maxHealth;
			data.maxArmour = maxArmour;
		}
		else
		{
			data.maxHealth = 100;
			data.maxArmour = 100;
		}

		auto unk9 = state.buffer.ReadBit();
		auto unk10 = state.buffer.ReadBit();
		int unk11 = state.buffer.Read<int>(2);
		auto unk12 = state.buffer.ReadBit();
		auto unk13 = state.buffer.ReadBit();
		auto bulletProof = state.buffer.ReadBit();
		auto fireProof = state.buffer.ReadBit();
		auto explosionProof = state.buffer.ReadBit();
		auto collisionProof = state.buffer.ReadBit();
		auto meleeProof = state.buffer.ReadBit();
		auto drownProof = state.buffer.ReadBit();
		auto steamProof = state.buffer.ReadBit();
		auto unk21 = state.buffer.ReadBit();
		auto unk22 = state.buffer.ReadBit();

		if (unk12)
		{
			int unk23 = state.buffer.Read<int>(7);
		}

		auto neverTarget = state.buffer.ReadBit();
		data.neverTarget = neverTarget;
		auto useKinematicPhysics = state.buffer.ReadBit();
		auto isOverridingReceiveChat = state.buffer.ReadBit();

		if (isOverridingReceiveChat) // v45
		{
			int overrideReceiveChat = state.buffer.Read<int>(32);
		}

		auto isOverridingSendChat = state.buffer.ReadBit();

		if (isOverridingSendChat) // v46
		{
			int overrideSendChat = state.buffer.Read<int>(32);
		}

		auto unk29 = state.buffer.ReadBit();
		auto unk30 = state.buffer.ReadBit();
		auto isSpectating = state.buffer.ReadBit();

		if (isSpectating)
		{
			auto spectatorId = state.buffer.ReadBit();
			data.spectatorId = spectatorId;
		}
		else
		{
			data.spectatorId = 0;
		}

		if (Is2060())
		{
			state.buffer.ReadBit();
		}

		auto isAntagonisticToAnotherPlayer = state.buffer.ReadBit();

		if (isAntagonisticToAnotherPlayer)
		{
			int antagonisticPlayerIndex = state.buffer.Read<int>(5);
		}

		auto unk35 = state.buffer.ReadBit();
		auto pendingTutorialChange = state.buffer.ReadBit();

		if (unk35)
		{
			int tutorialIndex = state.buffer.Read<int>(3);
			int tutorialInstanceId = state.buffer.Read<int>(Is2060() ? 7 : 6);
		}

		auto unk39 = state.buffer.ReadBit();
		auto unk40 = state.buffer.ReadBit();
		auto unk41 = state.buffer.ReadBit();
		auto unk42 = state.buffer.ReadBit();
		auto unk43 = state.buffer.ReadBit();

		auto randomPedsFlee = state.buffer.ReadBit();
		data.randomPedsFlee = randomPedsFlee;
		auto everybodyBackOff = state.buffer.ReadBit();
		data.everybodyBackOff = everybodyBackOff;

		auto unk46 = state.buffer.ReadBit();
		auto unk47 = state.buffer.ReadBit();
		auto unk48 = state.buffer.ReadBit();
		auto unk49 = state.buffer.ReadBit();
		auto unk50 = state.buffer.ReadBit();
		auto unk51 = state.buffer.ReadBit();
		auto unk52 = state.buffer.ReadBit();
		auto unk53 = state.buffer.ReadBit();
		auto unk54 = state.buffer.ReadBit();
		auto unk55 = state.buffer.ReadBit();
		auto unk56 = state.buffer.ReadBit();
		auto unk57 = state.buffer.ReadBit();
		auto unk58 = state.buffer.ReadBit();
		auto unk59 = state.buffer.ReadBit();
		auto unk60 = state.buffer.ReadBit();
		auto unk61 = state.buffer.ReadBit();
		auto unk62 = state.buffer.ReadBit();
		auto unk63 = state.buffer.ReadBit();
		auto unk64 = state.buffer.ReadBit();
		auto unk65 = state.buffer.ReadBit();
		auto unk66 = state.buffer.ReadBit();
		auto unk67 = state.buffer.ReadBit();
		auto unk68 = state.buffer.ReadBit();
		auto unk69 = state.buffer.ReadBit();

		if (Is2060())
		{
			state.buffer.ReadBit();
		}

		if (Is2189())
		{
			state.buffer.ReadBit();
		}

		if (Is3095())
		{
			state.buffer.ReadBit();
		}

		auto unk70 = state.buffer.ReadBit();

		if (unk70)
		{
			int unk71 = state.buffer.Read<int>(16);
		}

		auto unk72 = state.buffer.ReadBit();

		if (unk72)
		{
			int unk73 = state.buffer.Read<int>(5);
		}

		auto unk74 = state.buffer.ReadBit();

		if (unk74)
		{
			int unk75 = state.buffer.Read<int>(32);
		}

		auto isOverridingVoiceProximity = state.buffer.ReadBit();

		if (isOverridingVoiceProximity)
		{
			float voiceProximityOverrideX = state.buffer.ReadSignedFloat(19, 27648.0f);
			float voiceProximityOverrideY = state.buffer.ReadSignedFloat(19, 27648.0f);
			float voiceProximityOverrideZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;

			data.voiceProximityOverrideX = voiceProximityOverrideX;
			data.voiceProximityOverrideY = voiceProximityOverrideY;
			data.voiceProximityOverrideZ = voiceProximityOverrideZ;
		}
		else
		{
			data.voiceProximityOverrideX = 0.0f;
			data.voiceProximityOverrideY = 0.0f;
			data.voiceProximityOverrideZ = 0.0f;
		}

		int unk78 = state.buffer.Read<int>(19);
		auto isInvincible = state.buffer.ReadBit();
		data.isInvincible = isInvincible;

		auto unk80 = state.buffer.ReadBit();

		if (unk80)
		{
			int unk81 = state.buffer.Read<int>(3);
		}

		auto hasDecor = state.buffer.ReadBit();

		if (hasDecor)
		{
			uint8_t decoratorCount = state.buffer.Read<int>(2);

			for (int i = 0; i < decoratorCount; i++)
			{
				uint8_t decorType = state.buffer.Read<int>(3);
				int decorValue = state.buffer.Read<int>(32);
				int decorName = state.buffer.Read<int>(32);
			}
		}

		auto isFriendlyFireAllowed = state.buffer.ReadBit();
		data.isFriendlyFireAllowed = isFriendlyFireAllowed;

		auto unk88 = state.buffer.ReadBit();

		auto isInGarage = state.buffer.ReadBit();

		if (isInGarage)
		{
			int garageInstanceIndex = state.buffer.Read<int>(5);
		}

		auto isInProperty = state.buffer.ReadBit();

		if (isInProperty)
		{
			int propertyId = state.buffer.Read<int>(8);
		}

		auto unk93 = state.buffer.Read<int>(3);
		int unk94 = state.buffer.Read<int>(4);
		auto unk95 = state.buffer.ReadBit();
		auto unk96 = state.buffer.ReadBit();

		float weaponDefenseModifier = state.buffer.ReadFloat(8, 2.0f);
		float weaponDefenseModifier2 = state.buffer.ReadFloat(8, 2.0f);

		data.weaponDefenseModifier = weaponDefenseModifier;
		data.weaponDefenseModifier2 = weaponDefenseModifier2;

		auto isOverridingPopulationControlSphere = state.buffer.ReadBit();

		if (isOverridingPopulationControlSphere)
		{
			float populationSphereX = state.buffer.ReadSignedFloat(19, 27648.0f);
			float populationSphereY = state.buffer.ReadSignedFloat(19, 27648.0f);
			float populationSphereZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;
		}

		int unk101 = state.buffer.Read<int>(13);
		
		if (Is3095())
		{
			state.buffer.Read<int>(8);
			state.buffer.Read<int>(8);
		}

		auto unk102 = state.buffer.ReadBit();
		auto noCollision = state.buffer.ReadBit();
		auto unk104 = state.buffer.ReadBit();
		auto unk105 = state.buffer.ReadBit();
		auto unk106 = state.buffer.ReadBit();

		if (unk106)
		{
			auto unk107 = state.buffer.ReadBit();
			int unk108 = state.buffer.Read<int>(2);
		}

		float unk109 = state.buffer.ReadFloat(8, 10.0f);
		auto isWeaponDamageModifierSet = state.buffer.ReadBit();

		if (isWeaponDamageModifierSet)
		{
			float weaponDamageModifier = state.buffer.ReadFloat(10, 10.0f);
			data.weaponDamageModifier = weaponDamageModifier;
		}
		else
		{
			data.weaponDamageModifier = 1.0f;
		}

		auto isMeleeWeaponDamageModifierSet = state.buffer.ReadBit();

		if (isMeleeWeaponDamageModifierSet)
		{
			float meleeWeaponDamageModifier = state.buffer.ReadFloat(10, 100.0f);
			data.meleeWeaponDamageModifier = meleeWeaponDamageModifier;
		}
		else
		{
			data.meleeWeaponDamageModifier = 1.0f;
		}

		auto isSomethingModifierSet = state.buffer.ReadBit();

		if (isSomethingModifierSet)
		{
			float somethingModifier = state.buffer.ReadFloat(10, 100.0f);
		}
		else
		{
			// 1.0f
		}

		auto isSuperJumpEnabled = state.buffer.ReadBit();
		data.isSuperJumpEnabled = isSuperJumpEnabled;

		auto unk117 = state.buffer.ReadBit();
		auto unk118 = state.buffer.ReadBit();
		auto unk119 = state.buffer.ReadBit();

		if (unk119)
		{
			float unk120X = state.buffer.ReadSignedFloat(19, 27648.0f);
			float unk120Y = state.buffer.ReadSignedFloat(19, 27648.0f);
			float unk120Z = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;
		}

		if (Is2060()) // Added in trailing bits in 2060
		{
			auto v47 = state.buffer.ReadBit();
			if (v47)
			{
				auto unk496 = state.buffer.Read<uint8_t>(2);
				auto unk497 = state.buffer.Read<uint8_t>(3);
				auto unk498 = state.buffer.ReadBit();
				auto unk499 = state.buffer.Read<uint8_t>(6);
			}
		}

		return true;
	}
};

struct CPlayerAppearanceDataNode
{
	uint32_t model;

	bool Parse(SyncParseState& state)
	{
		model = state.buffer.Read<uint32_t>(32);

#if 0
		uint32_t voiceGroup = state.buffer.Read<uint32_t>(32);
		int phoneMode = state.buffer.Read<int>(2);

		bool isTakingOffVehicleHelmet = state.buffer.ReadBit();
		bool isPuttingOnVehicleHelmet = state.buffer.ReadBit();
		bool hasVehicleHelmet = state.buffer.ReadBit();
		bool isHelmetVisorUp = state.buffer.ReadBit();
		bool hasHelmetVisorPropIndices = state.buffer.ReadBit();
		bool isSwitchingHelmetVisor = state.buffer.ReadBit();

		int unk_0x451 = state.buffer.Read<int>(8); // 0x98 of CTaskTakeOffHelmet (b2545)
		uint32_t badgeTextureDict = state.buffer.Read<uint32_t>(32);
		uint32_t badgeTextureName = state.buffer.Read<uint32_t>(32);

		if (isPuttingOnVehicleHelmet || hasVehicleHelmet || isSwitchingHelmetVisor)
		{
			int vehicleHelmetTextureId = state.buffer.Read<int>(8);
			int vehicleHelmetPropId = state.buffer.Read<int>(10);
			int helmetVisorDownPropId = state.buffer.Read<int>(10);
			int helmetVisorUpPropId = state.buffer.Read<int>(10);
		}

		bool hasParachuteTintIndex = state.buffer.ReadBit();
		if (hasParachuteTintIndex)
		{
			int parachuteTintIndex = state.buffer.Read<int>(4);
			int parachutePackTintIndex = state.buffer.Read<int>(4);
		}

		bool hasRespawnObjectId = state.buffer.ReadBit();
		if (hasRespawnObjectId)
		{
			int respawnObjectId = state.buffer.Read<int>(13);
		}

		CPedAppearanceDataNode::DeserializePedProps(state);

		CPedAppearanceDataNode::DeserializePedComponents(state);

		bool hasHeadBlend = state.buffer.ReadBit();
		if (hasHeadBlend)
		{
			DeserializeHeadBlend(state);
		}

		bool hasDecorations = state.buffer.ReadBit();
		if (hasDecorations)
		{
			int numDecorations = Is2699() ? 56 : Is2060() ? 52 : 44;
			for (int i = 0; i < numDecorations; i++)
			{
				int packedDecoration = state.buffer.Read<int>(32);
			}

			if (state.buffer.ReadBit())
			{
				state.buffer.Read<int>(32); // 0x340 of CNetObj* (b2545, damage / emblem related?)
			}
		}

		DeserializePedSecondaryTask(state);

		bool hasFacialClipset = state.buffer.ReadBit();
		if (hasFacialClipset)
		{
			uint32_t facialClipsetName = state.buffer.Read<uint32_t>(32);
			uint32_t facialClipsetDict = state.buffer.Read<uint32_t>(32);
		}

		bool hasFacialIdleAnim = state.buffer.ReadBit();
		if (hasFacialIdleAnim)
		{
			uint32_t facialIdleAnim = state.buffer.Read<uint32_t>(32);
		}

		bool hasFacialIdleAnimDict = state.buffer.ReadBit();
		if (hasFacialIdleAnimDict)
		{
			uint32_t facialIdleAnimDict = state.buffer.Read<uint32_t>(32); 
		}

		bool hasDamagePack = state.buffer.ReadBit();
		if (hasDamagePack)
		{
			uint32_t damagePackName = state.buffer.Read<uint32_t>(32);
		}
#endif

		return true;
	}

	static void DeserializeHeadBlend(SyncParseState& state)
	{
		float headBlend = state.buffer.ReadFloat(8, 1.05f);
		float textureBlend = state.buffer.ReadFloat(8, 1.05f);
		float variationBlend = state.buffer.ReadFloat(8, 1.05f);

		int eyeColourIndex = state.buffer.ReadSigned<int>(9);
		int hairFirstTint = state.buffer.Read<int>(8);
		int hairSecondTint = state.buffer.Read<int>(8);

		bool hasHeadGeometry = state.buffer.ReadBit();
		if (hasHeadGeometry)
		{
			int headGeometryFlags = state.buffer.Read<int>(3);
			for (int i = 0; i < 3; i++)
			{
				if ((headGeometryFlags >> i) & 1)
				{
					int headGeometry = state.buffer.Read<int>(8);
				}
			}
		}

		bool hasHeadTextures = state.buffer.ReadBit();
		if (hasHeadTextures)
		{
			int headTextureFlags = state.buffer.Read<int>(3);
			for (int i = 0; i < 3; i++)
			{
				if ((headTextureFlags >> i) & 1)
				{
					int headTexture = state.buffer.Read<int>(8);
				}
			}
		}

		bool hasOverlayTextures = state.buffer.ReadBit();
		if (hasOverlayTextures)
		{
			int overlayTextureFlags = state.buffer.Read<int>(13);
			for (int i = 0; i < 13; i++)
			{
				if ((overlayTextureFlags >> i) & 1)
				{
					int overlayTexture = state.buffer.Read<int>(8);
					float overlayAlpha = state.buffer.ReadFloat(8, 1.0f);
					float overlayNormal = state.buffer.ReadFloat(8, 1.0f);

					bool hasOverlayTint = state.buffer.ReadBit();
					if (hasOverlayTint)
					{
						int overlayFirstTint = state.buffer.Read<int>(8);
						int overlaySecondTint = state.buffer.Read<int>(8);
						int overlayRampType = state.buffer.Read<int>(2);
					}
				}
			}
		}

		bool hasMicroMorph = state.buffer.ReadBit();
		if (hasMicroMorph)
		{
			int microMorphFlags = state.buffer.Read<int>(20);
			for (int i = 0; i < 20; i++)
			{
				if ((microMorphFlags >> i) & 1)
				{
					if (i >= 18)
					{
						float morphBlend = state.buffer.ReadFloat(8, 1.0f);
					}
					else
					{
						float morphBlend = state.buffer.ReadSignedFloat(8, 1.0f);
					}
				}
			}
		}

		bool usePaletteRgb = state.buffer.ReadBit();
		if (usePaletteRgb)
		{
			for (int i = 0; i < 4; i++)
			{
				int paletteRed = state.buffer.Read<int>(8);
				int paletteGreen = state.buffer.Read<int>(8);
				int paletteBlue = state.buffer.Read<int>(8);
			}
		}

		bool hasParents = state.buffer.ReadBit();
		if (hasParents)
		{
			int firstParentHead = state.buffer.Read<int>(8);
			int secondParentHead = state.buffer.Read<int>(8);
			float firstParentBlend = state.buffer.ReadFloat(8, 1.0f);
			float secondParentBlend = state.buffer.ReadFloat(8, 1.0f);
		}
	}

	static void DeserializePedSecondaryTask(SyncParseState& state)
	{
		bool hasSecondaryTask = state.buffer.ReadBit();
		if (hasSecondaryTask)
		{
			uint32_t animDictHash = state.buffer.Read<uint32_t>(32);
			bool isTaskMove = state.buffer.ReadBit();
			bool isTaskMoveBlendDefault = state.buffer.ReadBit();

			if (isTaskMove)
			{
				bool isTaskMoveBlocked = state.buffer.ReadBit();
				bool hasFirstFloatSignal = state.buffer.ReadBit();
				bool hasSecondFloatSignal = state.buffer.ReadBit();
				bool hasThirdFloatSignal = state.buffer.ReadBit();

				if (hasFirstFloatSignal)
				{
					uint32_t signalName = state.buffer.Read<uint32_t>(32);
					float signalValue = state.buffer.ReadSignedFloat(9, 2.0f);
				}

				if (hasSecondFloatSignal)
				{
					uint32_t signalName = state.buffer.Read<uint32_t>(32);
					float signalValue = state.buffer.ReadSignedFloat(9, 2.0f);
				}

				if (hasThirdFloatSignal)
				{
					uint32_t signalName = state.buffer.Read<uint32_t>(32);
					float signalValue = state.buffer.ReadSignedFloat(9, 2.0f);
				}

				uint32_t animName = state.buffer.Read<uint32_t>(32);
				uint32_t stateName = state.buffer.Read<uint32_t>(32);
			}
			else
			{
				bool hasPhoneTask = state.buffer.ReadBit();
				if (hasPhoneTask)
				{
					bool hasPhoneGesture = state.buffer.ReadBit();
					if (hasPhoneGesture)
					{
						uint32_t animName = state.buffer.Read<uint32_t>(32);
						uint32_t boneMask = state.buffer.Read<uint32_t>(32);
						float blendInDuration = state.buffer.ReadSignedFloat(9, 1.0f);
						float blendOutDuration = state.buffer.ReadSignedFloat(9, 1.0f);
						int animFlags = state.buffer.Read<int>(8);
					}
				}
				else
				{
					bool usingDefaultBoneMask = state.buffer.ReadBit();
					int animId = state.buffer.Read<int>(5);

					if (!usingDefaultBoneMask)
					{
						uint32_t boneMask = state.buffer.Read<uint32_t>(32);
					}

					uint32_t animName = state.buffer.Read<uint32_t>(32);
					int animFlags = state.buffer.Read<int>(32);
				}
			}
		}
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
				m_standingOnHandle = state.buffer.Read<int>(13); // Standing On
				m_standingOffsetX = state.buffer.ReadSignedFloat(14, 40.0f); // Standing On Local Offset X
				m_standingOffsetY = state.buffer.ReadSignedFloat(14, 40.0f); // Standing On Local Offset Y
				m_standingOffsetZ = state.buffer.ReadSignedFloat(10, 20.0f); // Standing On Local Offset Z
			}
			else
			{
				m_standingOnHandle = 0;
				m_standingOffsetX = 0.0f;
				m_standingOffsetY = 0.0f;
				m_standingOffsetZ = 0.0f;
			}

			isStandingOn = isStandingOn;
		}

		auto posX = state.buffer.ReadFloat(12, 54.0f);
		auto posY = state.buffer.ReadFloat(12, 54.0f);
		auto posZ = state.buffer.ReadFloat(12, 69.0f);

		m_posX = posX;
		m_posY = posY;
		m_posZ = posZ;

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

struct CPlayerWantedAndLOSDataNode
{
	CPlayerWantedAndLOSNodeData data;

	bool Parse(SyncParseState& state)
	{
		auto wantedLevel = state.buffer.Read<int>(3);
		data.wantedLevel = wantedLevel;
		auto unk0 = state.buffer.Read<int>(3);
		auto fakeWantedLevel = state.buffer.Read<int>(3);
		data.fakeWantedLevel = fakeWantedLevel;
		auto pendingWantedLevel = state.buffer.ReadBit();
		auto unk3 = state.buffer.ReadBit();
		auto isWanted = state.buffer.ReadBit();

		if (isWanted) {
			auto wantedPositionX = state.buffer.ReadSignedFloat(19, 27648.0f);
			auto wantedPositionY = state.buffer.ReadSignedFloat(19, 27648.0f);
			auto wantedPositionZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;
			data.wantedPositionX = wantedPositionX;
			data.wantedPositionY = wantedPositionY;
			data.wantedPositionZ = wantedPositionZ;

			auto posX2 = state.buffer.ReadSignedFloat(19, 27648.0f);
			auto posY2 = state.buffer.ReadSignedFloat(19, 27648.0f);
			auto posZ2 = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;


			auto currentTime = state.buffer.Read<int>(32);
			auto pursuitStartTime = state.buffer.Read<int>(32);
			if (pursuitStartTime != 0)
				data.timeInPursuit = currentTime - pursuitStartTime;
			else
				data.timeInPursuit = 0;
		}
		else {
			data.wantedPositionX = 0.0f;
			data.wantedPositionY = 0.0f;
			data.wantedPositionZ = 0.0f;

			if (data.timeInPursuit != -1) {
				data.timeInPrevPursuit = data.timeInPursuit;
				data.timeInPursuit = -1;
			}
		}

		auto unk4 = state.buffer.ReadBit();
		auto copsCantSeePlayer = state.buffer.ReadBit();
		auto isEvading = state.buffer.ReadBit();
		data.isEvading = isEvading;
		return true;
	}
};

template<typename TNode>
struct SyncTree : public SyncTreeBaseImpl<TNode, false>
{
	virtual void GetPosition(float* posOut) override
	{
		auto [hasSdn, secDataNode] = this->template GetData<CSectorDataNode>();
		auto [hasSpdn, secPosDataNode] = this->template GetData<CSectorPositionDataNode>();
		auto [hasPspdn, playerSecPosDataNode] = this->template GetData<CPlayerSectorPosNode>();
		auto [hasOspdn, objectSecPosDataNode] = this->template GetData<CObjectSectorPosNode>();
		auto [hasPspmdn, pedSecPosMapDataNode] = this->template GetData<CPedSectorPosMapNode>();
		auto [hasDoor, doorCreationDataNode] = this->template GetData<CDoorCreationDataNode>();
		auto [hasPickupPlacement, pickupPlacementCreationDataNode] = this->template GetData<CPickupPlacementCreationDataNode>();
		auto [hasPgsdn, pedGameStateDataNode] = this->template GetData<CPedGameStateDataNode>();

		auto sectorX = (hasSdn) ? secDataNode->m_sectorX : 512;
		auto sectorY = (hasSdn) ? secDataNode->m_sectorY : 512;
		auto sectorZ = (hasSdn) ? secDataNode->m_sectorZ : 0;

		auto sectorPosX =
			(hasSpdn) ? secPosDataNode->m_posX :
				(hasPspdn) ? playerSecPosDataNode->m_posX :
					(hasOspdn) ? objectSecPosDataNode->m_posX :
						(hasPspmdn) ? pedSecPosMapDataNode->m_posX :
							0.0f;

		auto sectorPosY =
			(hasSpdn) ? secPosDataNode->m_posY :
				(hasPspdn) ? playerSecPosDataNode->m_posY :
					(hasOspdn) ? objectSecPosDataNode->m_posY :
						(hasPspmdn) ? pedSecPosMapDataNode->m_posY :
							0.0f;

		auto sectorPosZ =
			(hasSpdn) ? secPosDataNode->m_posZ :
				(hasPspdn) ? playerSecPosDataNode->m_posZ :
					(hasOspdn) ? objectSecPosDataNode->m_posZ :
						(hasPspmdn) ? pedSecPosMapDataNode->m_posZ :
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

		if (hasPickupPlacement)
		{
			posOut[0] = pickupPlacementCreationDataNode->posX;
			posOut[1] = pickupPlacementCreationDataNode->posY;
			posOut[2] = pickupPlacementCreationDataNode->posZ;
		}

		if (hasPspdn)
		{
			if (g_serverGameState && playerSecPosDataNode->isStandingOn)
			{
				auto entity = g_serverGameState->GetEntity(0, playerSecPosDataNode->m_standingOnHandle);

				if (entity && entity->type != sync::NetObjEntityType::Player)
				{
					entity->syncTree->GetPosition(posOut);

					posOut[0] += playerSecPosDataNode->m_standingOffsetX;
					posOut[1] += playerSecPosDataNode->m_standingOffsetY;
					posOut[2] += playerSecPosDataNode->m_standingOffsetZ;
				}
			}
		}

		// if in a vehicle, force the current vehicle's position to be used
		if (hasPgsdn)
		{
			if (g_serverGameState && pedGameStateDataNode->data.curVehicle != -1)
			{
				auto entity = g_serverGameState->GetEntity(0, pedGameStateDataNode->data.curVehicle);

				if (entity && entity->type != fx::sync::NetObjEntityType::Ped && entity->type != fx::sync::NetObjEntityType::Player)
				{
					entity->syncTree->GetPosition(posOut);
				}
			}
		}
	}

	virtual CDoorMovementDataNodeData* GetDoorMovement() override
	{
		auto [hasNode, node] = this->template GetData<CDoorMovementDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CDoorScriptInfoDataNodeData* GetDoorScriptInfo() override
	{
		auto [hasNode, node] = this->template GetData<CDoorScriptInfoDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CDoorScriptGameStateDataNodeData* GetDoorScriptGameState() override
	{
		auto [hasNode, node] = this->template GetData<CDoorScriptGameStateDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CHeliControlDataNodeData* GetHeliControl() override
	{
		auto [hasNode, node] = this->template GetData<CHeliControlDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CPlayerCameraNodeData* GetPlayerCamera() override
	{
		auto [hasCdn, cameraNode] = this->template GetData<CPlayerCameraDataNode>();

		return (hasCdn) ? &cameraNode->data : nullptr;
	}

	virtual CPlayerWantedAndLOSNodeData* GetPlayerWantedAndLOS() override 
	{
		auto [hasNode, node] = this->template GetData<CPlayerWantedAndLOSDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CPedGameStateNodeData* GetPedGameState() override
	{
		auto[hasPdn, pedNode] = this->template GetData<CPedGameStateDataNode>();

		return (hasPdn) ? &pedNode->data : nullptr;
	}

	virtual uint64_t GetPedGameStateFrameIndex() override
	{
		auto pedBase = this->template GetNode<CPedGameStateDataNode>();

		return (pedBase) ? pedBase->frameIndex : 0;
	}

	virtual CVehicleGameStateNodeData* GetVehicleGameState() override
	{
		auto[hasVdn, vehNode] = this->template GetData<CVehicleGameStateDataNode>();

		return (hasVdn) ? &vehNode->data : nullptr;
	}

	virtual CPedTaskTreeDataNodeData* GetPedTaskTree() override
	{
		auto [hasNode, node] = this->template GetData<CPedTaskTreeDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CPlaneGameStateDataNodeData* GetPlaneGameState() override
	{
		auto [hasNode, node] = this->template GetData<CPlaneGameStateDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CPlaneControlDataNodeData* GetPlaneControl() override
	{
		auto [hasNode, node] = this->template GetData<CPlaneControlDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CTrainGameStateDataNodeData* GetTrainState() override
	{
		auto [hasNode, node] = this->template GetData<CTrainGameStateDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CPlayerGameStateNodeData* GetPlayerGameState() override
	{
		auto [hasNode, node] = this->template GetData<CPlayerGameStateDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CVehicleAppearanceNodeData* GetVehicleAppearance() override
	{
		auto [hasNode, node] = this->template GetData<CVehicleAppearanceDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CPedHealthNodeData* GetPedHealth() override
	{
		auto [hasNode, node] = this->template GetData<CPedHealthDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CVehicleHealthNodeData* GetVehicleHealth() override
	{
		auto [hasNode, node] = this->template GetData<CVehicleHealthDataNode>();

		return (hasNode) ? &node->data : nullptr;
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
		auto [hasNode, node] = this->template GetData<CObjectOrientationDataNode>();

		return (hasNode) ? &node->data : nullptr;
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

	virtual CDummyObjectCreationNodeData* GetDummyObjectState() override
	{
		auto [hasObject, objectCreationNode] = this->template GetData<CObjectCreationDataNode>();
		if (hasObject)
		{
			if (objectCreationNode->m_createdBy == 0 || objectCreationNode->m_createdBy == 2)
			{
				return &objectCreationNode->dummy;
			}
		}

		return nullptr;
	}

	virtual CBaseAttachNodeData* GetAttachment() override
	{
		auto [hasPed, pedAttachNode] = this->template GetData<CPedAttachDataNode>();
		if (hasPed)
		{
			return &pedAttachNode->data;
		}

		auto [hasPhys, physicalAttachNode] = this->template GetData<CPhysicalAttachDataNode>();
		if (hasPhys)
		{
			return &physicalAttachNode->data;
		}

		return nullptr;
	}

	virtual CObjectGameStateNodeData* GetObjectGameState() override
	{
		auto [hasObj, objStateNode] = this->template GetData<CObjectGameStateDataNode>();
		if (hasObj)
		{
			return &objStateNode->data;
		}

		return nullptr;
	}

	virtual CHeliHealthNodeData* GetHeliHealth() override
	{
		auto [hasNode, node] = this->template GetData<CHeliHealthDataNode>();

		return hasNode ? &node->data : nullptr;
	}

	virtual CVehicleSteeringNodeData* GetVehicleSteeringData() override
	{
		auto [hasNode, node] = this->template GetData<CVehicleSteeringDataNode>();

		return hasNode ? &node->data : nullptr;
	}

	virtual CEntityScriptGameStateNodeData* GetEntityScriptGameState() override
	{
		auto [hasNode, node] = this->template GetData<CEntityScriptGameStateDataNode>();

		return hasNode ? &node->data : nullptr;
	}

	virtual CVehicleDamageStatusNodeData* GetVehicleDamageStatus() override
	{
		auto [hasNode, node] = this->template GetData<CVehicleDamageStatusDataNode>();

		return hasNode ? &node->data : nullptr;
	}

	virtual CBoatGameStateNodeData* GetBoatGameState() override
	{
		auto [hasNode, node] = this->template GetData<CBoatGameStateDataNode>();

		return hasNode ? &node->data : nullptr;
	}

	virtual CPedMovementGroupNodeData* GetPedMovementGroup() override
	{
		auto [hasNode, node] = this->template GetData<CPedMovementGroupDataNode>();

		return hasNode ? &node->data : nullptr;
	}

	virtual CPedAINodeData* GetPedAI() override
	{
		auto [hasNode, node] = this->template GetData<CPedAIDataNode>();

		return hasNode ? &node->data : nullptr;
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
			auto vehScriptNode = this->template GetNode<CVehicleScriptGameStateDataNode>();
			if (vehScriptNode && vehScriptNode->length > 0)
			{
				*popType = vehScriptNode->node.m_popType;
			}
			else
			{
				*popType = vehCreationNode->m_popType;
			}
			return true;
		}

		auto[hasPcn, pedCreationNode] = this->template GetData<CPedCreationDataNode>();

		if (hasPcn)
		{
			auto pedScriptNode = this->template GetNode<CPedScriptGameStateDataNode>();
			if (pedScriptNode && pedScriptNode->length > 0)
			{
				*popType = pedScriptNode->node.m_popType;
			}
			else
			{
				*popType = pedCreationNode->m_popType;
			}
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

		auto[hasPan, playerAppearanceNode] = this->template GetData<CPlayerAppearanceDataNode>();

		if (hasPan)
		{
			*modelHash = playerAppearanceNode->model;
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
		return nullptr;
	}
};

using CAutomobileSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 14>,
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode, 2>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 57>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 50>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 28>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 179>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 34>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 65>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 57>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode, 34>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 28>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 31>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 36>
		>
	>
>;
using CBikeSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 14>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 57>,
					NodeWrapper<NodeIds<127, 127, 0>, CBikeGameStateDataNode, 1>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 50>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 28>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 179>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 34>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 65>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 57>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode, 34>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 28>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 31>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 36>
		>
	>
>;
using CBoatSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 14>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 87, 0>,
				ParentNode<
					NodeIds<127, 87, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 57>,
					NodeWrapper<NodeIds<87, 87, 0>, CBoatGameStateDataNode, 5>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 50>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 28>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 179>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 34>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 65>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 57>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode, 34>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 28>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 31>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 36>
		>
	>
>;
using CDoorSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CDoorCreationDataNode, 17>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
			NodeWrapper<NodeIds<127, 127, 1>, CDoorScriptInfoDataNode, 28>,
			NodeWrapper<NodeIds<127, 127, 1>, CDoorScriptGameStateDataNode, 8>
		>,
		NodeWrapper<NodeIds<86, 86, 0>, CDoorMovementDataNode, 2>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>
		>
	>
>;
using CHeliSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 14>,
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode, 2>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 57>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 50>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 28>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 179>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 34>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 65>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 57>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode, 34>,
			NodeWrapper<NodeIds<87, 87, 0>, CHeliHealthDataNode, 16>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 28>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 31>,
				NodeWrapper<NodeIds<86, 86, 0>, CHeliControlDataNode, 8>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 36>
		>
	>
>;
using CObjectSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CObjectCreationDataNode, 30>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CObjectGameStateDataNode, 44>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CObjectScriptGameStateDataNode, 14>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 28>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalHealthDataNode, 19>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CObjectSectorPosNode, 8>,
			NodeWrapper<NodeIds<87, 87, 0>, CObjectOrientationDataNode, 8>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalAngVelocityDataNode, 4>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>
		>
	>
>;
using CPedSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CPedCreationDataNode, 20>,
			NodeWrapper<NodeIds<1, 0, 1>, CPedScriptCreationDataNode, 1>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedGameStateDataNode, 104>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedComponentReservationDataNode, 65>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CPedScriptGameStateDataNode, 115>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, CPedAttachDataNode, 22>,
			NodeWrapper<NodeIds<127, 127, 0>, CPedHealthDataNode, 17>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementGroupDataNode, 26>,
			NodeWrapper<NodeIds<127, 127, 1>, CPedAIDataNode, 9>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedAppearanceDataNode, 141>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode, 3>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode, 5>,
			ParentNode<
				NodeIds<127, 87, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, CPedTaskTreeDataNode, 28>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosMapNode, 12>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedSectorPosNavMeshNode, 4>
		>,
		ParentNode<
			NodeIds<87, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>,
			NodeWrapper<NodeIds<87, 0, 0>, CPedInventoryDataNode, 321>, // Changed from 5 to 87 in CloneManager.cpp
			NodeWrapper<NodeIds<4, 4, 1>, CPedTaskSequenceDataNode, 1>
		>
	>
>;
using CPickupSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CPickupCreationDataNode, 66>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
				NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>
			>,
			ParentNode<
				NodeIds<127, 127, 1>,
				NodeWrapper<NodeIds<127, 127, 1>, CPickupScriptGameStateNode, 14>,
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalGameStateDataNode, 4>,
				NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
				NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>,
				NodeWrapper<NodeIds<127, 127, 1>, CPhysicalHealthDataNode, 19>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, CPhysicalAttachDataNode, 28>
		>,
		ParentNode<
			NodeIds<87, 87, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPickupSectorPosNode, 8>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalAngVelocityDataNode, 4>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>
		>
	>
>;
using CPickupPlacementSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		NodeWrapper<NodeIds<1, 0, 0>, CPickupPlacementCreationDataNode, 54>,
		NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
		NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
		NodeWrapper<NodeIds<127, 127, 0>, CPickupPlacementStateDataNode, 7>
	>
>;
using CPlaneSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 14>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 57>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 50>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 28>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 179>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 34>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 65>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 57>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode, 34>,
			NodeWrapper<NodeIds<127, 127, 0>, CPlaneGameStateDataNode, 52>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 28>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 31>,
				NodeWrapper<NodeIds<86, 86, 0>, CPlaneControlDataNode, 7>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 36>
		>
	>
>;
using CSubmarineSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 14>
		>,
		ParentNode<
			NodeIds<127, 87, 0>,
			ParentNode<
				NodeIds<127, 87, 0>,
				ParentNode<
					NodeIds<127, 87, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 57>,
					NodeWrapper<NodeIds<87, 87, 0>, CSubmarineGameStateDataNode, 1>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 50>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 28>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 179>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 34>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 65>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 57>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode, 34>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 28>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 31>,
				NodeWrapper<NodeIds<86, 86, 0>, CSubmarineControlDataNode, 4>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 36>
		>
	>
>;
using CPlayerSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CPlayerCreationDataNode, 128>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			ParentNode<
				NodeIds<127, 87, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedGameStateDataNode, 104>,
					NodeWrapper<NodeIds<127, 127, 0>, CPedComponentReservationDataNode, 65>
				>,
				ParentNode<
					NodeIds<127, 87, 0>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<87, 87, 0>, CPlayerGameStateDataNode, 104>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 1>, CPedAttachDataNode, 22>,
			NodeWrapper<NodeIds<127, 127, 0>, CPedHealthDataNode, 17>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementGroupDataNode, 26>,
			NodeWrapper<NodeIds<127, 127, 1>, CPedAIDataNode, 9>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerAppearanceDataNode, 560>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerPedGroupDataNode, 19>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerAmbientModelStreamingNode, 5>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerGamerDataNode, 370>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerExtendedGameStateNode, 20>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedOrientationDataNode, 3>,
			NodeWrapper<NodeIds<87, 87, 0>, CPedMovementDataNode, 5>,
			ParentNode<
				NodeIds<127, 87, 0>,
				NodeWrapper<NodeIds<127, 127, 0>, CPedTaskTreeDataNode, 28>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>,
				NodeWrapper<NodeIds<87, 87, 0>, CPedTaskSpecificDataNode, 77>
			>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CPlayerSectorPosNode, 13>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerCameraDataNode, 24>,
			NodeWrapper<NodeIds<86, 86, 0>, CPlayerWantedAndLOSDataNode, 30>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>
		>
	>
>;
using CTrailerSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 14>,
			NodeWrapper<NodeIds<1, 0, 0>, CAutomobileCreationDataNode, 2>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 57>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 50>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 28>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 179>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 34>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 65>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 57>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode, 34>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 28>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 31>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 36>
		>
	>
>;
using CTrainSyncTree = SyncTree<
	ParentNode<
		NodeIds<127, 0, 0>,
		ParentNode<
			NodeIds<1, 0, 0>,
			NodeWrapper<NodeIds<1, 0, 0>, CVehicleCreationDataNode, 14>
		>,
		ParentNode<
			NodeIds<127, 127, 0>,
			ParentNode<
				NodeIds<127, 127, 0>,
				ParentNode<
					NodeIds<127, 127, 0>,
					NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode, 2>,
					NodeWrapper<NodeIds<127, 127, 0>, CDynamicEntityGameStateDataNode, 102>,
					NodeWrapper<NodeIds<127, 127, 0>, CPhysicalGameStateDataNode, 4>,
					NodeWrapper<NodeIds<127, 127, 0>, CVehicleGameStateDataNode, 57>,
					NodeWrapper<NodeIds<127, 127, 0>, CTrainGameStateDataNode, 16>
				>,
				ParentNode<
					NodeIds<127, 127, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptGameStateDataNode, 1>,
					NodeWrapper<NodeIds<127, 127, 1>, CPhysicalScriptGameStateDataNode, 13>,
					NodeWrapper<NodeIds<127, 127, 1>, CVehicleScriptGameStateDataNode, 50>,
					NodeWrapper<NodeIds<127, 127, 1>, CEntityScriptInfoDataNode, 24>
				>
			>,
			NodeWrapper<NodeIds<127, 127, 0>, CPhysicalAttachDataNode, 28>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleAppearanceDataNode, 179>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleDamageStatusDataNode, 34>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleComponentReservationDataNode, 65>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleHealthDataNode, 57>,
			NodeWrapper<NodeIds<127, 127, 0>, CVehicleTaskDataNode, 34>
		>,
		ParentNode<
			NodeIds<127, 86, 0>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorDataNode, 4>,
			NodeWrapper<NodeIds<87, 87, 0>, CSectorPositionDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CEntityOrientationDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CPhysicalVelocityDataNode, 5>,
			NodeWrapper<NodeIds<87, 87, 0>, CVehicleAngVelocityDataNode, 4>,
			ParentNode<
				NodeIds<127, 86, 0>,
				NodeWrapper<NodeIds<86, 86, 0>, CVehicleSteeringDataNode, 2>,
				NodeWrapper<NodeIds<87, 87, 0>, CVehicleControlDataNode, 28>,
				NodeWrapper<NodeIds<127, 127, 0>, CVehicleGadgetDataNode, 31>
			>
		>,
		ParentNode<
			NodeIds<4, 0, 0>,
			NodeWrapper<NodeIds<4, 0, 0>, CMigrationDataNode, 13>,
			NodeWrapper<NodeIds<4, 0, 0>, CPhysicalMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 1>, CPhysicalScriptMigrationDataNode, 1>,
			NodeWrapper<NodeIds<4, 0, 0>, CVehicleProximityMigrationDataNode, 36>
		>
	>
>;
}
