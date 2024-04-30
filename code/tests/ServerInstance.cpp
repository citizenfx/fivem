#include <StdInc.h>

#include "ServerInstanceBase.h"

#include "ServerInstance.h"

// Implements a none abstract ServerInstanceBase instance used for tests
class ServerInstanceBaseImpl : public fx::ServerInstanceBase
{
	std::string rootPath;

public:
	const std::string& GetRootPath() override
	{
		return rootPath;
	}
};

fx::ServerInstanceBase* ServerInstance::Create()
{
	return new ServerInstanceBaseImpl();
}
