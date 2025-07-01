#pragma once

namespace fx::sync
{
template<int Id1, int Id2, int Id3, bool CanSendOnFirst = true>
struct NodeIds
{
	inline static constexpr std::tuple<int, int, int> GetIds()
	{
		return { Id1, Id2, Id3 };
	}

	inline static constexpr bool CanSendOnFirstUpdate()
	{
		return CanSendOnFirst;
	}
};

template<int syncType, int objType, typename Ids>
inline bool shouldRead(SyncParseState& state)
{
	constexpr auto ids = Ids::GetIds();

	if constexpr ((std::get<0>(ids) & syncType) == 0)
	{
		return false;
	}

	// because we hardcode this sync type to 0 (mA0), we can assume it's not used
	if constexpr (std::get<2>(ids) && !(objType & std::get<2>(ids)))
	{
		return false;
	}

	if constexpr ((std::get<1>(ids) & syncType) != 0)
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

private:
	template<int syncType, int objType, typename TChild>
	inline static auto ParseChild(TChild& child, SyncParseState& state) -> decltype(child.Parse(state))
	{
		return child.Parse(state);
	}

	template<int syncType, int objType, typename TChild>
	inline static auto ParseChild(TChild& child, SyncParseState& state) -> decltype(child.template Parse<syncType, objType>(state))
	{
		return child.template Parse<syncType, objType>(state);
	}

public:
	template<int syncType, int objType>
	bool Parse(SyncParseState& state)
	{
		if (shouldRead<syncType, objType, TIds>(state))
		{
			Foreacher<decltype(children)>::for_each_in_tuple(children, [&state](auto& child)
			{
				ParseChild<syncType, objType>(child, state);
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

template<typename TIds, typename TNode, size_t Length = 1024, typename = void>
struct NodeWrapper : public NodeBase
{
	uint32_t length;

	TNode node;

	// this **HAS** to be the last field as some callers may not be aware of the correct Length template argument
	eastl::fixed_vector<uint8_t, Length> data;

	NodeWrapper()
		: data(Length), length(0)
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

	// template disambiguation hack: `int` will be preferred to `char`
	// if the `int` overload fails (as Parse() is not available), it'll use `char`.
	//
	// other than that, this is plain SFINAE
	template<typename TNode2>
	static constexpr auto ShouldParseNode(char)
	{
		return false;
	}

	template<typename TNode2>
	static constexpr auto ShouldParseNode(int) -> decltype(std::declval<TNode2>().Parse(std::declval<SyncParseState&>()))
	{
		return true;
	}

	template<int syncType, int objType>
	bool Parse(SyncParseState& state)
	{
		if (shouldRead<syncType, objType, TIds>(state))
		{
			// read into data array
			auto length = state.buffer.Read<uint32_t>(13);
			uint32_t endBit = 0;

			if constexpr (ShouldParseNode<TNode>(0))
			{
				endBit = state.buffer.GetCurrentBit();
			}

			auto leftoverLength = length;
			auto leftoverByteLength = std::min<size_t>(size_t(leftoverLength / 8) + 1, 1024);

			if (data.size() < leftoverByteLength)
			{
				data.resize(leftoverByteLength);
			}

			this->length = leftoverLength;
			state.buffer.ReadBits(data.data(), std::min(int(data.size() * 8), int(leftoverLength)));

			// hac
			timestamp = state.timestamp;

			// parse manual data
			if constexpr (ShouldParseNode<TNode>(0))
			{
				state.buffer.SetCurrentBit(endBit);
				node.Parse(state);

				state.buffer.SetCurrentBit(endBit + length);
			}

			frameIndex = state.frameIndex;
			state.entity->lastFrameIndex = std::max(state.entity->lastFrameIndex, frameIndex);

			ackedPlayers.reset();
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

	template<typename T>
	bool SerializeCapped(int size, int maxValue, T& data)
	{
		data = std::min(data, static_cast<T>(maxValue));

		return Serialize(size, data);
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

	template<typename T>
	bool SerializeCapped(int size, int maxValue, T& data)
	{
		data = std::min(data, static_cast<T>(maxValue));

		return Serialize(size, data);
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

template<typename TNode, bool IsRDR>
struct SyncTreeBaseImpl : SyncTreeBase
{
	TNode root;
	std::mutex mutex;

	template<typename TData>
	inline static constexpr size_t GetOffsetOf()
	{
		auto doff = TNode::template GetOffsetOf<TData>();

		return (doff) ? offsetof(SyncTreeBaseImpl, root) + doff : 0;
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

		return (doff) ? offsetof(SyncTreeBaseImpl, root) + doff : 0;
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

	virtual void ParseSync(SyncParseStateDynamic& state) final override
	{
		std::unique_lock<std::mutex> lock(mutex);

		// mA0 flag
		state.objType = state.buffer.ReadBit();

		if constexpr (IsRDR)
		{
			state.buffer.ReadBit();
		}

		if (state.objType == 1)
		{
			root.template Parse<2, 1>(state);
		}
		else
		{
			root.template Parse<2, 0>(state);
		}
	}

	virtual void ParseCreate(SyncParseStateDynamic& state) final override
	{
		if constexpr (IsRDR)
		{
			state.buffer.ReadBit();
		}

		std::unique_lock<std::mutex> lock(mutex);
		root.template Parse<1, 0>(state);
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

		if constexpr (IsRDR)
		{
			state.buffer.WriteBit(0);
		}

		return root.Unparse(state);
	}

	virtual void Visit(const SyncTreeVisitor& visitor) final override
	{
		std::unique_lock<std::mutex> lock(mutex);

		root.Visit(visitor);
	}
};
}
