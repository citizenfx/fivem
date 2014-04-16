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