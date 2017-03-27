/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

/* Interface for network metric sinks. */

#pragma once

#include <numeric>

class NetPacketMetrics;

class INetMetricSink : public fwRefCountable
{
public:
	virtual void OnIncomingPacket(const NetPacketMetrics& packetMetrics) = 0;

	virtual void OnOutgoingPacket(const NetPacketMetrics& packetMetrics) = 0;

	virtual void OnPingResult(int msec) = 0;

	virtual void OnRouteDelayResult(int msec) = 0;
};

enum NetPacketSubComponent
{
	NET_PACKET_SUB_ROUTED_MESSAGES,
	NET_PACKET_SUB_RELIABLES,
	NET_PACKET_SUB_MISC,
	NET_PACKET_SUB_MAX
};

class NetPacketMetrics
{
private:
	uint32_t m_subSizes[NET_PACKET_SUB_MAX];

	uint32_t m_subCounts[NET_PACKET_SUB_MAX];

public:
	inline NetPacketMetrics()
	{
		memset(m_subSizes, 0, sizeof(m_subSizes));
		memset(m_subCounts, 0, sizeof(m_subCounts));
	}

	inline uint32_t GetTotalSize() const
	{
		return std::accumulate(m_subSizes, m_subSizes + _countof(m_subSizes), 0);
	}

	inline uint32_t GetElementCount(NetPacketSubComponent index) const
	{
		return m_subCounts[index];
	}

	inline uint32_t GetElementSize(NetPacketSubComponent index) const
	{
		return m_subSizes[index];
	}

	inline void SetElementSize(NetPacketSubComponent index, uint32_t value)
	{
		m_subSizes[index] = value;
	}

	inline void AddElementSize(NetPacketSubComponent index, uint32_t value)
	{
		++m_subCounts[index];
		m_subSizes[index] += value;
	}
};

inline NetPacketMetrics operator+(const NetPacketMetrics& left, const NetPacketMetrics& right)
{
	NetPacketMetrics retval;
	
	for (int i = 0; i < NET_PACKET_SUB_MAX; i++)
	{
		NetPacketSubComponent sub = (NetPacketSubComponent)i;

		retval.SetElementSize(sub, left.GetElementSize(sub) + right.GetElementSize(sub));
	}

	return retval;
}