#pragma once

#include <EventCore.h>;

struct SDKGameProcessManager {
public:
	enum class GameProcessState {
		GP_STOPPED,
		GP_STARTING,
		GP_RUNNING,
		GP_STOPPING,
	};

	void StartGame();
	void StopGame();

	inline GameProcessState GetGameProcessState()
	{
		return gameProcessState;
	}

	fwEvent<GameProcessState> OnGameProcessStateChanged;

private:
	GameProcessState gameProcessState = GameProcessState::GP_STOPPED;

	PROCESS_INFORMATION gameProcessInfo = { 0 };

	inline void SetGameProcessState(GameProcessState state)
	{
		gameProcessState = state;
		OnGameProcessStateChanged(state);
	}
};
