#pragma once

namespace fx
{
class ServerInstanceBase;
}

class SvGuiModule
{
public:
	inline SvGuiModule(const std::string& name, const std::string& toggleName, const std::function<void(fx::ServerInstanceBase*)>& render)
		: name(name), toggleName(toggleName), render(render)
	{
		Register(this);
	}

	inline void Render(fx::ServerInstanceBase* instance)
	{
		render(instance);
	}

	std::string name;
	bool toggleVar = false;

private:
	std::string toggleName;
	std::function<void(fx::ServerInstanceBase*)> render;

	static void Register(SvGuiModule* module);
};
