#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <ComponentLoader.h>

int main(const int argc, char* argv[])
{
	ComponentLoader* loader = ComponentLoader::GetInstance();

	loader->Initialize();

	ComponentLoader::GetInstance()->ForAllComponents([&](fwRefContainer<ComponentData> componentData)
	{
		for (auto& instance : componentData->GetInstances())
		{
			// only initialize the vfs server implementation for now, because it is required for lua sandboxing tests
			if (componentData->GetName() != "vfs:impl:server")
			{
				continue;
			}

			instance->SetCommandLine(argc, argv);
			instance->Initialize();
		}
	});

	Catch::Session session;
	const int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0)
	{
		return returnCode;
	}
	const int numFailed = session.run();
	return numFailed;
}
