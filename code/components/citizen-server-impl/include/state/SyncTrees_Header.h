#pragma once

namespace fx::sync
{
template<int Id1, int Id2, int Id3, bool CanSendOnFirst = true>
struct NodeIds
{
	inline static std::tuple<int, int, int> GetIds()
	{
		return { Id1, Id2, Id3 };
	}

	inline static bool CanSendOnFirstUpdate()
	{
		return CanSendOnFirst;
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
template<class F, class... Ts, std::size_t... Is>
void for_each_in_tuple(std::tuple<Ts...>& tuple, F func, std::index_sequence<Is...>)
{
	using expander = int[];
	(void)expander{
		0, ((void)func(std::get<Is>(tuple)), 0)...
	};
}

template<class F, class... Ts>
void for_each_in_tuple(std::tuple<Ts...>& tuple, F func)
{
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
		offset + offsetof(TList, rest));
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
		return offset + offsetof(TList, first);
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

	bool Parse(SyncParseState& state)
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

	bool Unparse(SyncUnparseState& state)
	{
		bool should = false;

		// TODO: back out writes if we didn't write any child
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

	bool Visit(const SyncTreeVisitor& visitor)
	{
		visitor(*this);

		Foreacher<decltype(children)>::for_each_in_tuple(children, [&](auto& child)
		{
			child.Visit(visitor);
		});

		return true;
	}

	bool IsAdditional()
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

	bool Parse(SyncParseState& state)
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

	bool Unparse(SyncUnparseState& state)
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

		if (state.isFirstUpdate)
		{
			if (!TIds::CanSendOnFirstUpdate())
			{
				couldWrite = false;
			}

			// if this doesn't need activation flags, don't write it
			if ((std::get<2>(TIds::GetIds()) & 1) == 0)
			{
				couldWrite = false;
			}
		}

		if (shouldWrite(state, TIds::GetIds(), couldWrite))
		{
			state.buffer.WriteBits(data.data(), length);

			return true;
		}

		return false;
	}

	bool Visit(const SyncTreeVisitor& visitor)
	{
		visitor(*this);

		return true;
	}

	bool IsAdditional()
	{
		return (std::get<2>(TIds::GetIds()) & 1);
	}
};

struct ParseSerializer
{
	inline ParseSerializer(SyncParseState* state)
		: state(state)
	{
	}

	template<typename T>
	bool Serialize(int size, T& data)
	{
		return state->buffer.Read(size, &data);
	}

	bool Serialize(bool& data)
	{
		data = state->buffer.ReadBit();
		return true;
	}

	bool Serialize(int size, float div, float& data)
	{
		data = state->buffer.ReadFloat(size, div);
		return true;
	}

	bool SerializeSigned(int size, float div, float& data)
	{
		data = state->buffer.ReadSignedFloat(size, div);
		return true;
	}

	static constexpr bool isReader = true;
	SyncParseState* state;
};

struct UnparseSerializer
{
	inline UnparseSerializer(SyncUnparseState* state)
		: state(state)
	{
	}

	template<typename T>
	bool Serialize(int size, T& data)
	{
		state->buffer.Write<T>(size, data);
		return true;
	}

	bool Serialize(bool& data)
	{
		return state->buffer.WriteBit(data);
	}

	bool Serialize(int size, float div, float& data)
	{
		state->buffer.WriteFloat(size, div, data);
		return true;
	}

	bool SerializeSigned(int size, float div, float& data)
	{
		state->buffer.WriteSignedFloat(size, div, data);
		return true;
	}

	static constexpr bool isReader = false;
	SyncUnparseState* state;
};

template<typename TNode>
struct GenericSerializeDataNode
{
	bool Parse(SyncParseState& state)
	{
		auto self = static_cast<TNode*>(this);
		auto serializer = ParseSerializer{ &state };
		return self->Serialize(serializer);
	}

	bool Unparse(SyncUnparseState& state)
	{
		auto self = static_cast<TNode*>(this);
		auto serializer = UnparseSerializer{ &state };
		return self->Serialize(serializer);
	}
};
}
