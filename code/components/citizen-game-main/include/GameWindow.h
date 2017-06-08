#pragma once

namespace citizen
{
class Game;

class GameWindow
{
public:
	virtual ~GameWindow() = default;
	
	virtual void Close() = 0;

	virtual void Render() = 0;

	virtual void ProcessEvents() = 0;

	virtual void ProcessEventsOnce() = 0;

	virtual void GetMetrics(int& width, int& height) = 0;

	virtual void* GetNativeHandle() = 0;

public:
	static std::unique_ptr<GameWindow> Create(const std::string& title, int defaultWidth, int defaultHeight, Game* game);
};
}
