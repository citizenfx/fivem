/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

/* Debugging overlay for the 'net' component. */

#include "StdInc.h"
#include "NetLibrary.h"
#include "FontRenderer.h"
#include "DrawCommands.h"
#include <CoreConsole.h>
#include <mmsystem.h>

#include <ConsoleHost.h>

#include <imgui.h>
#include <imguivariouscontrols.h>

const int g_netOverlayOffsetX = -30;
const int g_netOverlayOffsetY = -60;
const int g_netOverlayWidth = 400;
const int g_netOverlayHeight = 320;

const int g_netOverlaySampleSize = 200; // milliseconds per sample frame
const int g_netOverlaySampleCount = 150;

class RageHashList
{
public:
	template<int Size>
	RageHashList(const char* (&list)[Size])
	{
		for (int i = 0; i < Size; i++)
		{
			m_lookupList.insert({ HashRageString(list[i]), list[i] });
		}
	}

	inline std::string LookupHash(uint32_t hash)
	{
		auto it = m_lookupList.find(hash);

		if (it != m_lookupList.end())
		{
			return std::string(it->second);
		}

		return fmt::sprintf("0x%08x", hash);
	}

private:
	std::map<uint32_t, std::string_view> m_lookupList;
};

static const char* g_knownPackets[]
{
	"msgConVars",
	"msgEnd",
	"msgEntityCreate",
	"msgNetGameEvent",
	"msgObjectIds",
	"msgPackedAcks",
	"msgPackedClones",
	"msgPaymentRequest",
	"msgRequestObjectIds",
	"msgResStart",
	"msgResStop",
	"msgRoute",
	"msgRpcEntityCreation",
	"msgRpcNative",
	"msgServerCommand",
	"msgServerEvent",
	"msgTimeSync",
	"msgTimeSyncReq",
	"msgWorldGrid",
	"msgFrame",
	"msgIHost",
	"gameStateAck",
	"msgNetEvent",
	"msgServerEvent",
};

static RageHashList g_hashes{ g_knownPackets };

class NetOverlayMetricSink : public INetMetricSink
{
public:
	NetOverlayMetricSink();

	virtual void OnIncomingPacket(const NetPacketMetrics& packetMetrics) override;

	virtual void OnOutgoingPacket(const NetPacketMetrics& packetMetrics) override;

	virtual void OnIncomingRoutePackets(int num) override;

	virtual void OnOutgoingRoutePackets(int num) override;

	virtual void OnPingResult(int msec) override;

	virtual void OnRouteDelayResult(int msec) override;

	virtual void OnIncomingCommand(uint32_t type, size_t size, bool reliable) override;

	virtual void OnOutgoingCommand(uint32_t type, size_t size, bool reliable) override;

	virtual void OverrideBandwidthStats(uint32_t in, uint32_t out) override;

private:
	int m_ping;

	int m_lastInPackets;
	int m_lastOutPackets;

	int m_lastInBytes;
	int m_lastOutBytes;

	int m_lastInRoutePackets;
	int m_lastOutRoutePackets;

	int m_inPackets;
	int m_outPackets;

	int m_inBytes;
	int m_outBytes;

	int m_inRoutePackets;
	int m_outRoutePackets;

	int m_inRouteDelay;
	int m_inRouteDelayMax;

	int m_inRouteDelaySample;
	int m_inRouteDelaySamples[8];

	int m_inRouteDelaySampleArchive;
	int m_inRouteDelaySamplesArchive[2000];

	bool m_enabled;

	bool m_enabledCommands;

	NetPacketMetrics m_metrics[g_netOverlaySampleCount + 1];

	uint32_t m_lastUpdatePerSec;

	uint32_t m_lastUpdatePerSample;

	std::mutex m_metricMutex;

	std::map<uint32_t, bool> m_incomingReliable;

	std::map<uint32_t, bool> m_outgoingReliable;

	std::map<uint32_t, size_t> m_incomingMetrics;

	std::map<uint32_t, size_t> m_outgoingMetrics;

	std::map<uint32_t, size_t> m_lastIncomingMetrics;

	std::map<uint32_t, size_t> m_lastOutgoingMetrics;

private:
	inline int GetOverlayLeft()
	{
		int x, y;
		GetGameResolution(x, y);

		if (g_netOverlayOffsetX < 0)
		{
			return x + g_netOverlayOffsetX - g_netOverlayWidth;
		}
		else
		{
			return g_netOverlayOffsetX;
		}
	}

	inline int GetOverlayTop()
	{
		int x, y;
		GetGameResolution(x, y);

		if (g_netOverlayOffsetY < 0)
		{
			return y + g_netOverlayOffsetY - g_netOverlayHeight;
		}
		else
		{
			return g_netOverlayOffsetY;
		}
	}

	ImColor GetColorIndex(int i);

	void UpdateMetrics();

	void DrawBaseMetrics();

	void DrawGraph();
};

NetOverlayMetricSink::NetOverlayMetricSink()
	: m_ping(0), m_lastInBytes(0), m_lastInPackets(0), m_lastOutBytes(0), m_lastOutPackets(0),
	  m_lastUpdatePerSample(0), m_lastUpdatePerSec(0),
	  m_inBytes(0), m_inPackets(0), m_outBytes(0), m_outPackets(0),
	  m_inRoutePackets(0), m_lastInRoutePackets(0), m_outRoutePackets(0), m_lastOutRoutePackets(0),
	  m_inRouteDelay(0), m_inRouteDelaySample(0), m_inRouteDelayMax(0), m_inRouteDelaySampleArchive(0),
	  m_enabled(false), m_enabledCommands(false)
{
	memset(m_inRouteDelaySamples, 0, sizeof(m_inRouteDelaySamples));
	memset(m_inRouteDelaySamplesArchive, 0, sizeof(m_inRouteDelaySamplesArchive));

	static ConVar<bool> conVar("netgraph", ConVar_Archive, false, &m_enabled);

	ConHost::OnShouldDrawGui.Connect([this](bool* should)
	{
		*should = *should || m_enabled;
	});

	ConHost::OnDrawGui.Connect([this]()
	{
		if (!m_enabled)
		{
			return;
		}

		auto& io = ImGui::GetIO();

		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x + g_netOverlayOffsetX, io.DisplaySize.y + g_netOverlayOffsetY), ImGuiCond_Once, ImVec2(1.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(g_netOverlayWidth, g_netOverlayHeight));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		if (ImGui::Begin("NetGraph", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
		{
			// draw the graph
			DrawGraph();

			// draw the base metrics
			DrawBaseMetrics();
		}

		ImGui::PopStyleVar();
		ImGui::End();
	});

	OnPostFrontendRender.Connect([=] ()
	{
		// update metrics
		UpdateMetrics();
	}, 50);

	static ConVar<bool> commandVar("net_showCommands", ConVar_Archive, false, &m_enabledCommands);

	ConHost::OnShouldDrawGui.Connect([this](bool* should)
	{
		*should = *should || m_enabledCommands;
	});

	ConHost::OnDrawGui.Connect([this]()
	{
		if (m_enabledCommands)
		{
			if (ImGui::Begin("Network Metrics"))
			{
				std::unique_lock<std::mutex> lock(m_metricMutex);

				static bool showIncoming = true;
				static bool showOutgoing = true;

				auto showList = [](const decltype(m_lastIncomingMetrics)& list, const decltype(m_incomingReliable)& reliable)
				{
					ImGui::Columns(3);

					for (auto& entry : list)
					{
						ImGui::Text("%s", (reliable.find(entry.first)->second ? "R" : "U"));
						ImGui::NextColumn();

						ImGui::Text("%s", g_hashes.LookupHash(entry.first));
						ImGui::NextColumn();

						ImGui::Text("%d B", entry.second);
						ImGui::NextColumn();
					}

					ImGui::Columns(1);
				};

				if (ImGui::CollapsingHeader("Incoming", &showIncoming))
				{
					showList(m_lastIncomingMetrics, m_incomingReliable);
				}

				if (ImGui::CollapsingHeader("Outgoing", &showOutgoing))
				{
					showList(m_lastOutgoingMetrics, m_outgoingReliable);
				}
			}

			ImGui::End();
		}
	});
}

void NetOverlayMetricSink::OnIncomingPacket(const NetPacketMetrics& packetMetrics)
{
	m_metrics[g_netOverlaySampleCount] = m_metrics[g_netOverlaySampleCount] + packetMetrics;

	m_inPackets++;
	m_inBytes += packetMetrics.GetTotalSize();

	m_inRoutePackets += packetMetrics.GetElementCount(NET_PACKET_SUB_ROUTED_MESSAGES);
}

void NetOverlayMetricSink::OnOutgoingPacket(const NetPacketMetrics& packetMetrics)
{
	m_outPackets++;
	m_outBytes += packetMetrics.GetTotalSize();

	m_outRoutePackets += packetMetrics.GetElementCount(NET_PACKET_SUB_ROUTED_MESSAGES);
}

void NetOverlayMetricSink::OnIncomingRoutePackets(int num)
{
	m_inRoutePackets += num;
}

void NetOverlayMetricSink::OnOutgoingRoutePackets(int num)
{
	m_outRoutePackets += num;
}

void NetOverlayMetricSink::OnPingResult(int msec)
{
	m_ping = msec;
}

void NetOverlayMetricSink::OnRouteDelayResult(int msec)
{
	// quick samples
	m_inRouteDelaySamples[m_inRouteDelaySample] = msec;
	m_inRouteDelaySample = (m_inRouteDelaySample + 1) % _countof(m_inRouteDelaySamples);

	// long archive
	m_inRouteDelaySamplesArchive[m_inRouteDelaySampleArchive] = msec;
	m_inRouteDelaySampleArchive = (m_inRouteDelaySampleArchive + 1) % _countof(m_inRouteDelaySamplesArchive);

	// calculate average
	int m = 1;

	for (int sample : m_inRouteDelaySamples)
	{
		m += sample;
	}

	m_inRouteDelay = m / _countof(m_inRouteDelaySamples);

	// calculate max
	m_inRouteDelayMax = *std::max_element(m_inRouteDelaySamplesArchive, m_inRouteDelaySamplesArchive + _countof(m_inRouteDelaySamplesArchive));
}

void NetOverlayMetricSink::OnIncomingCommand(uint32_t type, size_t size, bool reliable)
{
	std::unique_lock<std::mutex> lock(m_metricMutex);
	m_incomingMetrics[type] += size;
	m_incomingReliable[type] = reliable;
}

void NetOverlayMetricSink::OnOutgoingCommand(uint32_t type, size_t size, bool reliable)
{
	std::unique_lock<std::mutex> lock(m_metricMutex);
	m_outgoingMetrics[type] += size;
	m_outgoingReliable[type] = reliable;
}

// log data if enabled
static ConVar<std::string> netLogFile("net_statsFile", ConVar_Archive, "");

void NetOverlayMetricSink::OverrideBandwidthStats(uint32_t in, uint32_t out)
{
	m_lastInBytes = in;
	m_lastOutBytes = out;
}

void NetOverlayMetricSink::UpdateMetrics()
{
	uint32_t time = timeGetTime();

	if ((time - m_lastUpdatePerSample) > g_netOverlaySampleSize)
	{
		// move the metrics back by one
		std::copy(m_metrics + 1, m_metrics + _countof(m_metrics), m_metrics);

		// reset the first metric
		m_metrics[g_netOverlaySampleCount] = NetPacketMetrics();

		// update the timer
		m_lastUpdatePerSample = time;
	}

	if ((time - m_lastUpdatePerSec) > 1000)
	{
		// set 'last' values
		m_lastInBytes = m_inBytes;
		m_lastInPackets = m_inPackets;

		m_lastOutBytes = m_outBytes;
		m_lastOutPackets = m_outPackets;

		m_lastInRoutePackets = m_inRoutePackets;
		m_lastOutRoutePackets = m_outRoutePackets;

		// reset 'current' values
		m_inBytes = 0;
		m_inPackets = 0;

		m_outBytes = 0;
		m_outPackets = 0;

		m_inRoutePackets = 0;
		m_outRoutePackets = 0;

		// update the timer
		m_lastUpdatePerSec = time;

		// clear per-second metrics
		{
			std::unique_lock<std::mutex> lock(m_metricMutex);

			m_lastIncomingMetrics = std::move(m_incomingMetrics);
			m_lastOutgoingMetrics = std::move(m_outgoingMetrics);
		}

		// log output?
		auto netLog = netLogFile.GetValue();
		if (!netLog.empty())
		{
			// check if we want a new file
			static std::string lastNetLog;
			static uint32_t netLogInitTime;

			std::wstring netLogPath = MakeRelativeCitPath(ToWide(netLog));

			auto writeToNetLog = [&](const std::string& text)
			{
				FILE* f = _wfopen(netLogPath.c_str(), L"a");

				if (f)
				{
					fwrite(text.c_str(), text.size(), 1, f);
					fclose(f);
				}
			};

			if (lastNetLog != netLog)
			{
				// delete the file and write a header
				_wunlink(netLogPath.c_str());
				writeToNetLog("Time,Ping,InBytes,InPackets,OutBytes,OutPackets,InRoutePackets,OutRoutePackets\n");

				netLogInitTime = timeGetTime();

				lastNetLog = netLog;
			}

			writeToNetLog(fmt::sprintf("%d,%d,%d,%d,%d,%d,%d,%d\n",
				timeGetTime() - netLogInitTime, m_ping, m_lastInBytes, m_lastInPackets, m_lastOutBytes, m_lastOutPackets, m_lastInRoutePackets, m_lastOutRoutePackets));
		}
	}
}

void NetOverlayMetricSink::DrawGraph()
{
	static const char* names[NET_PACKET_SUB_MAX] =
	{
		"Routed Messages",
		"Reliables",
		"Misc",
		"Overhead"
	};

	static const ImColor colors[NET_PACKET_SUB_MAX] =
	{
		GetColorIndex(0),
		GetColorIndex(1),
		GetColorIndex(2),
		GetColorIndex(3)
	};

	struct DataContext
	{
		NetPacketMetrics* metrics;
		NetPacketSubComponent index;
	};

	auto data0 = DataContext{ m_metrics, NET_PACKET_SUB_ROUTED_MESSAGES };
	auto data1 = DataContext{ m_metrics, NET_PACKET_SUB_RELIABLES };
	auto data2 = DataContext{ m_metrics, NET_PACKET_SUB_MISC };
	auto data3 = DataContext{ m_metrics, NET_PACKET_SUB_OVERHEAD };

	const void* datas[NET_PACKET_SUB_MAX] =
	{
		&data0,
		&data1,
		&data2,
		&data3
	};

	ImGui::PlotMultiLines("Net Bw", NET_PACKET_SUB_MAX, names, colors, [](const void* cxt, int idx) -> float
	{
		auto dataContext = (DataContext*)cxt;

		return dataContext->metrics[idx].GetElementSize(dataContext->index);
	}, datas, _countof(m_metrics) - 1, FLT_MAX, FLT_MAX, ImVec2(g_netOverlayWidth, g_netOverlayHeight - 100));
}

ImColor NetOverlayMetricSink::GetColorIndex(int index)
{
	static CRGBA colorTable[] = {
		CRGBA(0x00, 0x00, 0xAA),
		CRGBA(0x00, 0xAA, 0x00),
		CRGBA(0x00, 0xAA, 0xAA),
		CRGBA(0xAA, 0x00, 0x00),
		CRGBA(0xAA, 0x00, 0xAA),
		CRGBA(0xAA, 0x55, 0x00),
		CRGBA(0x55, 0x55, 0xFF),
		CRGBA(0x55, 0xFF, 0x55),
		CRGBA(0x55, 0xFF, 0xFF),
		CRGBA(0xFF, 0x55, 0x55),
		CRGBA(0xFF, 0x55, 0xFF),
		CRGBA(0xFF, 0xFF, 0x55)
	};

	auto thisColor = colorTable[index % _countof(colorTable)];

	return ImColor{ thisColor.red, thisColor.green, thisColor.blue, thisColor.alpha };
}

void NetOverlayMetricSink::DrawBaseMetrics()
{
	// positioning
	int x = GetOverlayLeft();
	int y = GetOverlayTop() + (g_netOverlayHeight - 100) + 10;

	CRGBA color(255, 255, 255);
	CRect rect(x, y, x + (g_netOverlayWidth / 2), y + 100);

	// collecting
	int ping = m_ping;
	int inPackets = m_lastInPackets;
	int outPackets = m_lastOutPackets;
	int inRoutePackets = m_lastInRoutePackets;
	int outRoutePackets = m_lastOutRoutePackets;

	// drawing
	ImGui::Columns(2);
	ImGui::Text("%s", va("ping: %dms\nin: %d/s\nout: %d/s\nrt: %d/%d/s", ping, inPackets, outPackets, inRoutePackets, outRoutePackets));
	ImGui::NextColumn();

	//
	// second column
	//

	// positioning
	rect.SetRect(rect.fX2, rect.fY1, rect.fX2 + (g_netOverlayWidth / 2), rect.fY2);

	// collecting
	int inBytes = m_lastInBytes;
	int outBytes = m_lastOutBytes;
	int inRouteDelay = m_inRouteDelay;
	int inRouteDelayMax = m_inRouteDelayMax;

	// drawing
	ImGui::Text("%s", va("\nin: %d b/s\nout: %d b/s\nrd: %d~%dms", inBytes, outBytes, inRouteDelay, inRouteDelayMax));
	ImGui::Columns(1);
}

static InitFunction initFunction([] ()
{
	// register an event to create a network metric sink
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* netLibrary)
	{
		fwRefContainer<INetMetricSink> sink = new NetOverlayMetricSink();
		netLibrary->SetMetricSink(sink);
	});
});
