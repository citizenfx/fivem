#pragma once

struct GlobalInputHandler
{
	virtual ~GlobalInputHandler() = default;

	fwEvent<DWORD, bool> OnKey;
};

extern std::shared_ptr<GlobalInputHandler> CreateGlobalInputHandler();
