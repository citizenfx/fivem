#pragma once

#include <memory>

#include <LauncherIPC.h>

namespace citizen
{
class GameImplBase
{
public:
	virtual ~GameImplBase() = default;

	virtual ipc::Endpoint& GetIPC() = 0;
};

class Game
{
public:
	Game();

	void Run();

	inline GameImplBase* GetImpl()
	{
		return m_impl.get();
	}

private:
	std::unique_ptr<GameImplBase> m_impl;
};
}
