#include <StdInc.h>
#include <state/ServerGameStatePublic.h>
#include <ClientRegistry.h>
#include <GameServer.h>
#include <UdpInterceptor.h>

#include <ServerGui.h>
#include <imgui.h>
#include <imguivariouscontrols.h>

#include <deque>

static std::pair<uint32_t, const char*> g_knownPackets[]
{
	{ HashRageString("msgConVars"),"msgConVars" },
	{ HashRageString("msgEnd"),	"msgEnd" },
	{ HashRageString("msgEntityCreate"),"msgEntityCreate" },
	{ HashRageString("msgNetGameEvent"),"msgNetGameEvent" },
	{ HashRageString("msgObjectIds"),"msgObjectIds" },
	{ HashRageString("msgPackedAcks"),"msgPackedAcks" },
	{ HashRageString("msgPackedClones"),"msgPackedClones" },
	{ HashRageString("msgPaymentRequest"),"msgPaymentRequest" },
	{ HashRageString("msgRequestObjectIds"),"msgRequestObjectIds" },
	{ HashRageString("msgResStart"),"msgResStart" },
	{ HashRageString("msgResStop"),"msgResStop" },
	{ HashRageString("msgRoute"),"msgRoute" },
	{ HashRageString("msgRpcEntityCreation"),"msgRpcEntityCreation" },
	{ HashRageString("msgRpcNative"),"msgRpcNative" },
	{ HashRageString("msgServerCommand"),"msgServerCommand" },
	{ HashRageString("msgServerEvent"),"msgServerEvent" },
	{ HashRageString("msgTimeSync"),"msgTimeSync" },
	{ HashRageString("msgTimeSyncReq"),"msgTimeSyncReq" },
	{ HashRageString("msgWorldGrid"),"msgWorldGrid" },
	{ HashRageString("msgFrame"),"msgFrame" },
	{ HashRageString("msgIHost"),"msgIHost" },
	{ HashRageString("gameStateAck"),"gameStateAck" },
	{ HashRageString("gameStateNAck"),"gameStateNAck" },
	{ HashRageString("msgNetEvent"),"msgNetEvent" },
};

struct PlayerAdvancedData
{
	void Reset()
	{
		clientInBytes.clear();
		clientOutBytes.clear();
		graphUpdateRate = 1; //500ms
		lastUpdate = msec();
		showIncoming = true;
		showOutgoing = true;
	}
	struct PlayerAdvancedData()
	{
		Reset();
	}
	static constexpr int GRAPH_MAX_SAMPLES = 150;
	static constexpr char* UPDATE_RATES[] = { "2 Seconds", "1 Second", "1/2 Second", "1/4 Second", "1/8 Second" };
	static constexpr int UPDATE_RATE_VALUES_MS[] = { 2000, 1000, 500, 250, 125 };
	int graphUpdateRate; //array index ^^
	std::chrono::milliseconds lastUpdate;
	bool showIncoming; // for collapsing header
	bool showOutgoing; // ^^

	// the callbacks will add to this automatically over time
	std::atomic_uint32_t bytesOutRaw;
	std::atomic_uint32_t bytesInRaw;
	// # of times each packet was seen. (Array sizeof g_knownPackets)
	std::atomic_uint16_t pktOccurancesRecv[std::size(g_knownPackets)];
	std::atomic_uint16_t pktOccurancesSend[std::size(g_knownPackets)];
	// # of bytes used per packet type. (Array sizeof g_knownPackets)
	std::atomic_uint16_t pktBytesRecv[std::size(g_knownPackets)];
	std::atomic_uint16_t pktBytesSend[std::size(g_knownPackets)];

	// After it is time for a Graph update, the collect function will shove the number into the record here(graph requires float type)
	std::deque<float> clientOutBytes;
	std::deque<float> clientInBytes;
	// and also collect their names into here
	std::vector<std::string> pktsSeenRecv;
	std::vector<std::string> pktsSeenSend;
};
struct PlayerListData
{
	PlayerListData()
	{
		showPopout = false;
	}
	std::string name;
	glm::vec3 position;
	// pre-calculate these
	unsigned int hoursOnline;
	unsigned int minutesOnline;
	unsigned int secondsOnline;
	std::string connectionState;
	int currentPing;

	bool showPopout;
	PlayerAdvancedData advancedData;
};

static std::shared_mutex g_playerListDataMutex;
static std::map<std::string, PlayerListData> g_playerListData;

static const auto& CollectPlayers(fx::ServerInstanceBase* instance)
{
	static auto lastListCollect = msec();
	if ((msec() - lastListCollect) < 500ms)
	{
		return g_playerListData;
	}

	auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
	auto sgs = instance->GetComponent<fx::ServerGameStatePublic>();

	// Skips Dropping clients.
	clientRegistry->ForAllClients([](fx::ClientSharedPtr client)
	{
		std::unique_lock lock(g_playerListDataMutex);
		auto& entry = g_playerListData[client->GetGuid()];
		entry.name = fmt::sprintf("[%d] %s", client->GetNetId(), client->GetName());
		unsigned int secondsTotalOnline = std::chrono::duration_cast<std::chrono::seconds>(client->GetLastSeen() - client->GetFirstSeen()).count();
		entry.secondsOnline = (secondsTotalOnline % 60);
		entry.minutesOnline = ((secondsTotalOnline / 60) % 60);
		entry.hoursOnline = (secondsTotalOnline / 3600);
		entry.currentPing = client->GetPing();
	});

	sgs->ForAllEntities([](fx::sync::Entity* entity)
	{
		auto owner = entity->GetOwner();
		if (!owner || !entity->IsPlayer())
		{
			return;
		}

		{
			std::unique_lock lock(g_playerListDataMutex);
			auto& pld = g_playerListData[owner->GetGuid()];
			pld.position = entity->GetPosition();
			pld.connectionState = "SPAWNED";
		}
	});

	lastListCollect = msec();
	return g_playerListData;
}

void RecvFromClient_Callback(fx::Client* client, uint32_t packetId, net::Buffer& buffer)
{
	// only a shared lock: we assume g_playerListData already contains our data
	std::shared_lock lock(g_playerListDataMutex);
	auto& entry = g_playerListData[client->GetGuid()];

	if (!entry.showPopout)
	{
		return;
	}
	entry.advancedData.bytesInRaw += buffer.GetLength();

	auto data = buffer.GetData();
	// first 4 bytes are the rageHash
	uint32_t pktHash = *(uint32_t*)data.data();
	int i = 0;
	for (auto hash : g_knownPackets)
	{
		if (hash.first == pktHash)
		{
			entry.advancedData.pktOccurancesRecv[i]++;
			entry.advancedData.pktBytesRecv[i] += buffer.GetLength();
		}
		i++;
	}
}

void SendToClient_Callback(fx::Client* client, int channel, const net::Buffer& buffer, NetPacketType flags)
{
	std::shared_lock lock(g_playerListDataMutex);
	auto& entry = g_playerListData[client->GetGuid()];

	if (!entry.showPopout)
	{
		return;
	}
	entry.advancedData.bytesOutRaw += buffer.GetLength();

	auto data = buffer.GetData();
	// first 4 bytes are the rageHash
	uint32_t pktHash = *(uint32_t*)data.data();
	int i = 0;
	for (auto hash : g_knownPackets)
	{
		if (hash.first == pktHash)
		{
			entry.advancedData.pktOccurancesSend[i]++;
			entry.advancedData.pktBytesSend[i] += buffer.GetLength();
		}
		i++;
	}
}

// Connect to various event listeners
static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
		auto server = instance->GetComponent<fx::GameServer>();

		clientRegistry->OnClientCreated.Connect([](const fx::ClientSharedPtr& client)
		{
			std::unique_lock lock(g_playerListDataMutex);
			g_playerListData[client->GetGuid()].connectionState = "CONNECTING";
			client->SetNetworkMetricsRecvCallback(RecvFromClient_Callback);
			client->SetNetworkMetricsSendCallback(SendToClient_Callback);

			client->OnDrop.Connect([client]()
			{
				std::unique_lock lock(g_playerListDataMutex);
				g_playerListData.erase(client->GetGuid());
			});
		});
		clientRegistry->OnConnectedClient.Connect([](fx::Client* client)
		{
			std::unique_lock lock(g_playerListDataMutex);
			g_playerListData[client->GetGuid()].connectionState = "LOADING";
		});
	});
});

// Data for the popup, not collected unless open
static void CollectAdvancedDataForPlayer(PlayerAdvancedData* entry)
{
	if ((msec() - entry->lastUpdate) < std::chrono::milliseconds(PlayerAdvancedData::UPDATE_RATE_VALUES_MS[entry->graphUpdateRate]))
	{
		return;
	}

	// Data is automatically collected from the callbacks.
	float bytesIn = (float)entry->bytesInRaw;
	float bytesOut = (float)entry->bytesOutRaw;
	entry->bytesInRaw = 0;
	entry->bytesOutRaw = 0;
	entry->clientInBytes.push_back(bytesIn);
	entry->clientOutBytes.push_back(bytesOut);

	if (entry->clientInBytes.size() > PlayerAdvancedData::GRAPH_MAX_SAMPLES)
	{
		entry->clientInBytes.pop_front();
	}
	if (entry->clientOutBytes.size() > PlayerAdvancedData::GRAPH_MAX_SAMPLES)
	{
		entry->clientOutBytes.pop_front();
	}

	entry->pktsSeenRecv.clear();
	entry->pktsSeenSend.clear();

	for (int i = 0; i < std::size(g_knownPackets); i++)
	{
		if (entry->pktOccurancesRecv[i] > 0)
		{
			entry->pktsSeenRecv.push_back(fmt::sprintf("%s[x%d] - %dB\n", g_knownPackets[i].second, entry->pktOccurancesRecv[i], entry->pktBytesRecv[i]));
			entry->pktOccurancesRecv[i] = 0;
			entry->pktBytesRecv[i] = 0;
		}
		if (entry->pktOccurancesSend[i] > 0)
		{
			entry->pktsSeenSend.push_back(fmt::sprintf("%s[x%d] - %dB\n", g_knownPackets[i].second, entry->pktOccurancesSend[i], entry->pktBytesSend[i]));
			entry->pktOccurancesSend[i] = 0;
			entry->pktBytesSend[i] = 0;
		}
	}

	entry->lastUpdate = msec();
}

// Detailed Popout Section for player
static void ShowPopout(std::string guid, PlayerListData* data)
{
	ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(guid.c_str(), &data->showPopout, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking))
	{
		CollectAdvancedDataForPlayer(&data->advancedData);

		ImGui::Separator();

		ImGui::Columns(2);
		{
			ImGui::Text("Player: %s", data->name.c_str());

			ImGui::Text("Update Rate");
			ImGui::Combo(fmt::sprintf("##graphrate_%s", guid.c_str()).c_str(), &data->advancedData.graphUpdateRate, PlayerAdvancedData::UPDATE_RATES, IM_ARRAYSIZE(PlayerAdvancedData::UPDATE_RATES), 4);

			
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

			static const char* names[] = {
				"BytesIn",
				"BytesOut"
			};
			static ImColor colors[] = {
				{ 225, 10, 10, 225 },
				{ 10, 255, 225, 225 },
			};
			std::deque<float>* datas[] = {
				&data->advancedData.clientInBytes,
				&data->advancedData.clientOutBytes,
			};
			ImGui::PlotMultiLines(
			"##playernetmetrics", 2, names, colors, [](const void* ctx, int i) -> float
			{
				std::deque<float>* buffer = (std::deque<float>*)ctx;
				if (i >= buffer->size())
				{
					return 0.0f;
				}
				return buffer->operator[](i);
			},
			(void**)datas, PlayerAdvancedData::GRAPH_MAX_SAMPLES, FLT_MAX, FLT_MAX, ImVec2(500.0f, 350.0f));

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
		ImGui::NextColumn();
		{
			if (ImGui::CollapsingHeader("Incoming", &data->advancedData.showIncoming))
			{
				for (auto& entry : data->advancedData.pktsSeenRecv)
				{
					ImGui::Text("%s", entry.c_str());
				}
			}
			if (ImGui::CollapsingHeader("Outgoing", &data->advancedData.showIncoming))
			{
				for (auto& entry : data->advancedData.pktsSeenSend)
				{
					ImGui::Text("%s", entry.c_str());
				}
			}
		}
		ImGui::Columns(1);

		ImGui::End();
	}
}

static SvGuiModule svplayerlist("Player List", "svplayerlist", ImGuiWindowFlags_NoCollapse, [](fx::ServerInstanceBase* instance)
{
	const auto& playerData = CollectPlayers(instance);

	// Lock Mutex Here, it's rarely contested
	std::shared_lock lock(g_playerListDataMutex);

	ImGui::BeginTable("##svplayerlist", 5 /*Column Num*/, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_PreferSortDescending);
		ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_PreferSortDescending);
		ImGui::TableSetupColumn("Time Online", ImGuiTableColumnFlags_PreferSortDescending);
		ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_PreferSortDescending);
		ImGui::TableSetupColumn("Ping", ImGuiTableColumnFlags_PreferSortDescending);
		ImGui::TableHeadersRow();

		ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f)));

		for (auto& [guid, data] : g_playerListData)
		{
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable(data.name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
			{
				// no need to lock-unique here, safe value
				data.showPopout = true;
				// Bring to front when opening
				ImGui::SetWindowFocus(guid.c_str());
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("(%.1f, %.1f, %.1f)", data.position.x, data.position.y, data.position.z);
			if (ImGui::BeginPopupContextItem("coords", ImGuiPopupFlags_MouseButtonRight))
			{
				if (ImGui::Button("Copy"))
				{
					ImGui::GetIO().SetClipboardTextFn(NULL, fmt::sprintf("%.1f %.1f %.1f", data.position.x, data.position.y, data.position.z).c_str());
				}
				ImGui::EndPopup();
			}

			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%02d:%02d:%02d", data.hoursOnline, data.minutesOnline, data.secondsOnline);

			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%s", data.connectionState);

			ImGui::TableSetColumnIndex(4);
			ImGui::Text("%dms", data.currentPing);

			if (data.showPopout)
			{
				ShowPopout(guid, &data);
			}
		}
	ImGui::EndTable();
});
