#pragma once

namespace console
{
	class Context;
}

class ConsoleContextInstance
{
public:
	static console::Context* Get();
};
