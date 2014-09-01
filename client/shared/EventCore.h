#pragma once

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
		m_ref->AddRef();
	}

	~fwRefContainer()
	{
		m_ref->Release();
	}

	fwRefContainer(const fwRefContainer& rc)
	{
		m_ref = rc.m_ref;
		m_ref->AddRef();
	}

	T* GetRef() const
	{
		return m_ref;
	}

	T* operator->() const
	{
		return m_ref;
	}

	template<class TOther>
	void operator=(const fwRefContainer<TOther>& other)
	{
		m_ref = other.GetRef();
		m_ref->AddRef();
	}
};

class fwRefCountable
{
private:
	class RefCount
	{
	private:
		uint32_t m_count;

	public:
		RefCount()
			: m_count(0)
		{
		}

		inline uint32_t& GetCount()
		{
			return m_count;
		}
	};

	RefCount m_refCount;

public:
	virtual ~fwRefCountable();

	virtual void AddRef();

	virtual void Release();
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
	fwAction(const std::function<void(Args...)>& stlFunc)
		: fwRefContainer(new fwActionImpl<Args...>(stlFunc))
	{

	}

	void operator()(Args... args)
	{
		GetRef()->Invoke(args...);
	}
};

template<typename... Args>
class fwEvent
{
public:
	typedef fwAction<Args...> TFunc;

private:
	struct callback
	{
		TFunc function;
		callback* next;

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

	void Connect(TFunc func)
	{
		auto cb = new callback(func);
		cb->next = m_callbacks;

		m_callbacks = cb;
	}

	void Connect(std::function<void(Args...)> func)
	{
		Connect(fwAction<Args...>(func));
	}

	void operator()(Args... args)
	{
		if (!m_callbacks)
		{
			return;
		}

		for (callback* cb = m_callbacks; cb; cb = cb->next)
		{
			cb->function(args...);
		}
	}
};

GAMESPEC_EXPORT fwEvent<int, int> TestEvent;