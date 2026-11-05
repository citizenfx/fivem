#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <CL2LaunchMode.h>

TEST_CASE("cl2 launch flag is matched as a whole argument")
{
	WHEN("cl2 is only part of the executable path, a connect URL or another argument")
	{
		THEN("it is not cl2 mode")
		{
			REQUIRE_FALSE(launch::HasCommandLineArgument(L"\"C:\\Users\\Marcl2\\AppData\\Local\\RedM\\RedM.exe\"", L"-cl2"));
			REQUIRE_FALSE(launch::HasCommandLineArgument(L"C:\\testcl2\\FiveM.exe", L"-cl2"));
			REQUIRE_FALSE(launch::HasCommandLineArgument(L"\"C:\\a -cl2 b\\RedM.exe\"", L"-cl2"));
			REQUIRE_FALSE(launch::HasCommandLineArgument(L"\"D:\\RedM\\RedM.exe\" \"redm://connect/cfx.re/join/cl2abc\"", L"-cl2"));
			REQUIRE_FALSE(launch::HasCommandLineArgument(L"\"D:\\RedM\\RedM.exe\" --user-data-dir=\"C:\\Users\\Marcl2\\x y\"", L"-cl2"));
			REQUIRE_FALSE(launch::HasCommandLineArgument(L"\"D:\\RedM\\RedM.exe\" --foo=\"a -cl2 b\" -cl2x", L"-cl2"));
			REQUIRE_FALSE(launch::HasCommandLineArgument(L"-cl2", L"-cl2"));
			REQUIRE_FALSE(launch::HasCommandLineArgument(L"", L"-cl2"));
		}
	}

	WHEN("-cl2 is passed")
	{
		THEN("it is cl2 mode")
		{
			REQUIRE(launch::HasCommandLineArgument(L"\"C:\\Users\\Marcl2\\AppData\\Local\\RedM\\RedM.exe\" -cl2", L"-cl2"));
			REQUIRE(launch::HasCommandLineArgument(L"C:\\RedM\\RedM.exe -cl2", L"-cl2"));
			REQUIRE(launch::HasCommandLineArgument(L"\"D:\\RedM\\RedM.exe\"\t\"-cl2\"", L"-cl2"));
			REQUIRE(launch::HasCommandLineArgument(L"\"D:\\RedM\\RedM.exe\" -b1491 -cl2  -switchcl:1234 \"redm://connect/1.2.3.4\"", L"-cl2"));
		}
	}
}

TEST_CASE("cl2 subprocesses are recognized by their executable name")
{
	REQUIRE(launch::IsCL2SubProcessImage(L"C:\\Users\\Marcl2\\RedM.app\\data\\cache\\subprocess\\RedM_cl2_b1491_GameProcess.exe"));
	REQUIRE(launch::IsCL2SubProcessImage(L"D:\\RedM\\RedM.app\\data\\cache\\subprocess\\RedM_cl2_ChromeBrowser"));
	REQUIRE_FALSE(launch::IsCL2SubProcessImage(L"C:\\Users\\Marcl2\\RedM.app\\data\\cache\\subprocess\\RedM_b1491_GameProcess.exe"));
	REQUIRE_FALSE(launch::IsCL2SubProcessImage(L"D:\\my_cl2_dir\\RedM.app\\data\\cache\\subprocess\\RedM_ChromeBrowser"));
	REQUIRE_FALSE(launch::IsCL2SubProcessImage(L"C:\\Users\\Marcl2\\AppData\\Local\\RedM\\RedM.exe"));
}
