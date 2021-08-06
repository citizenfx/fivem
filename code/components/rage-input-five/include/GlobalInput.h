#pragma once

struct GlobalInputHandler
{
	virtual ~GlobalInputHandler() = default;

	fwEvent<DWORD, bool> OnKey;
	fwEvent<RAWMOUSE> OnMouse;
};

extern std::shared_ptr<GlobalInputHandler> CreateGlobalInputHandler();
