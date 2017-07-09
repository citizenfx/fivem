#pragma once

namespace se
{
class Principal
{
public:
	inline explicit Principal(const std::string& principal)
	{
		m_identifier = principal;
	}

	inline const std::string& GetIdentifier() const
	{
		return m_identifier;
	}

	inline bool operator<(const Principal& right) const
	{
		return stricmp(m_identifier.c_str(), right.m_identifier.c_str()) < 0;
	}

private:
	std::string m_identifier;
};

class Object
{
public:
	inline explicit Object(const std::string& identifier)
	{
		m_identifier = identifier;
	}

	inline const std::string& GetIdentifier() const
	{
		return m_identifier;
	}

	inline bool operator<(const Object& right) const
	{
		return stricmp(m_identifier.c_str(), right.m_identifier.c_str()) < 0;
	}

private:
	std::string m_identifier;
};

enum class AccessType
{
	Allow,
	Unset,
	Deny
};

class ContextImpl;

class Context : public fwRefCountable
{
public:
	Context();

	virtual ~Context();

	virtual void Reset();

	virtual void LoadSnapshot(const std::string& snapshot);

	virtual std::string SaveSnapshot();

	virtual void AddPrincipalInheritance(const Principal& child, const Principal& parent);

	virtual void AddAccessControlEntry(const Principal& principal, const Object& object, AccessType type);

	virtual bool CheckPrivilege(const Object& object);

	virtual bool CheckPrivilege(const Principal& principal, const Object& object);

	virtual void PushPrincipal(const Principal& principal);

	virtual void PopPrincipal();

	virtual void MakeCurrent();

private:
	std::unique_ptr<ContextImpl> m_impl;
};
}


#ifdef COMPILING_CORE
extern "C" DLL_EXPORT se::Context* seGetCurrentContext();
extern "C" DLL_EXPORT void seCreateContext(fwRefContainer<se::Context>* outContext);
#elif defined(_WIN32)
inline se::Context* seGetCurrentContext()
{
	using TCoreFunc = decltype(&seGetCurrentContext);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "seGetCurrentContext");
	}

	return (func) ? func() : 0;
}

inline void seCreateContext(fwRefContainer<se::Context>* outContext)
{
	using TCoreFunc = decltype(&seCreateContext);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "seCreateContext");
	}

	return (func) ? func(outContext) : nullptr;
}
#else
extern "C" se::Context* seGetCurrentContext();
extern "C" void seCreateContext(fwRefContainer<se::Context>* outContext);
#endif

inline bool seCheckPrivilege(const std::string& object)
{
	return seGetCurrentContext()->CheckPrivilege(se::Object{ object });
}

namespace se
{
class ScopedPrincipal
{
public:
	ScopedPrincipal(const Principal& principal)
	{
		seGetCurrentContext()->PushPrincipal(principal);
	}

	~ScopedPrincipal()
	{
		seGetCurrentContext()->PopPrincipal();
	}
};
}
