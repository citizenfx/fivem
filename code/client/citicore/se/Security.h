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
		return _stricmp(m_identifier.c_str(), right.m_identifier.c_str()) < 0;
	}

	inline bool operator==(const Principal& right) const
	{
		return _stricmp(m_identifier.c_str(), right.m_identifier.c_str()) == 0;
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
		return _stricmp(m_identifier.c_str(), right.m_identifier.c_str()) < 0;
	}

	inline bool operator==(const Object& right) const
	{
		return _stricmp(m_identifier.c_str(), right.m_identifier.c_str()) == 0;
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

	virtual void RemovePrincipalInheritance(const Principal& child, const Principal& parent);

	virtual void AddAccessControlEntry(const Principal& principal, const Object& object, AccessType type);

	virtual void RemoveAccessControlEntry(const Principal& principal, const Object& object, AccessType type);

	virtual void ForAllPrincipalInheritances(const std::function<void(const Principal&, const Principal&)>&);

	virtual void ForAllAccessControlEntries(const std::function<void(const Principal&, const Object&, AccessType)>&);

	virtual bool CheckPrivilege(const Object& object);

	virtual bool CheckPrivilege(const Principal& principal, const Object& object);

	virtual void PushPrincipal(Principal& principal);

	virtual void PopPrincipal();

	virtual void PushPrincipalReset();

	virtual void PopPrincipalReset();

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

	assert(func);

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

	return (func) ? func(outContext) : (void)nullptr;
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
	inline ScopedPrincipal(Principal& principal)
		: m_principal(std::string{})
	{
		seGetCurrentContext()->PushPrincipal(principal);
	}

	inline ScopedPrincipal(const Principal& principal)
		: m_principal(principal)
	{
		seGetCurrentContext()->PushPrincipal(m_principal);
	}

	inline ScopedPrincipal(const ScopedPrincipal&) = delete;

	inline ScopedPrincipal(ScopedPrincipal&&) = delete;

	inline ~ScopedPrincipal()
	{
		seGetCurrentContext()->PopPrincipal();
	}

private:
	Principal m_principal;
};

class ScopedPrincipalReset
{
public:
	inline ScopedPrincipalReset()
	{
		seGetCurrentContext()->PushPrincipalReset();
	}

	inline ScopedPrincipalReset(const ScopedPrincipalReset&) = delete;

	inline ScopedPrincipalReset(ScopedPrincipalReset&&) = delete;

	inline ~ScopedPrincipalReset()
	{
		seGetCurrentContext()->PopPrincipalReset();
	}
};
}
