#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <ComponentLoader.h>

int main(const int argc, char* argv[])
{
	ComponentLoader* loader = ComponentLoader::GetInstance();

#ifdef IS_RDR3
	loader->InitializeWithString(R"(
[
	"vfs:core",
	"citizen:resources:core",
	"citizen:scripting:core",
	"scripting:gta",
	"conhost-v2",
	"http:client",
	"rage:scripting:rdr3",
	"rage:allocator:rdr3",
	"net:base",
	"net:packet"
]
	)");
#else
	loader->InitializeWithString(R"(
[
	"vfs:core",
	"citizen:resources:core",
	"citizen:scripting:core",
	"scripting:gta",
	"conhost-v2",
	"http:client",
	"rage:scripting:five",
	"rage:allocator:five",
	"net:base",
	"net:packet"
]
	)");
#endif

	Catch::Session session;
	const int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0)
	{
		return returnCode;
	}
	const int numFailed = session.run();
	return numFailed;
}
