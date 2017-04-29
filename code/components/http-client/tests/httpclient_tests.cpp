#include "StdInc.h"

#if 0
#include "HttpClient.h"
#include <gtest/gtest.h>

TEST(HttpClientTests, VerifyStaticCallback)
{
	HttpClient* client = new HttpClient();

	static bool reqPassed;
	reqPassed = false;

	static fwAction<bool, const char*, size_t> callback;
	callback = [=] (bool, const char*, size_t)
	{
		reqPassed = true;
	};

	client->DoPostRequest(L"127.0.0.1", 80, L"/", "dummy", callback);

	EXPECT_TRUE(reqPassed);
}
#endif