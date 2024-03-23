#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <ComponentLoader.h>

int main(int argc, char* argv[])
{
	ComponentLoader* loader = ComponentLoader::GetInstance();

	loader->Initialize();

	Catch::Session session;
	int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0)
	{
		return returnCode;
	}
	int numFailed = session.run();
	return numFailed;
}
