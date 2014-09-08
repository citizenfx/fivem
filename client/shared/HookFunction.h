#pragma once

//
// Initialization function that will be called after the game is loaded.
//

class HookFunction
{
private:
	void(*m_function)();

	HookFunction* m_next;

public:
	HookFunction(void(*function)())
	{
		m_function = function;

		Register();
	}

	static void RunAll();
	void Register();
};

class RuntimeHookFunction
{
private:
	void(*m_function)();
	std::string m_key;

	RuntimeHookFunction* m_next;

public:
	RuntimeHookFunction(const char* key, void(*function)())
	{
		m_key = key;
		m_function = function;

		Register();
	}

	static void Run(const char* key);
	void Register();
};