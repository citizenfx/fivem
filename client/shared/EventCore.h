#pragma once

#ifdef COMPILING_GAME
#define GAME_EXPORT_ __declspec(dllexport)
#else
#define GAME_EXPORT_
#endif

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_GAMESPEC) && !defined(COMPILING_HOOKS) && !defined(COMPILING_SHARED)
GAME_EXPORT_ void* fwAlloc(size_t size);
GAME_EXPORT_ void fwFree(void* ptr);
#elif defined(COMPILING_SHARED)
inline void* fwAllocSexy(size_t size)
{
	return malloc(size);
}

inline void fwFreeSexy(void* ptr)
{
	free(ptr);
}
#endif

template<class T>
class fwAllocator : public std::allocator<T>
{
public:
	typedef size_t size_type;
	typedef T* pointer;

	template<typename _Tp1>
	struct rebind
	{
		typedef fwAllocator<_Tp1> other;
	};

	pointer allocate(size_type n, const void *hint = 0)
	{
#ifndef COMPILING_SHARED
		return reinterpret_cast<pointer>(fwAlloc(n * sizeof(T)));
#else
		return reinterpret_cast<pointer>(fwAllocSexy(n * sizeof(T)));
#endif
	}

	void deallocate(pointer p, size_type n)
	{
#ifndef COMPILING_SHARED
		fwFree(p);
#else
		fwFreeSexy(p);
#endif
	}

	fwAllocator() throw() : std::allocator<T>() { }
	fwAllocator(const fwAllocator &a) throw() : std::allocator<T>(a) {}
	template <class U>
	fwAllocator(const fwAllocator<U> &a) throw() : std::allocator<T>(a) {}
	~fwAllocator() throw() {}

};

typedef std::basic_string<char, std::char_traits<char>, fwAllocator<char>> fwString;
typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, fwAllocator<wchar_t>> fwWString;

template<class TKey, class TValue>
class fwMap : public std::map<TKey, TValue, std::less<TKey>, fwAllocator<std::pair<const TKey, TValue>>>
{

};

template<class TKey, class TValue>
class fwHashMap : public std::unordered_map<TKey, TValue, std::less<TKey>, fwAllocator<std::pair<const TKey, TValue>>>
{

};

#include <functional>

template<class T>
class __declspec(dllexport) fwRefContainer
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
		m_ref = other.GetRef();
		m_ref->AddRef();

		return *this;
	}

	template<class TOther>
	fwRefContainer& operator=(const fwRefContainer<TOther>& other)
	{
		m_ref = other.GetRef();
		m_ref->AddRef();

		return *this;
	}

	fwRefContainer& operator=(T* ref)
	{
		if (m_ref)
		{
			m_ref->Release();
		}

		m_ref = ref;
		m_ref->AddRef();

		return *this;
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

	virtual bool Release();
};

template<typename... Args>
class __declspec(dllexport) fwActionImpl : public fwRefCountable
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
class __declspec(dllexport) fwAction : public fwRefContainer<fwActionImpl<Args...>>
{
public:
	fwAction()
		: fwRefContainer()
	{

	}

	fwAction(const std::function<void(Args...)>& stlFunc)
		: fwRefContainer(new fwActionImpl<Args...>(stlFunc))
	{

	}

	template<typename Fx>
	fwAction(const Fx& func)
		: fwRefContainer(new fwActionImpl<Args...>(std::function<void(Args...)>(func)))
	{

	}

	void operator()(Args... args)
	{
		GetRef()->Invoke(args...);
	}
};

template<typename... Args>
class __declspec(dllexport) fwEvent
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

	/*inline void Connect(std::function<void(Args...)> func)
	{
		Connect(fwAction<Args...>(func));
	}*/

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