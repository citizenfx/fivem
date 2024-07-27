#pragma once

#include <CoreConsole.h>

#include <shared_mutex>
#include <TokenBucket.h>

#include <NetAddress.h>

#include <tbb/concurrent_unordered_map.h>

#include "EASTL/bonus/lru_cache.h"

namespace eastl
{
template <>
struct hash<net::PeerAddress>
{
	size_t operator()(const net::PeerAddress& address) const
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

template <>
struct hash<std::string>
{
	size_t operator()(const std::string& str) const
	{
		return std::hash<std::string>()(str);
	}
};
}

namespace fx
{
// lru cache requires a default ctor for the value to compile
struct Bucket
{
	folly::TokenBucket bucket;

	Bucket(double genRate, double burstSize)
		: bucket(genRate, burstSize)
	{
	}

	Bucket(): bucket {1, 1}
	{
	}

	void Reset(double genRate, double burstSize)
	{
		bucket.reset(genRate, burstSize);
	}

	bool Consume(double toConsume)
	{
		return bucket.consume(toConsume);
	}

	void ReturnTokens(double tokensToReturn)
	{
		bucket.returnTokens(tokensToReturn);
	}
};

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

template<typename TKey, bool Cooldown = false>
class KeyedRateLimiter
{
private:
	static constexpr size_t MAX_KEYS = 4000;

	using TBucket = Bucket;
	using TMangledKey = std::result_of_t<KeyMangler<TKey>(TKey)>;

public:
	explicit KeyedRateLimiter(double genRate, double burstSize)
		: m_genRate(genRate), m_burstSize(burstSize)
	{

	}

	void Update(double genRate, double burstSize)
	{
		if (genRate <= 0.01 || burstSize <= 0.01)
		{
			return;
		}

		if (m_genRate != genRate || m_burstSize != burstSize)
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			m_genRate = genRate;
			m_burstSize = burstSize;

			for (auto& bucket : m_buckets)
			{
				bucket.second.first.Reset(genRate, burstSize);
			}
		}
	}

	void ReturnToken(const TKey& key, double n = 1.0)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		auto mangled = KeyMangler<TKey>()(key);

		TBucket& bucket = m_buckets.get(mangled);
		bucket.ReturnTokens(n);
	}

	bool Consume(const TKey& key, double n = 1.0, bool* isCooldown = nullptr)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		auto mangled = KeyMangler<TKey>()(key);

		if (Cooldown)
		{
			eastl::optional<std::chrono::milliseconds> cooldown = m_cooldowns.at(mangled);

			if (cooldown != eastl::nullopt)
			{
				if (std::chrono::high_resolution_clock::now().time_since_epoch() <= *cooldown)
				{
					if (isCooldown)
					{
						*isCooldown = true;
					}

					return false;
				}

				m_cooldowns.erase(mangled);
			}
		}

		TBucket& bucket = m_buckets.get(mangled);
		bool valid = bucket.Consume(n);

		if (Cooldown && !valid)
		{
			bucket = m_cooldownBuckets.get(mangled);
			bool cooldownValid = bucket.Consume(n);

			if (!cooldownValid)
			{
				m_cooldowns.insert_or_assign(mangled, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch() + std::chrono::seconds(15)));
			}
		}

		return valid;
	}

	void Reset(const TKey& key)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		auto mangled = KeyMangler<TKey>()(key);

		if (m_buckets.contains(mangled))
		{
			m_buckets.get(mangled).Reset(m_genRate, m_burstSize);
		}
	}
private:
	std::mutex m_mutex;

	double m_genRate;
	double m_burstSize;

	eastl::function<TBucket(TMangledKey)> m_createTBucket = [this](TMangledKey) { return TBucket{ m_genRate, m_burstSize }; };
	eastl::function<TBucket(TMangledKey)> m_createTCooldownBucket = [this](TMangledKey) { return TBucket{ m_genRate * 1.5, m_burstSize * 1.5 }; };

	eastl::lru_cache<TMangledKey, TBucket> m_buckets {MAX_KEYS, eastl::allocator(EASTL_LRUCACHE_DEFAULT_NAME), m_createTBucket};
	eastl::lru_cache<TMangledKey, TBucket> m_cooldownBuckets {MAX_KEYS, eastl::allocator(EASTL_LRUCACHE_DEFAULT_NAME), m_createTCooldownBucket};
	eastl::lru_cache<TMangledKey, std::chrono::milliseconds> m_cooldowns {MAX_KEYS};
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

template<typename TKey, bool Cooldown = false>
class RateLimiterStore : public fwRefCountable
{
private:
	using TRateLimiter = KeyedRateLimiter<TKey, Cooldown>;

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

using PeerAddressRateLimiterStore = RateLimiterStore<net::PeerAddress, true>;

class TokenRateLimiter : public fwRefCountable, public fx::KeyedRateLimiter<std::string, false>
{
public:
	TokenRateLimiter(double a, double b)
		: KeyedRateLimiter(a, b)
	{
	}
};
}

DECLARE_INSTANCE_TYPE(fx::TokenRateLimiter);
DECLARE_INSTANCE_TYPE(fx::PeerAddressRateLimiterStore);
