#pragma once

//
// Returns the Citizen root directory.
//

std::wstring GetAbsoluteCitPath();

//
// Returns the game root directory.
//

std::wstring GetAbsoluteGamePath();

//
// Returns a said path relative to the Citizen base installation directory (containing the launcher executable).
//

inline std::wstring MakeRelativeCitPath(std::wstring targetPath)
{
	return GetAbsoluteCitPath() + targetPath;
}

//
// Returns a said path relative to the game installation directory (containing the target executable).
//

inline std::wstring MakeRelativeGamePath(std::wstring targetPath)
{
	return GetAbsoluteGamePath() + targetPath;
}

//
// Base initialization function (for running on library init and so on)
//

class InitFunctionBase
{
private:
	InitFunctionBase* m_next;

public:
	virtual void Run() = 0;

	void Register();

	static void RunAll();
};

//
// Initialization function that will be called around initialization of the primary component.
//

class InitFunction : public InitFunctionBase
{
private:
	void(*m_function)();

public:
	InitFunction(void(*function)())
	{
		m_function = function;

		Register();
	}

	virtual void Run()
	{
		m_function();
	}
};

//
// 'compile-time' hashing
//

template <unsigned int N, unsigned int I>
struct FnvHash
{
	FORCEINLINE static unsigned int Hash(const char(&str)[N])
	{
		return (FnvHash<N, I - 1>::Hash(str) ^ str[I - 1]) * 16777619u;
	}
};

template <unsigned int N>
struct FnvHash<N, 1>
{
	FORCEINLINE static unsigned int Hash(const char(&str)[N])
	{
		return (2166136261u ^ str[0]) * 16777619u;
	}
};

class StringHash
{
private:
	unsigned int m_hash;

public:
	template <unsigned int N>
	FORCEINLINE StringHash(const char(&str)[N])
		: m_hash(FnvHash<N, N>::Hash(str))
	{
	}

	FORCEINLINE operator unsigned int() const
	{
		return m_hash;
	}
};

#define CALL_NO_ARGUMENTS(addr) ((void(*)())addr)()
#define EAXJMP(a) { _asm mov eax, a _asm jmp eax }
#define WRAPPER __declspec(naked)

//
// export class specifiers
//

#ifdef COMPILING_GAME
#define GAME_EXPORT __declspec(dllexport)
#else
#define GAME_EXPORT __declspec(dllimport)
#endif

#ifdef COMPILING_HOOKS
#define HOOKS_EXPORT __declspec(dllexport)
#else
#define HOOKS_EXPORT __declspec(dllimport)
#endif

#ifdef COMPILING_GAMESPEC
#define GAMESPEC_EXPORT __declspec(dllexport)
#else
#define GAMESPEC_EXPORT __declspec(dllimport)
#endif