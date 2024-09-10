#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <ComponentLoader.h>

int main(const int argc, char* argv[])
{
	ComponentLoader* loader = ComponentLoader::GetInstance();

	loader->Initialize();

	Catch::Session session;
	const int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0)
	{
		return returnCode;
	}
	const int numFailed = session.run();
	return numFailed;
}
