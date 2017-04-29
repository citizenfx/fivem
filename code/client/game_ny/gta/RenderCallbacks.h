#pragma once

class GAMESPEC_EXPORT RenderCallbacks
{
public:
	static void AddRenderCallback(const char* name, void(*callback)());
};