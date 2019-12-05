#pragma once

#include <CoreConsole.h>

#include <shared_mutex>
#include <unordered_map>
#include <TokenBucket.h>

#include <NetAddress.h>

#include <tbb/concurrent_unordered_map.h>

namespace fx
{
template<typename TKey>
struct KeyMangler
{
	TKey operator()(const TKey& key)
	{
		return key;
	}
};

template<>
struct KeyMangler<net::PeerAddress>
{
	size_t operator()(const net::PeerAddress& address)
	{
		auto sockaddr = address.GetSocketAddress();

		if (sockaddr->sa_family == AF_INET)
		{
			auto in = (sockaddr_in*)sockaddr;
			return std::hash<uint32_t>()(in->sin_addr.s_addr);
		}
		else if (sockaddr->sa_family == AF_INET6)
		{
			auto in6 = (sockaddr_in6*)sockaddr;
			return std::hash<std::string_view>()(std::string_view{ reinterpret_cast<char*>(&in6->sin6_addr), sizeof(in6->sin6_addr) });
		}

		return std::hash<std::string>()(address.GetHost());
	}
};

template<typename TKey>
class KeyedRateLimiter
{
private:
	using TBucket = folly::TokenBucket;
	using TMangledKey = std::result_of_t<KeyMangler<TKey>(TKey)>;

public:
	explicit KeyedRateLimiter(double genRate, double burstSize)
		: m_genRate(genRate), m_burstSize(burstSize)
	{

	}

	void Update(double genRate, double burstSize)
	{
		if (m_genRate != genRate || m_burstSize != burstSize)
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			m_genRate = genRate;
			m_burstSize = burstSize;

			for (auto& bucket : m_buckets)
			{
				bucket.second.reset(genRate, burstSize);
			}
		}
	}

	bool Consume(const TKey& key, double n = 1.0)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		auto mangled = KeyMangler<TKey>()(key);
		auto it = m_buckets.find(mangled);

		if (it == m_buckets.end())
		{
			it = m_buckets.emplace(mangled, TBucket{m_genRate, m_burstSize}).first;
		}

		return it->second.consume(n);
	}

	void Reset(const TKey& key)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		auto mangled = KeyMangler<TKey>()(key);
		auto it = m_buckets.find(mangled);

		if (it != m_buckets.end())
		{
			it->second.reset(m_genRate, m_burstSize);
		}
	}

private:
	std::unordered_map<TMangledKey, TBucket> m_buckets;
	std::mutex m_mutex;

	double m_genRate;
	double m_burstSize;
};

struct RateLimiterDefaults
{
	double rate;
	double burst;

	inline explicit RateLimiterDefaults(double rate, double burst)
		: rate(rate), burst(burst)
	{

	}
};

template<typename TKey>
class RateLimiterStore : public fwRefCountable
{
private:
	using TRateLimiter = KeyedRateLimiter<TKey>;

	struct RateLimiter
	{
		inline RateLimiter(console::Context* console, const std::string& key, RateLimiterDefaults defaults)
			: rateVar(console, fmt::sprintf("rateLimiter_%s_rate", key), ConVar_None, defaults.rate),
			  burstVar(console, fmt::sprintf("rateLimiter_%s_burst", key), ConVar_None, defaults.burst),
			  limiter(rateVar.GetValue(), burstVar.GetValue())
		{
			
		}

		inline TRateLimiter& GetLimiter()
		{
			limiter.Update(rateVar.GetValue(), burstVar.GetValue());

			return limiter;
		}

	private:
		ConVar<float> rateVar;
		ConVar<float> burstVar;
		TRateLimiter limiter;
	};

public:
	RateLimiterStore(console::Context* context)
	{
		m_console = context;
	}

	TRateLimiter* GetRateLimiter(const std::string& key, RateLimiterDefaults defaults)
	{
		auto it = m_rateLimiters.find(key);

		if (it == m_rateLimiters.end())
		{
			auto rateLimiter = std::make_shared<RateLimiter>(m_console, key, defaults);

			it = m_rateLimiters.insert({ key, rateLimiter }).first;
		}

		return &it->second->GetLimiter();
	}

private:
	tbb::concurrent_unordered_map<std::string, std::shared_ptr<RateLimiter>> m_rateLimiters;

	console::Context* m_console;
};

using PeerAddressRateLimiterStore = RateLimiterStore<net::PeerAddress>;
}

DECLARE_INSTANCE_TYPE(fx::PeerAddressRateLimiterStore);
