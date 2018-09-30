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

#include <rlNetBuffer.h>

#include <ICoreGameInit.h>

#include <Hooking.h>

static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
	using namespace ImGui;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	return SplitterBehavior(id, bb, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

namespace rage
{
	class netLogStub
	{
	public:
		virtual ~netLogStub() = default;

		virtual void m_8() = 0;

		virtual void m_10() = 0;

		virtual void m_18() = 0;

		virtual void m_20() = 0;

		virtual void m_28() = 0;

		virtual void LogString(const char* prefix, const char* fmt, ...) = 0;

		virtual void m_38() = 0;

		virtual void m_40() = 0;

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
		virtual void WriteObject(rage::netObject* object, rage::netBuffer* buffer, rage::netLogStub* logger, bool readFromObject) = 0;
		virtual void m_B0() = 0;
		virtual void m_B8() = 0;
		virtual void LogNode(rage::netLogStub* stub) = 0;
		virtual void m_C8() = 0;
		virtual void m_D0() = 0;
		virtual void m_D8() = 0;
		virtual void m_E0() = 0;
		virtual void LogObject(rage::netObject* object, rage::netLogStub* stub) = 0;

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
}

static rage::netObject* g_curNetObjectSelection;
static rage::netSyncNodeBase* g_curSyncNodeSelection;

static void RenderSyncNode(rage::netObject* object, rage::netSyncNodeBase* node)
{
	std::string objectName = typeid(*node).name();
	objectName = objectName.substr(6);

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
	RenderSyncNode(object, *(rage::netSyncNodeBase**)((char*)syncTree + 16));
}

static void RenderNetObjectTree()
{
	for (int player = 0; player < 64; player++)
	{
		std::vector<rage::netObject*> objects;

		rage::netObjectMgr::GetInstance()->ForAllNetObjects(player, [&objects](rage::netObject* object)
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
						std::string objectName = typeid(*object).name();
						objectName = objectName.substr(6);

						if (ImGui::TreeNodeEx(object,
							ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((g_curNetObjectSelection == object) ? ImGuiTreeNodeFlags_Selected : 0),
							"%s - %d", objectName.c_str(), object->objectId))
						{
							if (ImGui::IsItemClicked())
							{
								g_curNetObjectSelection = object;
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

	virtual void m_8()
	{

	}

	virtual void m_10() {}

	virtual void m_18() {}

	virtual void m_20() {}

	virtual void m_28() {}

	virtual void LogString(const char* prefix, const char* fmt, ...)
	{
		char buf[512];

		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		m_logger(fmt::sprintf("%s%s%s", prefix ? prefix : "", prefix ? ": " : "", buf));
	}

	virtual void m_38() {}

	virtual void m_40() {}

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
	TraverseSyncNode(retval, old, oldId, *(rage::netSyncNodeBase**)((char*)syncTree + 16), object);

	return retval;
}

#include <array>

namespace rage
{
struct WriteTreeState
{
	rage::netObject* object;
	int flags;
	int objFlags;
	rage::netBuffer* buffer;
	rage::netLogStub* logger;
	uint32_t time;
	bool wroteAny;
};

struct NetObjectNodeData
{
	std::array<uint8_t, 256> lastData;
	uint32_t lastChange;
	uint32_t lastAck;

	NetObjectNodeData()
	{
		memset(lastData.data(), 0, lastData.size());
		lastChange = 0;
		lastAck = 0;
	}
};

struct NetObjectData
{
	std::unordered_map<rage::netSyncNodeBase*, NetObjectNodeData> nodes;
};

std::unordered_map<int, NetObjectData> g_syncData;

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
	TraverseTreeInternal(*(rage::netSyncNodeBase**)((char*)tree + 16), state, cb);
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

bool netSyncTree::WriteTreeCfx(int flags, int objFlags, rage::netObject* object, rage::netBuffer* buffer, uint32_t time, void* logger, uint8_t targetPlayer, void* outNull)
{
	WriteTreeState state;
	state.object = object;
	state.flags = flags;
	state.objFlags = objFlags;
	state.buffer = buffer;
	state.logger = (rage::netLogStub*)logger;
	state.time = time;
	state.wroteAny = false;

	if (flags == 2 || flags == 4)
	{
		// mA0 bit
		buffer->WriteBit(objFlags & 1);
	}

	TraverseTree<WriteTreeState>(this, state, [](WriteTreeState& state, rage::netSyncNodeBase* node, const std::function<bool()>& cb)
	{
		auto buffer = state.buffer;
		bool didWrite = false;

		if (state.flags & node->flags1 && (!node->flags3 || state.objFlags & node->flags3))
		{
			// save position to allow rewinding
			auto startPos = buffer->GetPosition();

			// write presence header placeholder
			if (node->flags2 & state.flags)
			{
				buffer->WriteBit(0);
			}

			// write Cfx length placeholder
			if (node->IsDataNode())
			{
				buffer->WriteInteger(0, 11);
			}

			if (node->IsParentNode())
			{
				didWrite = cb();
			}
			else
			{
				// compare last data for the node
				auto nodeData = &g_syncData[state.object->objectId].nodes[node];

				uint32_t nodeSyncDelay = GetDelayForUpdateFrequency(node->GetUpdateFrequency(UpdateLevel::VERY_HIGH));

				// calculate node change state
				std::array<uint8_t, 256> tempData;
				memset(tempData.data(), 0, tempData.size());

				rage::netBuffer tempBuf(tempData.data(), tempData.size());

				node->WriteObject(state.object, &tempBuf, nullptr, true);

				if (memcmp(tempData.data(), nodeData->lastData.data(), tempData.size()) != 0)
				{
					// throttle sends by waiting for the requested node delay
					uint32_t lastChangeDelta = (state.time - nodeData->lastChange);

					if (lastChangeDelta > nodeSyncDelay)
					{
						nodeData->lastChange = state.time;
						nodeData->lastData = tempData;
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

				if (shouldWriteNode)
				{
					node->WriteObject(state.object, buffer, state.logger, true);

					didWrite = true;
				}
			}

			if (!didWrite)
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
			else
			{
				state.wroteAny = true;

				if (state.object)
				{
					g_netObjectNodeMapping[state.object->objectId][node] = { 1, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() };
				}

				uint32_t endPos = buffer->GetPosition();
				buffer->Seek(startPos);

				if (node->flags2 & state.flags)
				{
					buffer->WriteBit(true);
				}

				if (node->IsDataNode())
				{
					auto length = endPos - startPos - 11;

					if (node->flags2 & state.flags)
					{
						length -= 1;
					}

					buffer->WriteInteger(length, 11);
				}

				buffer->Seek(endPos);
			}
		}

		return didWrite;
	});

	return state.wroteAny;
}

struct AckState
{
	netObject* object;
	uint32_t time;
};

void netSyncTree::AckCfx(netObject* object, uint32_t timestamp)
{
	AckState state;
	state.object = object;
	state.time = timestamp;

	TraverseTree<AckState>(this, state, [](AckState& state, rage::netSyncNodeBase* node, const std::function<bool()>& cb)
	{
		if (node->IsParentNode())
		{
			cb();
		}
		else
		{
			if (state.time > g_syncData[state.object->objectId].nodes[node].lastAck)
			{
				g_syncData[state.object->objectId].nodes[node].lastAck = state.time;
			}
		}

		return true;
	});
}
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

	static std::string objectName;
	objectName = typeid(*vObject).name();
	objectName = objectName.substr(6);

	return objectName.c_str();
}

void RenderNetObjectDetail(rage::netObject* netObject)
{
	ImGui::Text("Object ID: %d", netObject->objectId);
	ImGui::Text("Object type: %d", netObject->objectType);
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

	node->LogObject(netObject, &logger);

	std::vector<std::string> right = syncLog[netObject->objectId][node];

	auto t = g_netObjectNodeMapping[netObject->objectId][node];
	
	ImGui::Text("Last %s: %d ms ago", std::get<int>(t) ? "written" : "read", rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() - std::get<uint32_t>(t));
	ImGui::Text("Last ack: %d ms ago", rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() - rage::g_syncData[netObject->objectId].nodes[node].lastAck);
	ImGui::Text("Last change: %d ms ago", rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() - rage::g_syncData[netObject->objectId].nodes[node].lastChange);
	ImGui::Text("Change - Ack: %d ms", rage::g_syncData[netObject->objectId].nodes[node].lastChange - rage::g_syncData[netObject->objectId].nodes[node].lastAck);

	ImGui::Columns(2);
	ImGui::Text("Current");
	ImGui::NextColumn();
	ImGui::Text("Saved");
	ImGui::NextColumn();

	for (int i = 0; i < std::max(left.size(), right.size()); i++)
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

uint32_t GetRemoteTime();

static InitFunction initFunction([]()
{
	static bool netViewerEnabled;

	static ConVar<bool> netViewerVar("netobjviewer", ConVar_Archive, false, &netViewerEnabled);
	static ConVar<bool> syncLogVar("netobjviewer_syncLog", ConVar_Archive, false, &g_captureSyncLog);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || netViewerEnabled || Instance<ICoreGameInit>::Get()->OneSyncEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (Instance<ICoreGameInit>::Get()->OneSyncEnabled)
		{
			static bool timeOpen = true;

			if (ImGui::Begin("Time", &timeOpen))
			{
				ImGui::Text("%u", rage::netInterface_queryFunctions::GetInstance()->GetTimestamp());
			}

			ImGui::End();
		}

		if (!netViewerEnabled)
		{
			return;
		}

		static bool novOpen = true;

		ImGui::SetNextWindowSizeConstraints(ImVec2(1020.0f, 400.0f), ImVec2(1020.0f, 2000.0f));

		if (ImGui::Begin("Network Object Viewer", &novOpen))
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

				if (g_curSyncNodeSelection)
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

static HookFunction hookFunction([]()
{
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
});
