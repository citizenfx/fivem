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

class fwRefCountable
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

template<bool IsBoolean>
struct fwEventConnectProxy
{
	template<typename... Args>
	struct Internal
	{
		template<typename TEvent, typename TFunc>
		static void Proxy(TEvent& event, TFunc func, int order)
		{
			event.ConnectInternal(func, order);
		}
	};
};

template<>
struct fwEventConnectProxy<false>
{
	template<typename... Args>
	struct Internal
	{
		template<typename TEvent, typename TFunc>
		static void Proxy(TEvent& event, TFunc func, int order)
		{
			event.ConnectInternal([=] (Args... args)
			{
				func(args...);
				return true;
			}, order);
		}
	};
};

template<typename... Args>
class fwEvent
{
public:
	friend struct fwEventConnectProxy<true>;
	friend struct fwEventConnectProxy<false>;

	typedef std::function<bool(Args...)> TFunc;

private:
	struct callback
	{
		TFunc function;
		callback* next;
		int order;

		callback(TFunc func)
			: function(func)
		{

		}
	};

	callback* m_callbacks;

public:
	fwEvent()
	{
		m_callbacks = nullptr;
	}

	~fwEvent()
	{
		callback* cb = m_callbacks;

		while (cb)
		{
			callback* curCB = cb;

			cb = cb->next;

			delete curCB;
		}
	}

	template<typename T>
	void Connect(T func)
	{
		fwEventConnectProxy<std::is_same<typename std::result_of<decltype(&T::operator())(T, Args...)>::type, bool>::value>::template Internal<Args...>::Proxy(*this, func, 0);
	}

	template<typename T>
	void Connect(T func, int order)
	{
		fwEventConnectProxy<std::is_same<typename std::result_of<decltype(&T::operator())(T, Args...)>::type, bool>::value>::template Internal<Args...>::Proxy(*this, func, order);
	}

private:
	void ConnectInternal(TFunc func, int order)
	{
		auto cb = new callback(func);
		cb->order = order;

		if (!m_callbacks)
		{
			cb->next = nullptr;
			m_callbacks = cb;
		}
		else
		{
			callback* cur = m_callbacks;
			callback* last = nullptr;

			while (cur && order >= cur->order)
			{
				last = cur;
				cur = cur->next;
			}

			cb->next = cur;

			(!last ? m_callbacks : last->next) = cb;
		}
	}

	void ConnectInternal(TFunc func)
	{
		ConnectInternal(func, 0);
	}

public:
	bool operator()(Args... args)
	{
		if (!m_callbacks)
		{
			return true;
		}

		for (callback* cb = m_callbacks; cb; cb = cb->next)
		{
			if (!cb->function(args...))
			{
				return false;
			}
		}

		return true;
	}
};