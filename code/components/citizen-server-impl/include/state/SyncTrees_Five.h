#pragma once

#include <state/ServerGameState.h>

#include <array>
#include <bitset>
#include <variant>

#include <boost/type_index.hpp>

extern std::string g_enforcedGameBuild;

inline bool Is2060()
{
	static bool value = ([]()
	{
		return g_enforcedGameBuild == "2060";
	})();

	return value;
}

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
	if (std::get<2>(ids) && !(state.objType & std::get<2>(ids)))
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
	if (std::get<2>(ids) && !(state.objType & std::get<2>(ids)))
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

template<class... TChildren>
struct ChildList
{

};

template<typename T, typename... TRest>
struct ChildList<T, TRest...>
{
	T first;
	ChildList<TRest...> rest;
};

template<typename T>
struct ChildList<T>
{
	T first;
};

template<typename T>
struct ChildListInfo
{

};

template<typename... TChildren>
struct ChildListInfo<ChildList<TChildren...>>
{
	static constexpr size_t Size = sizeof...(TChildren);
};

template<size_t I, typename T>
struct ChildListElement;

template<size_t I, typename T, typename... TChildren>
struct ChildListElement<I, ChildList<T, TChildren...>>
	: ChildListElement<I - 1, ChildList<TChildren...>>
{
};

template<typename T, typename... TChildren>
struct ChildListElement<0, ChildList<T, TChildren...>>
{
	using Type = T;
};

template<size_t I>
struct ChildListGetter
{
	template<typename... TChildren>
	static inline auto& Get(ChildList<TChildren...>& list)
	{
		return ChildListGetter<I - 1>::Get(list.rest);
	}

	template<typename TList>
	static inline constexpr size_t GetOffset(size_t offset = 0)
	{
		return ChildListGetter<I - 1>::template GetOffset<decltype(TList::rest)>(
			offset + offsetof(TList, rest)
		);
	}
};

template<>
struct ChildListGetter<0>
{
	template<typename... TChildren>
	static inline auto& Get(ChildList<TChildren...>& list)
	{
		return list.first;
	}

	template<typename TList>
	static inline constexpr size_t GetOffset(size_t offset = 0)
	{
		return offset +
			offsetof(TList, first);
	}
};

template<typename TTuple>
struct Foreacher
{
	template<typename TFn, size_t I = 0>
	static inline std::enable_if_t<(I == ChildListInfo<TTuple>::Size)> for_each_in_tuple(TTuple& tuple, const TFn& fn)
	{

	}

	template<typename TFn, size_t I = 0>
	static inline std::enable_if_t<(I != ChildListInfo<TTuple>::Size)> for_each_in_tuple(TTuple& tuple, const TFn& fn)
	{
		fn(ChildListGetter<I>::Get(tuple));

		for_each_in_tuple<TFn, I + 1>(tuple, fn);
	}
};

template<typename TIds, typename... TChildren>
struct ParentNode : public NodeBase
{
	ChildList<TChildren...> children;

	template<typename TData>
	inline static constexpr size_t GetOffsetOf()
	{
		return LoopChildren<TData>();
	}

	template<typename TData, size_t I = 0>
	inline static constexpr std::enable_if_t<I == sizeof...(TChildren), size_t> LoopChildren()
	{
		return 0;
	}

	template<typename TData, size_t I = 0>
	inline static constexpr std::enable_if_t<I != sizeof...(TChildren), size_t> LoopChildren()
	{
		size_t offset = ChildListElement<I, decltype(children)>::Type::template GetOffsetOf<TData>();

		if (offset != 0)
		{
			constexpr size_t elemOff = ChildListGetter<I>::template GetOffset<decltype(children)>();

			return offset + elemOff + offsetof(ParentNode, children);
		}

		return LoopChildren<TData, I + 1>();
	}

	template<typename TData>
	inline static constexpr size_t GetOffsetOfNode()
	{
		return LoopChildrenNode<TData>();
	}

	template<typename TData, size_t I = 0>
	inline static constexpr std::enable_if_t<I == sizeof...(TChildren), size_t> LoopChildrenNode()
	{
		return 0;
	}

	template<typename TData, size_t I = 0>
	inline static constexpr std::enable_if_t<I != sizeof...(TChildren), size_t> LoopChildrenNode()
	{
		size_t offset = ChildListElement<I, decltype(children)>::Type::template GetOffsetOfNode<TData>();

		if (offset != 0)
		{
			constexpr size_t elemOff = ChildListGetter<I>::template GetOffset<decltype(children)>();

			return offset + elemOff + offsetof(ParentNode, children);
		}

		return LoopChildrenNode<TData, I + 1>();
	}

	virtual bool Parse(SyncParseState& state) final override
	{
		if (shouldRead(state, TIds::GetIds()))
		{
			Foreacher<decltype(children)>::for_each_in_tuple(children, [&](auto& child)
			{
				child.Parse(state);
			});
		}

		return true;
	}

	virtual bool Unparse(SyncUnparseState& state) final override
	{
		bool should = false;

		// TODO: back out writes if we literally shouldn't have?
		if (shouldWrite(state, TIds::GetIds()))
		{
			Foreacher<decltype(children)>::for_each_in_tuple(children, [&](auto& child)
			{
				bool thisShould = child.Unparse(state);

				should = should || thisShould;
			});
		}

		return should;
	}

	virtual bool Visit(const SyncTreeVisitor& visitor) final override
	{
		visitor(*this);

		Foreacher<decltype(children)>::for_each_in_tuple(children, [&](auto& child)
		{
			child.Visit(visitor);
		});

		return true;
	}

	virtual bool IsAdditional() override
	{
		return (std::get<2>(TIds::GetIds()) & 1);
	}
};

template<typename TIds, typename TNode, typename = void>
struct NodeWrapper : public NodeBase
{
	std::array<uint8_t, 1024> data;
	uint32_t length;

	TNode node;

	NodeWrapper()
		: length(0)
	{
		ackedPlayers.set();
	}
	
	template<typename TData>
	inline static constexpr size_t GetOffsetOf()
	{
		if constexpr (std::is_same_v<TNode, TData>)
		{
			return offsetof(NodeWrapper, node);
		}

		return 0;
	}

	template<typename TData>
	inline static constexpr size_t GetOffsetOfNode()
	{
		if constexpr (std::is_same_v<TNode, TData>)
		{
			return offsetof(NodeWrapper, ackedPlayers);
		}

		return 0;
	}

	virtual bool Parse(SyncParseState& state) final override
	{
		/*auto isWrite = state.buffer.ReadBit();

		if (!isWrite)
		{
			return true;
		}*/

		auto curBit = state.buffer.GetCurrentBit();

		if (shouldRead(state, TIds::GetIds()))
		{
			// read into data array
			auto length = state.buffer.Read<uint32_t>(13);
			auto endBit = state.buffer.GetCurrentBit();

			auto leftoverLength = length;

			auto oldData = data;

			this->length = leftoverLength;
			state.buffer.ReadBits(data.data(), std::min(uint32_t(data.size() * 8), leftoverLength));

			// hac
			timestamp = state.timestamp;

			state.buffer.SetCurrentBit(endBit);

			// parse
			node.Parse(state);

			//if (memcmp(oldData.data(), data.data(), data.size()) != 0)
			{
				//trace("resetting acks on node %s\n", boost::typeindex::type_id<TNode>().pretty_name());
				frameIndex = state.frameIndex;

				if (frameIndex > state.entity->lastFrameIndex)
				{
					state.entity->lastFrameIndex = frameIndex;
				}

				ackedPlayers.reset();
			}

			state.buffer.SetCurrentBit(endBit + length);
		}

		return true;
	}

	virtual bool Unparse(SyncUnparseState& state) final override
	{
		bool hasData = (length > 0);

		// do we even want to write?
		bool couldWrite = false;

		// we can only write if we have data
		if (hasData)
		{
			// if creating, ignore acks
			if (state.syncType == 1)
			{
				couldWrite = true;
			}
			// otherwise, we only want to write if the player hasn't acked
			else if (frameIndex > state.lastFrameIndex)
			{
				couldWrite = true;
			}
		}

		// enable this for boundary checks
		//state.buffer.Write(8, 0x5A);

		if (state.timestamp && state.timestamp != timestamp)
		{
			couldWrite = false;
		}
		
		if (shouldWrite(state, TIds::GetIds(), couldWrite))
		{
			state.buffer.WriteBits(data.data(), length);

			return true;
		}

		return false;
	}

	virtual bool Visit(const SyncTreeVisitor& visitor) final override
	{
		visitor(*this);

		return true;
	}

	virtual bool IsAdditional() override
	{
		return (std::get<2>(TIds::GetIds()) & 1);
	}
};

struct CVehicleCreationDataNode
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
	bool m_unk5;

	bool Parse(SyncParseState& state)
	{
		uint32_t model = state.buffer.Read<uint32_t>(32);
		m_model = model;

		uint8_t popType = state.buffer.Read<uint8_t>(4);
		m_popType = (ePopType)popType;

		m_randomSeed = state.buffer.Read<int>(16);

		if (m_popType - 6 <= 1)
		{
			// false
			m_carBudget = state.buffer.ReadBit();
		}

		// 1000
		m_maxHealth = state.buffer.Read<int>(19);

		// 0
		m_vehicleStatus = state.buffer.Read<int>(3);

		// [timestamp]
		m_creationToken = state.buffer.Read<uint32_t>(32);

		// false, false, false
		m_needsToBeHotwired = state.buffer.ReadBit();
		m_tyresDontBurst = state.buffer.ReadBit();
		m_unk5 = state.buffer.ReadBit();

		return true;
	}

	bool Unparse(rl::MessageBuffer& buffer)
	{
		buffer.Write<uint32_t>(32, m_model);
		buffer.Write<uint8_t>(4, (uint8_t)m_popType);

		buffer.Write<int>(16, m_randomSeed);

		if (m_popType - 6 <= 1)
		{
			buffer.WriteBit(m_carBudget);
		}

		// 1000
		buffer.Write<int>(19, m_maxHealth);

		// 0
		buffer.Write<int>(3, m_vehicleStatus);

		// [timestamp]
		buffer.Write<uint32_t>(32, m_creationToken);

		// false, false, false
		buffer.WriteBit(m_needsToBeHotwired);
		buffer.WriteBit(m_tyresDontBurst);
		buffer.WriteBit(m_unk5);

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

	bool Unparse(rl::MessageBuffer& buffer)
	{
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

struct CGlobalFlagsDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CDynamicEntityGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CVehicleGameStateDataNode
{
	CVehicleGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		int radioStation = state.buffer.Read<int>(6);
		data.radioStation = radioStation;

		bool unk1 = state.buffer.ReadBit();

		int isEngineOn = state.buffer.ReadBit();
		data.isEngineOn = isEngineOn;

		int isEngineStarting = state.buffer.ReadBit();
		data.isEngineStarting = isEngineStarting;

		bool unk4 = state.buffer.ReadBit();

		int handbrake = state.buffer.ReadBit();
		data.handbrake = handbrake;

		bool unk6 = state.buffer.ReadBit();
		bool unk7 = state.buffer.ReadBit();
		int unk8 = state.buffer.ReadBit();

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
			int noLongerNeeded = state.buffer.ReadBit();
			data.noLongerNeeded = noLongerNeeded;
			bool unk28 = state.buffer.ReadBit();
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
			data.noLongerNeeded = 0;
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

	bool Unparse(rl::MessageBuffer& buffer)
	{
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

struct CVehicleAppearanceDataNode {
	CVehicleAppearanceNodeData data;

	bool Parse(SyncParseState& state)
	{
		int primaryColour = state.buffer.Read<int>(8);
		data.primaryColour = primaryColour;

		int secondaryColour = state.buffer.Read<int>(8);
		data.secondaryColour = secondaryColour;

		int pearlColour = state.buffer.Read<int>(8);
		data.pearlColour = pearlColour;

		int wheelColour = state.buffer.Read<int>(8);
		data.wheelColour = wheelColour;

		int interiorColour = state.buffer.Read<int>(8);
		data.interiorColour = interiorColour;

		int dashboardColour = state.buffer.Read<int>(8);
		data.dashboardColour = dashboardColour;

		int isPrimaryColourRGB = state.buffer.ReadBit();
		data.isPrimaryColourRGB = isPrimaryColourRGB;

		if (isPrimaryColourRGB)
		{
			int primaryRedColour = state.buffer.Read<int>(8);
			int primaryGreenColour = state.buffer.Read<int>(8);
			int primaryBlueColour = state.buffer.Read<int>(8);

			data.primaryRedColour = primaryRedColour;
			data.primaryGreenColour = primaryGreenColour;
			data.primaryBlueColour = primaryBlueColour;
		}

		int isSecondaryColourRGB = state.buffer.ReadBit();
		data.isSecondaryColourRGB = isSecondaryColourRGB;

		if (isSecondaryColourRGB)
		{
			int secondaryRedColour = state.buffer.Read<int>(8);
			int secondaryGreenColour = state.buffer.Read<int>(8);
			int secondaryBlueColour = state.buffer.Read<int>(8);

			data.secondaryRedColour = secondaryRedColour;
			data.secondaryGreenColour = secondaryGreenColour;
			data.secondaryBlueColour = secondaryBlueColour;
		}

		int unk0 = state.buffer.Read<int>(8);
		bool unk1 = state.buffer.ReadBit();

		if (unk1)
		{
			int dirtLevel = state.buffer.Read<int>(5);
			data.dirtLevel = dirtLevel;

			int extras = state.buffer.Read<int>(16);
			data.extras = extras;
			
			bool hasCustomLivery = state.buffer.ReadBit();

			if (hasCustomLivery)
			{
				int liveryIndex = state.buffer.Read<int>(5);
				data.liveryIndex = liveryIndex;
			}
			else
			{
				data.liveryIndex = -1;
			}

			bool hasCustomRoofLivery = state.buffer.ReadBit();

			if (hasCustomRoofLivery)
			{
				int roofLiveryIndex = state.buffer.Read<int>(5);
				data.roofLiveryIndex = roofLiveryIndex;
			}
			else
			{
				data.roofLiveryIndex = -1;
			}
		}
		else
		{
			data.dirtLevel = 1;
			data.liveryIndex = -1;
			data.roofLiveryIndex = -1;
			data.extras = 0;
		}

		int hasCustom = state.buffer.Read<int>(2);

		if (hasCustom)
		{
			int v5 = 0;
			do
			{
				bool hasMod = state.buffer.ReadBit();

				if (hasMod)
				{
					int modIndex = state.buffer.Read<int>(32);
				}
				++v5;
			} while (v5 < 0xD);

			bool unk3 = state.buffer.ReadBit();

			if (unk3)
			{
				int unk4 = state.buffer.Read<int>(6);
			}

			int wheelChoice = state.buffer.Read<int>(8);
			data.wheelChoice = wheelChoice;

			int wheelType = state.buffer.Read<int>(4);
			data.wheelType = wheelType;

			bool unk7 = state.buffer.ReadBit();

			if (unk7)
			{
				int unk8 = state.buffer.Read<int>(8);
			}

			bool hasCustomTires = state.buffer.ReadBit();
			data.hasCustomTires = hasCustomTires;

			bool unk10 = state.buffer.ReadBit();
		}
		else
		{
			data.hasCustomTires = false;
			data.wheelChoice = 0;
			data.wheelType = 0;
		}

		bool hasWindowTint = state.buffer.ReadBit();

		if (hasWindowTint)
		{
			int windowTintIndex = state.buffer.Read<int>(8);
			data.windowTintIndex = windowTintIndex;
		}
		else
		{
			data.windowTintIndex = -1;
		}

		bool hasTyreSmokeColours = state.buffer.ReadBit();

		if (hasTyreSmokeColours)
		{
			int tyreSmokeRedColour = state.buffer.Read<int>(8);
			int tyreSmokeGreenColour = state.buffer.Read<int>(8);
			int tyreSmokeBlueColour = state.buffer.Read<int>(8);

			data.tyreSmokeRedColour = tyreSmokeRedColour;
			data.tyreSmokeGreenColour = tyreSmokeGreenColour;
			data.tyreSmokeBlueColour = tyreSmokeBlueColour;
		}
		else
		{
			data.tyreSmokeRedColour = 255;
			data.tyreSmokeGreenColour = 255;
			data.tyreSmokeBlueColour = 255;
		}

		bool hasPlate = state.buffer.ReadBit();

		for (int i = 0; i < 8; i++)
		{
			if (hasPlate)
			{
				int plateChar = state.buffer.Read<int>(7);
				data.plate[i] = plateChar;
			}
			else
			{
				data.plate[i] = ' ';
			}
		}

		int numberPlateTextIndex = state.buffer.Read<int>(32);
		data.numberPlateTextIndex = numberPlateTextIndex;

		int unk20 = state.buffer.Read<int>(32);
		bool hasEmblems = state.buffer.ReadBit();

		if (hasEmblems)
		{
			// Crew emblems stuff
			bool unk22 = state.buffer.ReadBit();

			if (!unk22)
			{
				// TODO
			}
			else
			{
				bool unk27 = state.buffer.ReadBit();
				bool unk28 = state.buffer.ReadBit();
				bool unk29 = state.buffer.ReadBit();

				bool unk30 = state.buffer.ReadBit();
			}

			int v15 = 0;
			do
			{
				// TODO
				++v15;
			} while(v15 < 4);
		}

		/*
		bool hasNeonLights = state.buffer.ReadBit();

		if (hasNeonLights)
		{
			int neonRedColour = state.buffer.Read<int>(8);
			int neonGreenColour = state.buffer.Read<int>(8);
			int neonBlueColour = state.buffer.Read<int>(8);

			bool leftNeonEnabled = state.buffer.ReadBit();
			bool rightNeonEnabled = state.buffer.ReadBit();
			bool frontNeonEnabled = state.buffer.ReadBit();
			bool rearNeonEnabled = state.buffer.ReadBit();
		}
		*/

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

		bool unk16 = state.buffer.ReadBit();

		if (!unk16)
		{
			auto unk17 = state.buffer.ReadSigned<int>(19);
		}

		bool unk18 = state.buffer.ReadBit();

		if (unk18)
		{
			auto unk19 = state.buffer.ReadBit();
			int lastDamageSource = state.buffer.Read<int>(32);
		}

		int unk21 = state.buffer.Read<int>(4);
		int totalRepairs = state.buffer.Read<int>(4); // maximum 15
		auto unk23 = state.buffer.ReadBit();

		if (unk23)
		{
			int unk24 = state.buffer.Read<int>(64);
		}

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

	bool Unparse(rl::MessageBuffer& buffer)
	{
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

	bool Unparse(rl::MessageBuffer& buffer)
	{
		buffer.WriteFloat(12, 54.0f, m_posX);
		buffer.WriteFloat(12, 54.0f, m_posY);
		buffer.WriteFloat(12, 69.0f, m_posZ);

		return true;
	}
};

struct CPedCreationDataNode
{
	uint32_t m_model;
	ePopType m_popType;

	bool Parse(SyncParseState& state)
	{
		auto isRespawnObjectId = state.buffer.ReadBit();
		auto respawnFlaggedForRemoval = state.buffer.ReadBit();

		auto popType = state.buffer.Read<int>(4);
		uint32_t model = state.buffer.Read<uint32_t>(32);
		m_model = model;

		m_popType = (ePopType)popType;

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
	CPedGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		auto bool1 = state.buffer.ReadBit();
		auto bool2 = state.buffer.ReadBit();
		auto bool3 = state.buffer.ReadBit();
		auto bool4 = state.buffer.ReadBit();
		auto bool5 = state.buffer.ReadBit();
		auto bool6 = state.buffer.ReadBit();

		if (Is2060())
		{
			state.buffer.ReadBit();
			state.buffer.ReadBit();
		}

		auto arrestState = state.buffer.Read<int>(1);
		auto deathState = state.buffer.Read<int>(2);

		auto hasWeapon = state.buffer.ReadBit();
		int weapon = 0;

		if (hasWeapon)
		{
			weapon = state.buffer.Read<int>(32);
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

		// TODO

		return true;
	}
};

struct CEntityOrientationDataNode
{
	CEntityOrientationNodeData data;

	bool Parse(SyncParseState& state)
	{
#if 0
		auto rotX = state.buffer.ReadSigned<int>(9) * 0.015625f;
		auto rotY = state.buffer.ReadSigned<int>(9) * 0.015625f;
		auto rotZ = state.buffer.ReadSigned<int>(9) * 0.015625f;

		data.rotX = rotX;
		data.rotY = rotY;
		data.rotZ = rotZ;
#else
		data.quat.largest = state.buffer.Read<uint32_t>(2);
		data.quat.integer_a = state.buffer.Read<uint32_t>(11);
		data.quat.integer_b = state.buffer.Read<uint32_t>(11);
		data.quat.integer_c = state.buffer.Read<uint32_t>(11);
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

struct CObjectCreationDataNode
{
	uint32_t m_model;

	bool Parse(SyncParseState& state)
	{
		/*
			Probably a subsystem ID
			If it's 0 or 2, it's a dummy object
		*/
		int createdBy = state.buffer.Read<int>(5);
		if (createdBy != 0 && createdBy != 2)
		{
			uint32_t model = state.buffer.Read<uint32_t>(32);
			m_model = model;

			bool hasInitPhysics = state.buffer.ReadBit();
			bool scriptGrabbedFromWorld = state.buffer.ReadBit();
			bool noReassign = state.buffer.ReadBit();

			if (scriptGrabbedFromWorld)
			{
				float scriptGrabPosX = state.buffer.ReadSignedFloat(19, 27648.0f);
				float scriptGrabPosY = state.buffer.ReadSignedFloat(19, 27648.0f);
				float scriptGrabPosZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;

				auto scriptGrabRadius = state.buffer.ReadFloat(8, 20); // wrong divisor
			}
		}
		else
		{
			float dummyPosX = state.buffer.ReadSignedFloat(31, 27648.0f);
			float dummyPosY = state.buffer.ReadSignedFloat(31, 27648.0f);
			float dummyPosZ = state.buffer.ReadFloat(31, 4416.0f) - 1700.0f;

			auto playerWantsControl = state.buffer.ReadBit();
			auto unk9 = state.buffer.ReadBit();
			auto unk10 = state.buffer.ReadBit();
			auto unk11 = state.buffer.ReadBit();
			auto unk12 = state.buffer.ReadBit();
			auto unk13 = state.buffer.ReadBit();
			auto unk14 = state.buffer.ReadBit();

			if (unk9)
			{
				auto fragGroupIndex = state.buffer.Read<int>(5);
			}

			auto unk16 = state.buffer.ReadBit();

			if (!unk16)
			{
				auto ownershipToken = state.buffer.Read<int>(10);
				float objectPosX = state.buffer.ReadSignedFloat(19, 27648.0f);
				float objectPosY = state.buffer.ReadSignedFloat(19, 27648.0f);
				float objectPosZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;
				
				auto rotX = state.buffer.ReadSigned<int>(9) * 0.015625f;
				auto rotY = state.buffer.ReadSigned<int>(9) * 0.015625f;
				auto rotZ = state.buffer.ReadSigned<int>(9) * 0.015625f;
			}
		}

		bool unk20 = state.buffer.ReadBit();

		if (unk20)
		{
			auto unk21 = state.buffer.ReadBit();
		}

		bool unk22 = state.buffer.ReadBit();

		if (unk22)
		{
			auto unk23 = state.buffer.Read<int>(16);
		}

		bool unk24 = state.buffer.ReadBit();

		return true;
	}
};

struct CObjectGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CObjectScriptGameStateDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPhysicalHealthDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CObjectSectorPosNode
{
	float m_sectorPosX;
	float m_sectorPosY;
	float m_sectorPosZ;

	bool Parse(SyncParseState& state)
	{
		bool highRes = state.buffer.ReadBit();

		int bits = (highRes) ? 20 : 12;

		auto posX = state.buffer.ReadFloat(bits, 54.0f);
		auto posY = state.buffer.ReadFloat(bits, 54.0f);
		auto posZ = state.buffer.ReadFloat(bits, 69.0f);

		m_sectorPosX = posX;
		m_sectorPosY = posY;
		m_sectorPosZ = posZ;

		state.entity->syncTree->CalculatePosition();

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
		bool isFine = state.buffer.ReadBit();
		auto maxHealthChanged = state.buffer.ReadBit();

		int maxHealth = 200;

		if (maxHealthChanged)
		{
			maxHealth = state.buffer.Read<int>(13);
		}

		data.maxHealth = maxHealth;

		if (!isFine)
		{
			int pedHealth = state.buffer.Read<int>(13);
			auto unk4 = state.buffer.ReadBit();
			auto unk5 = state.buffer.ReadBit();

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

			if (hasUnk1)
			{
				state.buffer.Read<int>(13);
			}

			if (!hasUnk2)
			{
				state.buffer.Read<int>(13);
			}
		}


		auto unk8 = state.buffer.ReadBit();

		if (unk8) // unk9 != 0
		{
			// object ID
			auto unk9 = state.buffer.Read<short>(13);
		}

		int causeOfDeath = state.buffer.Read<int>(32);
		data.causeOfDeath = causeOfDeath;

		int injuredStatus = state.buffer.Read<int>(2); // Change below 150 HP, injured data?

		auto unk13 = state.buffer.ReadBit();

		if (unk13)
		{
			int unk14 = state.buffer.Read<int>(8);
		}

		return true;
	}
};

struct CPedMovementGroupDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedAIDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedAppearanceDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CPedOrientationDataNode
{
	CPedOrientationNodeData data;

	bool Parse(SyncParseState& state)
	{
		auto currentHeading = state.buffer.ReadSignedFloat(8, 6.28318548f);
		auto desiredHeading = state.buffer.ReadSignedFloat(8, 6.28318548f);

		data.currentHeading = currentHeading;
		data.desiredHeading = desiredHeading;

		return true;
	}
};

struct CPedMovementDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedTaskTreeDataNode { bool Parse(SyncParseState& state) { return true; } };
struct CPedTaskSpecificDataNode { bool Parse(SyncParseState& state) { return true; } };

struct CPedSectorPosMapNode
{
	float m_sectorPosX;
	float m_sectorPosY;
	float m_sectorPosZ;

	bool Parse(SyncParseState& state)
	{
		auto posX = state.buffer.ReadFloat(12, 54.0f);
		auto posY = state.buffer.ReadFloat(12, 54.0f);
		auto posZ = state.buffer.ReadFloat(12, 69.0f);

		m_sectorPosX = posX;
		m_sectorPosY = posY;
		m_sectorPosZ = posZ;

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

struct CPlayerGameStateDataNode {
	CPlayerGameStateNodeData data;

	bool Parse(SyncParseState& state)
	{
		int playerState = state.buffer.Read<int>(3);
		auto controlsDisabledByScript = state.buffer.ReadBit(); // SET_PLAYER_CONTROL
		int playerTeam = state.buffer.Read<int>(6);
		data.playerTeam = playerTeam;
		int mobileRingState = state.buffer.Read<int>(8);

		auto isAirDragMultiplierDefault = state.buffer.ReadBit();

		if (!isAirDragMultiplierDefault)
		{
			float airDragMultiplier = state.buffer.ReadFloat(7, 1.0f);
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
		auto unk14 = state.buffer.ReadBit();
		auto unk15 = state.buffer.ReadBit();
		auto unk16 = state.buffer.ReadBit();
		auto unk17 = state.buffer.ReadBit();
		auto unk18 = state.buffer.ReadBit();
		auto unk19 = state.buffer.ReadBit();
		auto unk20 = state.buffer.ReadBit();
		auto unk21 = state.buffer.ReadBit();
		auto unk22 = state.buffer.ReadBit();

		if (Is2060())
		{
			state.buffer.ReadBit();
		}

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
		auto unk70 = state.buffer.ReadBit();

		// #TODO2060: maybe extra read near here? list is weird

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
		auto unk79 = state.buffer.ReadBit();
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

		float weaponDefenseModifier = state.buffer.ReadFloat(8, 4.04761615646f);
		float weaponDefenseModifier2 = state.buffer.ReadFloat(8, 4.04761615646f);

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
		auto unk102 = state.buffer.ReadBit();
		auto unk103 = state.buffer.ReadBit();
		auto unk104 = state.buffer.ReadBit();
		auto unk105 = state.buffer.ReadBit();
		auto unk106 = state.buffer.ReadBit();

		if (unk106)
		{
			auto unk107 = state.buffer.ReadBit();
			int unk108 = state.buffer.Read<int>(2);
		}

		float unk109 = state.buffer.ReadFloat(8, 10.20002244f);
		auto isWeaponDamageModifierSet = state.buffer.ReadBit();

		if (isWeaponDamageModifierSet)
		{
			float weaponDamageModifier = state.buffer.ReadFloat(10, 25.5747934835f);
			data.weaponDamageModifier = weaponDamageModifier;
		}
		else
		{
			data.weaponDamageModifier = 1.0f;
		}

		auto unk112 = state.buffer.ReadBit();

		if (unk112)
		{
			float unk113 = state.buffer.ReadFloat(10, 25.5747934835f);
		}
		else
		{
			// 1.0
		}

		auto isMeleeWeaponDamageModifierSet = state.buffer.ReadBit();

		if (isMeleeWeaponDamageModifierSet)
		{
			float meleeWeaponDamageModifier = state.buffer.ReadFloat(10, 25.5747934835f);
			data.meleeWeaponDamageModifier = meleeWeaponDamageModifier;
		}
		else
		{
			data.meleeWeaponDamageModifier = 1.0f;
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

		return true;
	}
};

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
	float m_sectorPosX;
	float m_sectorPosY;
	float m_sectorPosZ;

	int isStandingOn;

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
				state.buffer.ReadFloat(14, 16.0f); // Standing On Local Offset X
				state.buffer.ReadFloat(14, 16.0f); // Standing On Local Offset Y
				state.buffer.ReadFloat(10, 4.0f); // Standing On Local Offset Z
			}

			isStandingOn = isStandingOn;
		}

		auto posX = state.buffer.ReadFloat(12, 54.0f);
		auto posY = state.buffer.ReadFloat(12, 54.0f);
		auto posZ = state.buffer.ReadFloat(12, 69.0f);

		m_sectorPosX = posX;
		m_sectorPosY = posY;
		m_sectorPosZ = posZ;

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
		auto unk1 = state.buffer.Read<int>(3);
		auto unk2 = state.buffer.ReadBit();
		auto unk3 = state.buffer.ReadBit();
		auto isWanted = state.buffer.ReadBit();

		if (isWanted) {
			// These coordinates need more exploration.
			// But I think these coordinates rely to where crime was started or last position seen of player by cops
			auto posX = state.buffer.ReadSignedFloat(19, 27648.0f);
			auto posY = state.buffer.ReadSignedFloat(19, 27648.0f);
			auto posZ = state.buffer.ReadFloat(19, 4416.0f) - 1700.0f;
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
		else if (data.timeInPursuit != -1) {
			data.timeInPrevPursuit = data.timeInPursuit;
			data.timeInPursuit = -1;
		}

		auto unk4 = state.buffer.ReadBit();
		auto copsCantSeePlayer = state.buffer.ReadBit();
		auto isEvading = state.buffer.ReadBit();
		data.isEvading = isEvading;
		return true;
	}
};

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
		auto [hasPspdn, playerSecPosDataNode] = GetData<CPlayerSectorPosNode>();
		auto [hasOspdn, objectSecPosDataNode] = GetData<CObjectSectorPosNode>();
		auto [hasPspmdn, pedSecPosMapDataNode] = GetData<CPedSectorPosMapNode>();

		auto sectorX = (hasSdn) ? secDataNode->m_sectorX : 512;
		auto sectorY = (hasSdn) ? secDataNode->m_sectorY : 512;
		auto sectorZ = (hasSdn) ? secDataNode->m_sectorZ : 0;

		auto sectorPosX =
			(hasSpdn) ? secPosDataNode->m_posX :
				(hasPspdn) ? playerSecPosDataNode->m_sectorPosX :
					(hasOspdn) ? objectSecPosDataNode->m_sectorPosX :
						(hasPspmdn) ? pedSecPosMapDataNode->m_sectorPosX :
							0.0f;

		auto sectorPosY =
			(hasSpdn) ? secPosDataNode->m_posY :
				(hasPspdn) ? playerSecPosDataNode->m_sectorPosY :
					(hasOspdn) ? objectSecPosDataNode->m_sectorPosY :
						(hasPspmdn) ? pedSecPosMapDataNode->m_sectorPosY :
							0.0f;

		auto sectorPosZ =
			(hasSpdn) ? secPosDataNode->m_posZ :
				(hasPspdn) ? playerSecPosDataNode->m_sectorPosZ :
					(hasOspdn) ? objectSecPosDataNode->m_sectorPosZ :
						(hasPspmdn) ? pedSecPosMapDataNode->m_sectorPosZ :
							0.0f;

		posOut[0] = ((sectorX - 512.0f) * 54.0f) + sectorPosX;
		posOut[1] = ((sectorY - 512.0f) * 54.0f) + sectorPosY;
		posOut[2] = ((sectorZ * 69.0f) + sectorPosZ) - 1700.0f;
	}

	virtual CPlayerCameraNodeData* GetPlayerCamera() override
	{
		auto [hasCdn, cameraNode] = GetData<CPlayerCameraDataNode>();

		return (hasCdn) ? &cameraNode->data : nullptr;
	}

	virtual CPlayerWantedAndLOSNodeData* GetPlayerWantedAndLOS() override 
	{
		auto [hasNode, node] = GetData<CPlayerWantedAndLOSDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CPedGameStateNodeData* GetPedGameState() override
	{
		auto[hasPdn, pedNode] = GetData<CPedGameStateDataNode>();

		return (hasPdn) ? &pedNode->data : nullptr;
	}

	virtual uint64_t GetPedGameStateFrameIndex() override
	{
		auto pedBase = GetNode<CPedGameStateDataNode>();

		return (pedBase) ? pedBase->frameIndex : 0;
	}

	virtual CVehicleGameStateNodeData* GetVehicleGameState() override
	{
		auto[hasVdn, vehNode] = GetData<CVehicleGameStateDataNode>();

		return (hasVdn) ? &vehNode->data : nullptr;
	}

	virtual CPlayerGameStateNodeData* GetPlayerGameState() override
	{
		auto [hasNode, node] = GetData<CPlayerGameStateDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CVehicleAppearanceNodeData* GetVehicleAppearance() override
	{
		auto [hasNode, node] = GetData<CVehicleAppearanceDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CPedHealthNodeData* GetPedHealth() override
	{
		auto [hasNode, node] = GetData<CPedHealthDataNode>();

		return (hasNode) ? &node->data : nullptr;
	}

	virtual CVehicleHealthNodeData* GetVehicleHealth() override
	{
		auto [hasNode, node] = GetData<CVehicleHealthDataNode>();

		return (hasNode) ? &node->data : nullptr;
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

	virtual void CalculatePosition() override
	{
		// TODO: cache it?
	}

	virtual bool GetPopulationType(ePopType* popType) override
	{
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

		return false;
	}

	virtual bool GetModelHash(uint32_t* modelHash) override
	{
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

		return root.Unparse(state);
	}

	virtual void Visit(const SyncTreeVisitor& visitor) final override
	{
		std::unique_lock<std::mutex> lock(mutex);

		root.Visit(visitor);
	}
};

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
		ParentNode<
			NodeIds<127, 127, 0>, 
			NodeWrapper<NodeIds<127, 127, 0>, CGlobalFlagsDataNode>, 
			NodeWrapper<NodeIds<127, 127, 1>, CDoorScriptInfoDataNode>, 
			NodeWrapper<NodeIds<127, 127, 1>, CDoorScriptGameStateDataNode>
		>, 
		NodeWrapper<NodeIds<86, 86, 0>, CDoorMovementDataNode>, 
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
using CTrainSyncTree = SyncTree<
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
					NodeWrapper<NodeIds<127, 127, 0>, CTrainGameStateDataNode>
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
