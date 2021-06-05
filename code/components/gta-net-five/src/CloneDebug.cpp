#include "StdInc.h"
#include <ConsoleHost.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_API DLL_IMPORT

#include <imgui.h>
#include <imgui_internal.h>

#include <CoreConsole.h>

#include <netBlender.h>
#include <netObjectMgr.h>
#include <CloneManager.h>
#include <netInterface.h>
#include <netSyncTree.h>

#include <EASTL/bitset.h>

#include <rlNetBuffer.h>

#include <ICoreGameInit.h>

#include <Error.h>

#include <Hooking.h>

// REDM1S: not all data from nodes is printed

inline size_t GET_NIDX(rage::netSyncTree* tree, void* node)
{
#ifdef GTA_FIVE
	return *((uint8_t*)node + 66);
#elif IS_RDR3
	return *((uint8_t*)node + 68);
#endif
}

static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
	using namespace ImGui;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

#ifdef IS_RDR3
struct LogVector
{
	float x;
	char pad1[4];
	float y;
	char pad2[4];
	float z;
	char pad3[4];
};
#endif

namespace rage
{
	class netLogStub
	{
	public:
		virtual ~netLogStub() = default;

#ifdef GTA_FIVE
		virtual void m_8() = 0;

		virtual void m_10() = 0;

		virtual void m_18() = 0;

		virtual void m_20() = 0;

		virtual void m_28() = 0;

		virtual void LogString(const char* prefix, const char* fmt, ...) = 0;

		virtual void m_38() = 0;

		virtual void m_40() = 0;
#elif IS_RDR3
		virtual void m_8(const char* prefix, uint64_t value) = 0;

		virtual void m_10(const char* prefix, uint64_t value) = 0;

		virtual void m_18(const char* prefix, uint64_t value) = 0;

		virtual void m_20(const char* prefix, uint64_t value) = 0;

		virtual void m_28(const char* prefix, LogVector* value) = 0; // log vector?

		virtual void m_30(const char* prefix, uint64_t value) = 0;

		virtual void m_38(const char* prefix, uint64_t value) = 0;

		virtual void m_40(const char* prefix, uint32_t value) = 0; // log hash

		virtual void m_48(const char* prefix, uint64_t value) = 0;

		virtual void m_50(const char* prefix, uint8_t value) = 0; // log boolean?

		virtual void LogString(const char* prefix, const char* fmt, ...) = 0;
#endif

		// ?
	};

	enum class UpdateLevel : uint32_t
	{
		VERY_LOW = 0,
		LOW = 1,
		MEDIUM = 2,
		HIGH = 3,
		VERY_HIGH = 4,
	};

	class netSyncNodeBase
	{
	public:
		virtual ~netSyncNodeBase() = 0;

		virtual bool IsDataNode() = 0;

		virtual bool IsParentNode() = 0;

#ifdef GTA_FIVE
		virtual void m_18() = 0;
		virtual void m_20() = 0;
		virtual void m_28() = 0;
		virtual void m_30() = 0;
		virtual void m_38() = 0;
		virtual void m_something() = 0; // calls calculatesize, added in a patch
		virtual void m_40() = 0;
		virtual void m_48() = 0;
		virtual void m_50() = 0;
		virtual void m_58() = 0;
		virtual uint8_t GetUpdateFrequency(UpdateLevel level) = 0;
		virtual void m_68() = 0;
		virtual void m_70() = 0;
		virtual void m_78() = 0;
		virtual void m_80() = 0;
		virtual void m_88() = 0;
		virtual void m_90() = 0;
		virtual void m_98() = 0;
		virtual void m_A0() = 0;
		virtual void WriteObject(rage::netObject* object, rage::datBitBuffer* buffer, rage::netLogStub* logger, bool readFromObject) = 0;
		virtual void m_B0() = 0;
		virtual void m_B8() = 0;
		virtual void LogNode(rage::netLogStub* stub) = 0;
		virtual void m_C8() = 0;
		virtual void m_D0() = 0;
		virtual void m_D8() = 0;
		virtual void m_E0() = 0;
		virtual void LogObject(rage::netObject* object, rage::netLogStub* stub) = 0;
#elif IS_RDR3
		virtual void m_18() = 0; // InitialiseNode
		virtual void m_20() = 0; // ShutdownNode
		virtual void m_28() = 0;
		virtual void m_30() = 0;
		virtual void m_added1311() = 0;
		virtual void m_38() = 0;
		virtual void m_something() = 0; // calls calculatesize, added in a patch
		virtual void m_40() = 0;
		virtual void m_48() = 0;
		virtual void m_50() = 0;
		virtual void m_58() = 0;
		virtual void m_60() = 0;
		virtual void m_68() = 0;
		virtual void m_70() = 0;
		virtual uint8_t GetUpdateFrequency(UpdateLevel level) = 0;
		virtual void m_80() = 0;
		virtual void m_88() = 0;
		virtual void m_90() = 0;
		virtual void m_98() = 0;
		virtual void m_A0() = 0;
		virtual void m_unk1() = 0;
		virtual void m_unk2() = 0;
		virtual void m_unk3() = 0;
		virtual void m_unk4() = 0;
		virtual void m_unk5() = 0;
		virtual void m_unk6() = 0;
		virtual void m_unk7() = 0;
		virtual void m_unk8() = 0;
		virtual void m_B0() = 0;
		virtual void WriteObject(rage::netObject* object, rage::datBitBuffer* buffer, rage::netLogStub* logger, bool readFromObject) = 0;
		virtual void m_D8() = 0;
		virtual void m_E832() = 0;
		virtual void LogNode(rage::netLogStub* stub) = 0;
		virtual void GetUsesCurrentStateBuffer() = 0;
		virtual void m_E328() = 0;
		virtual void m_E321() = 0;
		virtual void LogObject(rage::netObject* object, rage::netLogStub* stub) = 0;
#endif

		// data node

	public:
		netSyncNodeBase* nextSibling;

		uint8_t pad[24];

		uint32_t flags1;
		uint32_t flags2;
		uint32_t flags3;

		uint32_t pad2;

		netSyncNodeBase* firstChild;
	};

	class netSyncDataNodeBase : public netSyncNodeBase
	{
	public:
		uint32_t flags;
		uint32_t pad3;

#ifdef GTA_FIVE
		uint64_t pad4;
#endif

		netSyncDataNodeBase* externalDependentNodeRoot; // 0x50
		uint32_t externalDependencyCount;
		netSyncDataNodeBase* externalDependencies[8];
		uint8_t syncFrequencies[8];
		void* nodeBuffer;
	};
}

rage::netObject* g_curNetObjectSelection;
static rage::netSyncNodeBase* g_curSyncNodeSelection;

static std::string GetClassTypeName(void* ptr)
{
	std::string name;

#ifdef GTA_FIVE
	name = typeid(*(uint64_t*)ptr).name();
	name = name.substr(6);
#elif IS_RDR3
	name = fmt::sprintf("%016llx", *(uint64_t*)ptr);
#endif

	return name;
}

static void RenderSyncNode(rage::netObject* object, rage::netSyncNodeBase* node)
{
	std::string objectName = GetClassTypeName(node);

	if (node->IsParentNode())
	{
		if (ImGui::TreeNode(node, "%s", objectName.c_str()))
		{
			for (auto child = node->firstChild; child; child = child->nextSibling)
			{
				RenderSyncNode(object, child);
			}

			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::TreeNodeEx(node,
			ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((g_curSyncNodeSelection == node) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
			"%s", objectName.c_str());

		if (ImGui::IsItemClicked())
		{
			g_curNetObjectSelection = object;
			g_curSyncNodeSelection = node;
		}
	}
}

static void RenderSyncTree(rage::netObject* object, rage::netSyncTree* syncTree)
{
	RenderSyncNode(object, syncTree->syncNode);
}

static void RenderNetObjectTree()
{
	for (int player = 0; player < 128 + 1; player++)
	{
		std::vector<rage::netObject*> objects;

		CloneObjectMgr->ForAllNetObjects(player, [&objects](rage::netObject* object)
		{
			objects.push_back(object);
		});

		if (objects.size())
		{
			if (ImGui::TreeNode((void*)(intptr_t)player, "Player %d", player))
			{
				for (rage::netObject* object : objects)
				{
					try
					{
#ifdef GTA_FIVE
						std::string objectName = GetClassTypeName(object);
#elif IS_RDR3
						std::string objectName = GetNetObjEntityName(object->GetObjectType());
#endif

						if (ImGui::TreeNodeEx(object,
							ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((g_curNetObjectSelection == object) ? ImGuiTreeNodeFlags_Selected : 0),
							"%s - %d", objectName.c_str(), object->GetObjectId()))
						{
							if (ImGui::IsItemClicked())
							{
								bool isDifferent = !g_curNetObjectSelection || g_curNetObjectSelection->GetObjectType() != object->GetObjectType();

								g_curNetObjectSelection = object;

								if (isDifferent)
								{
									g_curSyncNodeSelection = nullptr; // in case the current sync node doesn't exist e.g. for a different sync tree
								}
							}

							RenderSyncTree(object, object->GetSyncTree());

							ImGui::TreePop();
						}
					}
					catch (std::exception& e)
					{

					}
				}

				ImGui::TreePop();
			}
		}
	}
}

using TSyncLog = std::unordered_map<rage::netSyncNodeBase*, std::vector<std::string>>;

static std::unordered_map<int, TSyncLog> syncLog;

class SyncLogger : public rage::netLogStub
{
public:
	SyncLogger(const std::function<void(const std::string&)>& logger)
		: m_logger(logger)
	{

	}

	virtual ~SyncLogger()
	{

	}

#ifdef GTA_FIVE
	virtual void m_8() { }

	virtual void m_10() { }

	virtual void m_18() { }

	virtual void m_20() { }

	virtual void m_28() { }

	virtual void LogString(const char* prefix, const char* fmt, ...)
	{
		char buf[512];

		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		m_logger(fmt::sprintf("%s%s%s", prefix ? prefix : "", prefix ? ": " : "", buf));
	}

	virtual void m_38() { }

	virtual void m_40() { }
#elif IS_RDR3
	virtual void m_8(const char* prefix, uint64_t value)
	{
		trace("m_8 %d\n", value);
	}

	virtual void m_10(const char* prefix, uint64_t value)
	{
		trace("m_10 %d\n", value);
	}

	virtual void m_18(const char* prefix, uint64_t value)
	{
		trace("m_18 %d\n", value);
	}

	virtual void m_20(const char* prefix, uint64_t value)
	{
		trace("m_20 %d\n", value);
	}

	virtual void m_28(const char* prefix, LogVector* value)
	{
		m_logger(fmt::sprintf("%s%s%f %f %f", prefix ? prefix : "", prefix ? ": " : "", value->x, value->y, value->z));
	}

	virtual void m_30(const char* prefix, uint64_t value)
	{
		trace("m_30 %d\n", value);
	}

	virtual void m_38(const char* prefix, uint64_t value)
	{
		trace("m_38 %d\n", value);
	}

	virtual void m_40(const char* prefix, uint32_t value)
	{
		m_logger(fmt::sprintf("%s%s%d", prefix ? prefix : "", prefix ? ": " : "", value));
	}

	virtual void m_48(const char* prefix, uint64_t value)
	{
		m_logger(fmt::sprintf("%s%s%d", prefix ? prefix : "", prefix ? ": " : "", value));
	}

	virtual void m_50(const char* prefix, uint8_t value)
	{
		m_logger(fmt::sprintf("%s%s%d", prefix ? prefix : "", prefix ? ": " : "", value));
	}

	virtual void LogString(const char* prefix, const char* fmt, ...)
	{
		char buf[512];

		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		m_logger(fmt::sprintf("%s%s%s", prefix ? prefix : "", prefix ? ": " : "", buf));
	}
#endif

private:
	std::function<void(const std::string&)> m_logger;
};

extern std::map<int, std::map<void*, std::tuple<int, uint32_t>>> g_netObjectNodeMapping;

static void TraverseSyncNode(TSyncLog& retval, TSyncLog* old, int oldId, rage::netSyncNodeBase* node, rage::netObject* object = nullptr)
{
	if (node->IsParentNode())
	{
		for (auto child = node->firstChild; child; child = child->nextSibling)
		{
			TraverseSyncNode(retval, old, oldId, child, object);
		}
	}
	else
	{
		SyncLogger logger([&retval, node, old, oldId](const std::string& line)
		{
			if (std::get<1>(g_netObjectNodeMapping[oldId][node]) < (rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() - 75))
			{
				retval[node] = (*old)[node];
				return;
			}

			retval[node].push_back(line);
		});

		if (object)
		{
			node->LogObject(object, &logger);
		}
		else
		{
			node->LogNode(&logger);
		}
	}
}

static TSyncLog TraverseSyncTree(TSyncLog* old, int oldId, rage::netSyncTree* syncTree, rage::netObject* object = nullptr)
{
	TSyncLog retval;
	TraverseSyncNode(retval, old, oldId, syncTree->syncNode, object);

	return retval;
}

#include <array>

void DirtyNode(rage::netObject* object, rage::netSyncDataNodeBase* node);

namespace rage
{
struct WriteTreeState
{
	rage::netObject* object;
	int flags;
	int objFlags;
	rage::datBitBuffer* buffer;
	rage::netLogStub* logger;
	uint32_t time;
	bool wroteAny;
	uint32_t* lastChangeTimePtr;
	int pass = 0;
};

struct NetObjectNodeData
{
	std::tuple<std::array<uint8_t, 1024>, int> lastData;
	std::tuple<std::array<uint8_t, 1024>, int> currentData;
	uint32_t lastChange;
	uint32_t lastAck;
	uint32_t lastResend;
	bool manuallyDirtied = false;

	NetObjectNodeData()
	{
		std::array<uint8_t, 1024> dummyData;
		memset(dummyData.data(), 0, dummyData.size());

		lastData = { dummyData, 0 };
		currentData = { dummyData, 0 };

		lastChange = 0;
		lastAck = 0;
		lastResend = 0;
	}
};

struct NetObjectData
{
	std::array<NetObjectNodeData, 200> nodes;
};

std::array<std::unique_ptr<NetObjectData>, 65536> g_syncData;

template<typename T>
static bool TraverseTreeInternal(rage::netSyncNodeBase* node, T& state, const std::function<bool(T&, rage::netSyncNodeBase*, const std::function<bool()>&)>& cb)
{
	return cb(state, node, [&node, &state, &cb]()
	{
		bool val = false;

		if (node->IsParentNode())
		{
			for (auto child = node->firstChild; child; child = child->nextSibling)
			{
				if (TraverseTreeInternal(child, state, cb))
				{
					val = true;
				}
			}
		}

		return val;
	});
}

template<typename T>
static void TraverseTree(rage::netSyncTree* tree, T& state, const std::function<bool(T&, rage::netSyncNodeBase*, const std::function<bool()>&)>& cb)
{
	TraverseTreeInternal(tree->syncNode, state, cb);
}

static void InitTree(rage::netSyncTree* tree)
{
	// unused padding in GTA5/RDR3
#ifdef GTA_FIVE
	auto didStuff = (uint16_t*)((char*)tree + 1222);
#elif IS_RDR3
	auto didStuff = (uint16_t*)((char*)tree + 1990);
#endif

	if (*didStuff != 0xCFCF)
	{
		size_t idx = 1;
		TraverseTree<size_t>(tree, idx, [](size_t& idx, rage::netSyncNodeBase* node, const std::function<bool()>& cb) -> bool
		{
			if (node->IsParentNode())
			{
				cb();
			}
			else if (node->IsDataNode())
			{
#ifdef GTA_FIVE
				*((uint8_t*)node + 66) = idx++;
#elif IS_RDR3
				*((uint8_t*)node + 68) = idx++;
#endif
			}

			return true;
		});


		*didStuff = 0xCFCF;
	}
}

inline uint32_t GetDelayForUpdateFrequency(uint8_t updateFrequency)
{
	switch (updateFrequency)
	{
	case 5:
		return 0;
	case 4:
		return 25;
	case 3:
		return 100;
	case 2:
		return 300;
	case 1:
		return 400;
	case 0:
	default:
		return 1000;
	}
}

static void AddNodeAndExternalDependentNodes(netSyncDataNodeBase* node, rage::netSyncDataNodeBase* children[], size_t* childCount, size_t maxChildren)
{
	// if we still have space
	if (*childCount < maxChildren)
	{
		// add self
		children[*childCount] = node;
		(*childCount)++;

		// add children
		for (size_t i = 0; i < node->externalDependencyCount; i++)
		{
			AddNodeAndExternalDependentNodes(node->externalDependencies[i], children, childCount, maxChildren);
		}
	}
}

#ifdef GTA_FIVE
static void LoadPlayerAppearanceDataNode(rage::netSyncNodeBase* node);
static void StorePlayerAppearanceDataNode(rage::netSyncNodeBase* node);
#endif

bool netSyncTree::WriteTreeCfx(int flags, int objFlags, rage::netObject* object, rage::datBitBuffer* buffer, uint32_t time, void* logger, uint8_t targetPlayer, void* outNull, uint32_t* lastChangeTime)
{
	InitTree(this);

	WriteTreeState state;
	state.object = object;
	state.flags = flags;
	state.objFlags = objFlags;
	state.buffer = buffer;
	state.logger = (rage::netLogStub*)logger;
	state.time = time;
	state.wroteAny = false;
	state.lastChangeTimePtr = lastChangeTime;

	if (lastChangeTime)
	{
		*lastChangeTime = 0;
	}

	if (flags == 2 || flags == 4)
	{
		// mA0 bit
		buffer->WriteBit(objFlags & 1);
	}

#ifdef IS_RDR3
	buffer->WriteBit(0);
#endif

	// #NETVER: 2018-12-27 17:41 -> increased maximum packet size to 768 from 256 to account for large CPlayerAppearanceDataNode
	static auto icgi = Instance<ICoreGameInit>::Get();

	int sizeLength = 13;

	if (icgi->OneSyncBigIdEnabled)
	{
		sizeLength = 16;
	}
	else if (icgi->NetProtoVersion < 0x201812271741)
	{
		sizeLength = 11;
	}

	eastl::bitset<200> processedNodes;

	// callback
	auto nodeWriter = [this, sizeLength, &processedNodes](WriteTreeState& state, rage::netSyncNodeBase* node, const std::function<bool()>& cb)
	{
		auto buffer = state.buffer;
		bool didWrite = false;

		size_t nodeIdx = GET_NIDX(this, node);

		if (state.flags & node->flags1 && (!node->flags3 || state.objFlags & node->flags3))
		{
			// save position to allow rewinding
			auto startPos = buffer->GetPosition();

			if (state.pass == 2)
			{
				// write presence header placeholder
				if (node->flags2 & state.flags)
				{
					buffer->WriteBit(0);
				}

				// write Cfx length placeholder
				if (node->IsDataNode())
				{
#ifndef ONESYNC_CLONING_NATIVES
					buffer->WriteUns(0, sizeLength);
#endif
				}
			}

			if (node->IsParentNode())
			{
				didWrite = cb();
			}
			else
			{
				// compare last data for the node
				auto nodeData = &g_syncData[state.object->GetObjectId()]->nodes[nodeIdx];
				uint32_t nodeSyncDelay = GetDelayForUpdateFrequency(node->GetUpdateFrequency(UpdateLevel::VERY_HIGH));

				// throttle sends by waiting for the requested node delay
				uint32_t lastChangeDelta = (state.time - nodeData->lastChange);

				if (state.pass == 1 && (lastChangeDelta > nodeSyncDelay || nodeData->manuallyDirtied))
				{
					auto updateNode = [this, &state, &processedNodes, sizeLength](rage::netSyncDataNodeBase* dataNode, bool force) -> bool
					{
						size_t dataNodeIdx = GET_NIDX(this, dataNode);
						auto nodeData = &g_syncData[state.object->GetObjectId()]->nodes[dataNodeIdx];

						if (processedNodes.test(dataNodeIdx))
						{
							return nodeData->lastChange == state.time || force;
						}

						// calculate node change state
						std::array<uint8_t, 1024> tempData;
						memset(tempData.data(), 0, tempData.size());

#ifdef GTA_FIVE
						LoadPlayerAppearanceDataNode(dataNode);
#endif

						rage::datBitBuffer tempBuf(tempData.data(), (sizeLength == 11) ? 256 : tempData.size());
						dataNode->WriteObject(state.object, &tempBuf, state.logger, true);

#ifdef GTA_FIVE
						StorePlayerAppearanceDataNode(dataNode);
#endif

						if (force || tempBuf.m_curBit != std::get<1>(nodeData->lastData) || memcmp(tempData.data(), std::get<0>(nodeData->lastData).data(), tempData.size()) != 0)
						{
							nodeData->lastResend = 0;
							nodeData->lastChange = state.time;
							nodeData->lastData = { tempData, tempBuf.m_curBit };
							nodeData->currentData = { tempData, tempBuf.m_curBit };
							nodeData->manuallyDirtied = false;

							return true;
						}

						processedNodes.set(dataNodeIdx);

						return false;
					};

					auto dataNode = (rage::netSyncDataNodeBase*)node;

#ifdef GTA_FIVE
					static_assert(offsetof(rage::netSyncDataNodeBase, externalDependentNodeRoot) == 0x50, "parentData off");
#elif IS_RDR3
					static_assert(offsetof(rage::netSyncDataNodeBase, externalDependentNodeRoot) == 0x48, "parentData off");
#endif

					// if we are a data node, we will have to ensure external-dependent nodes are sent as a bundle at all times
					// this means:
					// 1. trace up to the root of the external-dependent node tree.
					// 2. from this point, *recursively* dirty all children, for we are dirty as well.
					//
					// the original implementation here did not take rage::netSyncTree::Update nor rage::AddNodeAndExternalDependentNodes as a reference
					// and wasn't tracing up to the root of the external-dependent node tree, nor did it recursively dirty children.

					if (dataNode->externalDependentNodeRoot || dataNode->externalDependencyCount > 0)
					{
						auto rootNode = dataNode;

						while (rootNode->externalDependentNodeRoot)
						{
							rootNode = rootNode->externalDependentNodeRoot;
						}

						if (rootNode)
						{
							rage::netSyncDataNodeBase* children[16];
							size_t childCount = 0;

							AddNodeAndExternalDependentNodes(rootNode, children, &childCount, std::size(children));

							bool written = false;

							for (int child = 0; child < childCount; child++)
							{
								size_t childIdx = GET_NIDX(this, children[child]);
								auto childData = &g_syncData[state.object->GetObjectId()]->nodes[childIdx];

								written |= updateNode(children[child], nodeData->manuallyDirtied || childData->manuallyDirtied || written);
							}
						}
					}
					else
					{
						updateNode(dataNode, nodeData->manuallyDirtied);
					}
				}

				// resend skipping is broken, perhaps?
				bool isResendSkipped = false;//((state.time - nodeData->lastResend) < 150);

				if (state.pass == 2)
				{
					if (state.lastChangeTimePtr)
					{
						auto oldVal = *state.lastChangeTimePtr;

						if (nodeData->lastChange > oldVal && !isResendSkipped)
						{
							*state.lastChangeTimePtr = nodeData->lastChange;

							nodeData->lastResend = state.time;
						}
					}
				}

				bool shouldWriteNode = false;

				if (!(node->flags2 & state.flags))
				{
					shouldWriteNode = true;
				}

				if (nodeData->lastAck < nodeData->lastChange)
				{
					shouldWriteNode = true;
				}

				if (isResendSkipped)
				{
					shouldWriteNode = false;
				}

				if (shouldWriteNode)
				{
					if (state.pass == 2)
					{
						buffer->WriteBits(std::get<0>(nodeData->currentData).data(), std::get<1>(nodeData->currentData), 0);
					}

					didWrite = true;
				}
			}

			if (!didWrite)
			{
				if (state.pass == 2)
				{
					// set position to just past the 0
					if (node->flags2 & state.flags)
					{
						buffer->Seek(startPos + 1);
					}
					else
					{
						buffer->Seek(startPos);
					}
				}
			}
			else
			{
				state.wroteAny = true;

				if (state.object)
				{
					g_netObjectNodeMapping[state.object->GetObjectId()][node] = { 1, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() };
				}

				uint32_t endPos = buffer->GetPosition();

				if (state.pass == 2)
				{
					buffer->Seek(startPos);

					if (node->flags2 & state.flags)
					{
						buffer->WriteBit(true);
					}
				}

				if (node->IsDataNode())
				{
					auto length = endPos - startPos - sizeLength;

					if (node->flags2 & state.flags)
					{
						length -= 1;
					}

#if 0
					if (length >= (1 << 13))
					{
						auto extraDumpPath = MakeRelativeCitPath(L"data\\cache\\extra_dump_info.bin");

						auto f = _wfopen(extraDumpPath.c_str(), L"wb");

						if (f)
						{
							fwrite(buffer->m_data, 1, buffer->m_maxBit / 8, f);
							fclose(f);
						}

						trace("Node type: %s\n", typeid(*node).name());
						trace("Start offset: %d\n", startPos);

						FatalError("Tried to write a bad node length of %d bits in a '%s'. There should only ever be 8192 bits. Please report this on https://forum.fivem.net/t/318260 together with the .zip file from 'save information' below.", length, typeid(*node).name());
					}
#endif

					if (state.pass == 2)
					{
#ifndef ONESYNC_CLONING_NATIVES
						buffer->WriteUns(length, sizeLength);
#endif
					}
				}

				if (state.pass == 2)
				{
					buffer->Seek(endPos);
				}
			}
		}

		return didWrite;
	};

	// traverse state and dirty nodes first
	state.pass = 1;
	TraverseTree<WriteTreeState>(this, state, nodeWriter);

	// then traverse again, writing nodes
	state.pass = 2;
	state.wroteAny = false;
	TraverseTree<WriteTreeState>(this, state, nodeWriter);

	return state.wroteAny;
}

struct AckState
{
	netObject* object;
	uint32_t time;
};

void netSyncTree::AckCfx(netObject* object, uint32_t timestamp)
{
	InitTree(this);

	AckState state;
	state.object = object;
	state.time = timestamp;

	TraverseTree<AckState>(this, state, [this](AckState& state, rage::netSyncNodeBase* node, const std::function<bool()>& cb)
	{
		if (node->IsParentNode())
		{
			cb();
		}
		else
		{
			size_t nodeIdx = GET_NIDX(this, node);

			if (state.time > g_syncData[state.object->GetObjectId()]->nodes[nodeIdx].lastAck)
			{
				g_syncData[state.object->GetObjectId()]->nodes[nodeIdx].lastAck = state.time;
			}
		}

		return true;
	});
}
}

static bool g_recordingDrilldown;
static bool g_recordedDrilldown;
static std::chrono::milliseconds g_drilldownEnd;
static size_t g_drilldownIdx;
static size_t g_drilldownIdxOut;

struct ClonePacketMsg
{
	std::string_view what;
	std::string why;
};

struct ClonePacketData
{
	uint64_t frameIdx;
	uint32_t ts;

	std::vector<ClonePacketMsg> messages;
};

#include <nutsnbolts.h>

static auto msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

static InitFunction initFunctionDrilldown([]()
{
	OnGameFrame.Connect([]()
	{
		if (g_recordingDrilldown && msec() >= g_drilldownEnd)
		{
			g_recordingDrilldown = false;
			g_recordedDrilldown = true;
		}
	});
});

static std::map<uint32_t, ClonePacketData> g_drilldownData;
static std::map<uint32_t, ClonePacketData> g_drilldownDataOut;

namespace sync
{
bool IsDrilldown()
{
	return g_recordingDrilldown;
}

void AddDrilldown(uint64_t frameIdx, std::vector<std::tuple<std::string_view, std::string>>&& data, bool isIn)
{
	ClonePacketData bit;
	bit.frameIdx = frameIdx;

	for (auto& d : data)
	{
		ClonePacketMsg msg;
		msg.what = std::move(std::get<0>(d));
		msg.why = std::move(std::get<1>(d));

		bit.messages.push_back(std::move(msg));
	}

	bit.ts = 1500 - (g_drilldownEnd - msec()).count();

	(isIn ? g_drilldownData : g_drilldownDataOut)[(isIn ? g_drilldownIdx : g_drilldownIdxOut)++] = std::move(bit);
}
}

void RenderNetDrilldownWindow()
{
	static bool open = true;

	ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Appearing);

	if (ImGui::Begin("Network Drilldown", &open))
	{
		if (!g_recordingDrilldown && ImGui::Button("Record"))
		{
			g_drilldownData.clear();
			g_drilldownDataOut.clear();
			g_drilldownIdx = 0;
			g_drilldownIdxOut = 0;
			g_recordedDrilldown = false;

			g_recordingDrilldown = true;
			g_drilldownEnd = msec() + std::chrono::milliseconds{ 1500 };
		}

		if (g_recordingDrilldown)
		{
			ImGui::ButtonEx("Recording", {}, ImGuiButtonFlags_Disabled);
		}

		if (g_recordedDrilldown)
		{
			if (ImGui::TreeNode("In"))
			{
				for (auto& [id, node] : g_drilldownData)
				{
					sync::FrameIndex fi{ node.frameIdx };

					if (ImGui::TreeNode(va("Packet %d @+%d (%d:%d)", id, node.ts, fi.frameIndex, fi.currentFragment)))
					{
						for (auto& message : node.messages)
						{
							ImGui::TreeNodeEx(va("%s: %s", message.what, message.why), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
						}

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Out"))
			{
				for (auto& [id, node] : g_drilldownDataOut)
				{
					if (ImGui::TreeNode(va("Tick %d @+%d (%d)", id, node.ts, node.frameIdx)))
					{
						for (auto& message : node.messages)
						{
							ImGui::TreeNodeEx(va("%s: %s", message.what, message.why), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
						}

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}
		}
	}

	ImGui::End();
}

extern uint32_t* rage__s_NetworkTimeThisFrameStart;

void DirtyNode(rage::netObject* object, rage::netSyncDataNodeBase* node)
{
	auto tree = object->GetSyncTree();
	InitTree(tree);

	size_t nodeIdx = GET_NIDX(tree, node);
	const auto& sd = rage::g_syncData[((rage::netObject*)object)->GetObjectId()];

	if (!sd)
	{
		return;
	}

	auto& nodeData = sd->nodes[nodeIdx];
	nodeData.lastChange = *rage__s_NetworkTimeThisFrameStart;
	nodeData.lastAck = 0;
	nodeData.lastResend = 0;
	nodeData.manuallyDirtied = true;
}

static bool g_captureSyncLog;

void AssociateSyncTree(int objectId, rage::netSyncTree* syncTree)
{
	if (g_captureSyncLog)
	{
		syncLog[objectId] = TraverseSyncTree(&syncLog[objectId], objectId, syncTree);
	}
}

static const char* DescribeGameObject(void* object)
{
	struct VirtualBase
	{
		virtual ~VirtualBase() = default;
	};

	auto vObject = (VirtualBase*)object;

	static std::string objectName = GetClassTypeName(vObject);

	return objectName.c_str();
}

void RenderNetObjectDetail(rage::netObject* netObject)
{
	ImGui::Text("Object ID: %d", netObject->GetObjectId());
	ImGui::Text("Object type: %d", netObject->GetObjectType());
	ImGui::Text("Object owner: %d", netObject->syncData.ownerId);
	ImGui::Text("Is remote: %s", netObject->syncData.isRemote ? "true" : "false");
	ImGui::Text("Game object: %s", netObject->GetGameObject() ? DescribeGameObject(netObject->GetGameObject()) : "NULL");

	if (ImGui::Button("Force blend"))
	{
		auto blender = netObject->GetBlender();

		if (blender)
		{
			blender->ApplyBlend();
		}
	}
}

void RenderSyncNodeDetail(rage::netObject* netObject, rage::netSyncNodeBase* node)
{
	std::vector<std::string> left;

	SyncLogger logger([&left](const std::string& line)
	{
		left.push_back(line);
	});

#ifdef GTA_FIVE
	LoadPlayerAppearanceDataNode(node);
#endif

	node->LogObject(netObject, &logger);

#ifdef GTA_FIVE
	StorePlayerAppearanceDataNode(node);
#endif

	std::vector<std::string> right = syncLog[netObject->GetObjectId()][node];

	auto t = g_netObjectNodeMapping[netObject->GetObjectId()][node];

	InitTree(netObject->GetSyncTree());
	auto& sd = rage::g_syncData[netObject->GetObjectId()]->nodes[GET_NIDX(netObject->GetSyncTree(), node)];

	ImGui::Text("Last %s: %d ms ago", std::get<int>(t) ? "written" : "read", rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() - std::get<uint32_t>(t));
	ImGui::Text("Last ack: %d ms ago", rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() - sd.lastAck);
	ImGui::Text("Last change: %d ms ago", rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() - sd.lastChange);
	ImGui::Text("Change - Ack: %d ms", sd.lastChange - sd.lastAck);

	ImGui::Columns(2);
	ImGui::Text("Current");
	ImGui::NextColumn();
	ImGui::Text("Saved");
	ImGui::NextColumn();

	auto maxSize = std::max(left.size(), right.size());

	for (int i = 0; i < maxSize; i++)
	{
		if (i < left.size())
		{
			ImGui::Text("%s", left[i].c_str());
		}
		else
		{
			ImGui::Text("");
		}

		ImGui::NextColumn();

		if (i < right.size())
		{
			ImGui::Text("%s", right[i].c_str());
		}
		else
		{
			ImGui::Text("");
		}

		ImGui::NextColumn();
	}

	ImGui::Columns(1);
}

static InitFunction initFunction([]()
{
	static bool netViewerEnabled;
	static bool timeWindowEnabled;
	static bool drilldownWindowEnabled;

	static ConVar<bool> netViewerVar("netobjviewer", ConVar_Archive, false, &netViewerEnabled);
	static ConVar<bool> syncLogVar("netobjviewer_syncLog", ConVar_Archive, false, &g_captureSyncLog);
	static ConVar<bool> timeVar("net_showTime", ConVar_Archive, false, &timeWindowEnabled);
	static ConVar<bool> cloneDrilldownVar("net_showDrilldown", ConVar_Archive, false, &drilldownWindowEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || netViewerEnabled || timeWindowEnabled || drilldownWindowEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (drilldownWindowEnabled)
		{
			RenderNetDrilldownWindow();
		}

		if (timeWindowEnabled)
		{
			if (ImGui::Begin("Time", &timeWindowEnabled))
			{
				auto inst = rage::netInterface_queryFunctions::GetInstance();

				ImGui::Text("%u", inst ? inst->GetTimestamp() : 0);
			}

			ImGui::End();
		}

		if (!netViewerEnabled)
		{
			return;
		}

		ImGui::SetNextWindowSizeConstraints(ImVec2(1020.0f, 400.0f), ImVec2(1020.0f, 2000.0f));

		if (ImGui::Begin("Network Object Viewer", &netViewerEnabled) && rage::netObjectMgr::GetInstance())
		{
			static float treeSize = 400.f;
			static float detailSize = 600.f;

			Splitter(true, 8.0f, &treeSize, &detailSize, 8.0f, 8.0f);

			if (ImGui::BeginChild("tree", ImVec2(treeSize, -1.0f), true))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize());

				RenderNetObjectTree();

				ImGui::PopStyleVar();
			}

			ImGui::EndChild();
			ImGui::SameLine();

			if (ImGui::BeginChild("detail", ImVec2(detailSize, -1.0f), true))
			{
				if (g_curNetObjectSelection)
				{
					RenderNetObjectDetail(g_curNetObjectSelection);
				}

				if (g_curNetObjectSelection && g_curSyncNodeSelection)
				{
					ImGui::Separator();

					RenderSyncNodeDetail(g_curNetObjectSelection, g_curSyncNodeSelection);
				}
			}

			ImGui::EndChild();
		}

		ImGui::End();
	});
});

#if _DEBUG
static void DumpSyncNode(rage::netSyncNodeBase* node, std::string indent = "\t", bool last = true)
{
	std::string objectName = GetClassTypeName(node);

	if (node->IsParentNode())
	{
		trace("%sParentNode<\n", indent);
		trace("%s\tNodeIds<%d, %d, %d>,\n", indent, node->flags1, node->flags2, node->flags3);

		for (auto child = node->firstChild; child; child = child->nextSibling)
		{
			DumpSyncNode(child, indent + "\t", (child->nextSibling == nullptr));
		}

		trace("%s>%s\n", indent, !last ? "," : "");
	}
	else
	{
		trace("%sNodeWrapper<NodeIds<%d, %d, %d>, %s>%s\n", indent, node->flags1, node->flags2, node->flags3, objectName, !last ? "," : "");
	}
}

static void DumpSyncTree(rage::netSyncTree* syncTree)
{
	std::string objectName = GetClassTypeName(syncTree);

	trace("using %s = SyncTree<\n", objectName);

	DumpSyncNode(syncTree->syncNode);

	trace(">;\n");
}
#endif

#ifdef GTA_FIVE
static uintptr_t g_vtbl_playerAppearanceDataNode;
static uint32_t g_offset_playerAppearanceDataNode_hasDecorations;

static char g_localPlayer_hasDecorations;

namespace rage
{
static void LoadPlayerAppearanceDataNode(rage::netSyncNodeBase* node)
{
	if (*(uintptr_t*)node == g_vtbl_playerAppearanceDataNode)
	{
		*((char*)node + g_offset_playerAppearanceDataNode_hasDecorations) = g_localPlayer_hasDecorations;
	}
}

static void StorePlayerAppearanceDataNode(rage::netSyncNodeBase* node)
{
	if (*(uintptr_t*)node == g_vtbl_playerAppearanceDataNode)
	{
		g_localPlayer_hasDecorations = *((char*)node + g_offset_playerAppearanceDataNode_hasDecorations);
	}
}
}
#endif

static HookFunction hookFunction([]()
{

#if _DEBUG
	static ConsoleCommand dumpSyncTreesCmd("dumpSyncTrees", []()
	{
		for (int i = 0; i < (int)NetObjEntityType::Max; i++)
		{
			auto obj = rage::CreateCloneObject((NetObjEntityType)i, i + 204, 0, 0, 32);
			auto tree = obj->GetSyncTree();

			DumpSyncTree(tree);
		}
	});
#endif

#if GTA_FIVE
	// CPlayerAppearanceDataNode decorations uninitialized value
	{
		g_vtbl_playerAppearanceDataNode = hook::get_address<uintptr_t>(hook::pattern("48 89 BB B8 00 00 00 48 89 83 B0 00 00 00").count(2).get(1).get<void*>(-0xE));
		g_offset_playerAppearanceDataNode_hasDecorations = *hook::get_pattern<uint32_t>("88 83 ? ? ? ? 84 C0 75 0D 44 8B C5 33", 2);
	}

	// allow CSyncDataLogger even without label string
	hook::nop(hook::get_pattern("4D 85 C9 74 44 48 8D 4C", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C0 74 25 80 3A 00", 3), 2);

	{
		auto p = hook::pattern("4D 85 C9 74 13 4C 8B 0A").count(2);

		for (int i = 0; i < p.size(); i++)
		{
			hook::nop(p.get(i).get<void>(3), 2);
		}
	}

	{
		auto p = hook::pattern("4D 85 C9 74 13 44 8B 0A").count(2);

		for (int i = 0; i < p.size(); i++)
		{
			hook::nop(p.get(i).get<void>(3), 2);
		}
	}

	hook::nop(hook::get_pattern("4D 85 C9 74 14 44 0F BE 0A", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C9 74 14 44 0F B7 0A", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C9 74 14 44 0F B6 0A", 3), 2);
	hook::nop(hook::get_pattern("50 48 85 D2 74 1A", 4), 2);
	hook::nop(hook::get_pattern("48 85 DB 74 2D 44 0F B7 0A", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C0 74 38 F3 0F 10", 3), 2);
	hook::nop(hook::get_pattern("48 85 DB 74 5C 83 64 24", 3), 2);
	hook::nop(hook::get_pattern("48 85 D2 74 38 F3 41", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C9 74 46 F3", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C9 74 11 48 85", 3), 2);
#elif IS_RDR3
	{
		auto p = hook::pattern("48 85 D2 74 09 48 8B 01 4D").count(2);

		for (int i = 0; i < p.size(); i++)
		{
			hook::nop(p.get(i).get<void>(3), 2);
		}
	}

	{
		auto p = hook::pattern("48 85 D2 74 09 48 8B 01 45 8B").count(2);

		for (int i = 0; i < p.size(); i++)
		{
			hook::nop(p.get(i).get<void>(3), 2);
		}
	}

	{
		auto p = hook::pattern("48 85 D2 74 0A 48 8B 01 45 0F B7").count(2);

		for (int i = 0; i < p.size(); i++)
		{
			hook::nop(p.get(i).get<void>(3), 2);
		}
	}

	{
		auto p = hook::pattern("48 85 D2 74 0A 48 8B 01 45 0F B6").count(2);

		for (int i = 0; i < p.size(); i++)
		{
			hook::nop(p.get(i).get<void>(3), 2);
		}
	}

	hook::nop(hook::get_pattern("4D 85 C9 74 0C 44 8A 02", 3), 2);
	hook::nop(hook::get_pattern("48 85 D2 74 0A 48 8B 01 45 0F BF", 3), 2);
	hook::nop(hook::get_pattern("48 85 D2 74 0A 48 8B 01 45 0F BE", 3), 2);
	hook::nop(hook::get_pattern("48 85 D2 74 06 48 8B 01 FF 50 30", 3), 2);
	hook::nop(hook::get_pattern("48 85 D2 74 06 48 8B 01 FF 50 40", 3), 2);
	hook::nop(hook::get_pattern("48 85 D2 74 0B 48 8B 01 F3 41", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C9 74 0D 44 0F B6 02", 3), 2);
	hook::nop(hook::get_pattern("48 85 DB 74 34 44 0F B7 0A", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C9 74 38 F3 0F 10 42 08", 3), 2);
	hook::nop(hook::get_pattern("48 85 DB 74 4C 48 8D 54 24 30 48 8B C8", 3), 2);
	hook::nop(hook::get_pattern("48 85 D2 74 38 F3 41 0F 10 40 08", 3), 2);
	hook::nop(hook::get_pattern("48 85 D2 74 47 F3 41 0F 10 40 0C", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C9 74 0D 48 8B 01 4C", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C9 74 1B 8B 42 04 4C", 3), 2);
	hook::nop(hook::get_pattern("4D 85 C9 74 13 44 8B 0A 4C", 3), 2);
#endif
});


void CD_AllocateSyncData(uint16_t objectId)
{
	if (!rage::g_syncData[objectId])
	{
		rage::g_syncData[objectId] = std::make_unique<rage::NetObjectData>();
	}
}

void CD_FreeSyncData(uint16_t objectId)
{
	rage::g_syncData[objectId] = {};
}
