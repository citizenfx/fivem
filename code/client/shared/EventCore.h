/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

inline void* fwAlloc(size_t size)
{
    return malloc(size);
}

inline void fwFree(void* p)
{
    free(p);
}

typedef std::string fwString;
typedef std::wstring fwWString;

template<class TValue>
using fwVector = std::vector<TValue>;

template<class TValue>
using fwList = std::list<TValue>;

template<class TKey, class TValue>
using fwHashMap = std::unordered_map<TKey, TValue>;

template<class TKey, class TValue>
using fwMap = std::map<TKey, TValue>;

#include <functional>

template<class T>
class fwRefContainer
{
private:
	T* m_ref;

public:
	fwRefContainer()
		: m_ref(nullptr)
	{
		
	}

	fwRefContainer(T* ref)
		: m_ref(ref)
	{
		if (m_ref)
		{
			m_ref->AddRef();
		}
	}

	~fwRefContainer()
	{
		if (m_ref)
		{
			if (m_ref->Release())
			{
				m_ref = nullptr;
			}
		}
	}

	fwRefContainer(const fwRefContainer& rc)
	{
		m_ref = rc.m_ref;

		if (m_ref)
		{
			m_ref->AddRef();
		}
	}

	template<typename TOther>
	fwRefContainer(const fwRefContainer<TOther>& rc)
	{
		m_ref = static_cast<T*>(rc.GetRef());

		if (m_ref)
		{
			m_ref->AddRef();
		}
	}

	uint32_t GetRefCount() const
	{
		return m_ref->GetRefCount();
	}

	T* GetRef() const
	{
		return m_ref;
	}

	T* operator->() const
	{
		return m_ref;
	}

	fwRefContainer& operator=(const fwRefContainer& other)
	{
		if (m_ref)
		{
			m_ref->Release();
		}

		m_ref = other.GetRef();

		if (m_ref)
		{
			m_ref->AddRef();
		}

		return *this;
	}

	template<class TOther>
	fwRefContainer& operator=(const fwRefContainer<TOther>& other)
	{
		if (m_ref)
		{
			m_ref->Release();
		}

		m_ref = other.GetRef();

		if (m_ref)
		{
			m_ref->AddRef();
		}

		return *this;
	}

	fwRefContainer& operator=(T* ref)
	{
		if (m_ref)
		{
			m_ref->Release();
		}

		m_ref = ref;

		if (m_ref)
		{
			m_ref->AddRef();
		}

		return *this;
	}
};

template<typename T>
bool operator<(const fwRefContainer<T>& left, const fwRefContainer<T>& right)
{
	return (left.GetRef() < right.GetRef());
}

class EXPORTED_TYPE fwRefCountable
{
private:
	class RefCount
	{
	private:
		std::atomic<uint32_t> m_count;

	public:
		RefCount()
			: m_count(0)
		{
		}

		inline std::atomic<uint32_t>& GetCount()
		{
			return m_count;
		}
	};

	RefCount m_refCount;

public:
	inline uint32_t GetRefCount()
	{
		return m_refCount.GetCount();
	}

	virtual ~fwRefCountable();

	virtual void AddRef();

	virtual bool Release();
};

template<typename... Args>
class fwActionImpl : public fwRefCountable
{
private:
	std::function<void(Args&...)> m_func;

public:
	fwActionImpl(const std::function<void(Args...)>& stlFunc)
	{
		m_func = stlFunc;
	}

	virtual void Invoke(Args&... args)
	{
		m_func(args...);
	}
};

template<typename... Args>
class fwAction : public fwRefContainer<fwActionImpl<Args...>>
{
public:
	fwAction()
		: fwRefContainer<fwActionImpl<Args...>>()
	{

	}

	fwAction(const std::function<void(Args...)>& stlFunc)
		: fwRefContainer<fwActionImpl<Args...>>(new fwActionImpl<Args...>(stlFunc))
	{

	}

	template<typename Fx>
	fwAction(const Fx& func)
		: fwRefContainer<fwActionImpl<Args...>>(new fwActionImpl<Args...>(std::function<void(Args...)>(func)))
	{

	}

	void operator()(Args... args)
	{
		this->GetRef()->Invoke(args...);
	}
};

template<typename... Args>
class fwEvent
{
public:
	using TFunc = std::function<bool(Args...)>;

private:
	struct callback
	{
		TFunc function;
		std::unique_ptr<callback> next = nullptr;
		int order = 0;
		size_t cookie = -1;

		callback(TFunc func)
			: function(func)
		{

		}
	};

	std::unique_ptr<callback> m_callbacks;
	std::atomic<size_t> m_connectCookie = 0;

public:
	fwEvent()
	{
		m_callbacks = nullptr;
	}

	~fwEvent()
	{
		Reset();
	}

	template<typename T>
	auto Connect(T func)
	{
		return Connect(func, 0);
	}

	template<typename T>
	auto Connect(T func, int order)
	{
		if constexpr (std::is_same_v<std::invoke_result_t<T, Args...>, bool>)
		{
			return ConnectInternal(func, order);
		}
		else
		{
			return ConnectInternal([func](Args&&... args)
			{
				std::invoke(func, args...);
				return true;
			},
			order);
		}
	}

	void Reset()
	{
		m_callbacks.reset();
	}

private:
	size_t ConnectInternal(TFunc func, int order)
	{
		if (!func)
		{
			return -1;
		}

		auto cookie = m_connectCookie++;
		auto cb = std::unique_ptr<callback>(new callback(func));
		cb->order = order;
		cb->cookie = cookie;

		if (!m_callbacks)
		{
			m_callbacks = std::move(cb);
		}
		else
		{
			auto cur = &m_callbacks;
			callback* last = nullptr;

			while (*cur && order >= (*cur)->order)
			{
				last = cur->get();
				cur = &(*cur)->next;
			}

			cb->next = std::move(*cur);
			(!last ? m_callbacks : last->next) = std::move(cb);
		}

		return cookie;
	}

public:
	void Disconnect(size_t cookie)
	{
		if (cookie == -1)
		{
			return;
		}

		callback* prev = nullptr;

		for (auto cb = m_callbacks.get(); cb; cb = cb->next.get())
		{
			if (cb->cookie == cookie)
			{
				if (prev)
				{
					prev->next = std::move(cb->next);
				}
				else
				{
					m_callbacks = std::move(cb->next);
				}

				break;
			}

			prev = cb;
		}
	}

	auto ConnectInternal(TFunc func)
	{
		return ConnectInternal(func, 0);
	}

public:
	operator bool() const
	{
		return m_callbacks.get() != nullptr;
	}

	bool operator()(Args... args) const
	{
		if (!m_callbacks)
		{
			return true;
		}

		for (auto cb = m_callbacks.get(); cb; cb = cb->next.get())
		{
			if (!std::invoke(cb->function, args...))
			{
				return false;
			}
		}

		return true;
	}
};
